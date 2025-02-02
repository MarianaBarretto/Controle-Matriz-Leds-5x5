// As bibliotecas fornecem funcionalidades essenciais para entrada/saída, controle de periféricos, interrupções e manipulação de LEDs usando a interface PIO
#include <stdio.h>  // Para operações de entrada/saída padrão
#include "pico/stdlib.h"  // Biblioteca para usar funções padrão do Raspberry Pi Pico
#include "hardware/pio.h"  // Para manipulação de PIO (Peripheral I/O)
#include "hardware/clocks.h"  // Para controlar o clock do sistema
#include "pico/bootrom.h"  // Para acesso ao bootrom (não utilizado diretamente aqui)

// Arquivo gerado pelo PIO para manipular a matriz de LEDs
#include "controle_matriz_leds_5x5.pio.h"

// Definindo o número de LEDs na matriz (5x5 = 25 LEDs)
#define NUM_PIXELS 25

// Definindo o pino de saída onde os LEDs estão conectados
#define OUT_PIN 7

// Definindo o pino para o LED que pisca (LED para indicar o funcionamento)
#define led_piscando 13

// Definindo os pinos dos botões A e B que controlam a troca de números
#define button_A 5  // Botão A no pino GPIO 5
#define button_B 6  // Botão B no pino GPIO 6

// Variáveis globais
PIO pio;  // Ponteiro para o PIO (que gerencia os LEDs)
uint sm;  // Estado da máquina de estado do PIO
double r = 0.0, b = 0.3, g = 0;  // Cor inicial do LED (no formato RGB)
int i;  // Variável para armazenar o número exibido na matriz
static volatile uint32_t last_time = 0;  // Para controle de tempo (usado para debouncing)

// A matriz `numeros` contém os padrões para cada número de 0 a 9 em uma matriz de 5x5
double numeros[10][25] = {
                        { // Número 0
                            1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 0.0, 0.0, 0.0, 1.0,
                            1.0, 0.0, 0.0, 0.0, 1.0,
                            1.0, 0.0, 0.0, 0.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0
                        },
                        { // Número 1
                            0.0, 0.0, 1.0, 0.0, 0.0,
                            0.0, 0.0, 1.0, 1.0, 0.0,
                            0.0, 0.0, 1.0, 0.0, 0.0,
                            0.0, 0.0, 1.0, 0.0, 0.0,
                            1.0, 1.0, 1.0, 1.0, 1.0
                        },
                        { // Número 2
                            1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 0.0, 0.0, 0.0, 0.0,
                            1.0, 1.0, 1.0, 1.0, 1.0,
                            0.0, 0.0, 0.0, 0.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0
                        },
                        { // Número 3
                            1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 0.0, 0.0, 0.0, 0.0,
                            1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 0.0, 0.0, 0.0, 0.0,
                            1.0, 1.0, 1.0, 1.0, 1.0
                        },
                        { // Número 4
                            1.0, 0.0, 0.0, 1.0, 0.0,
                            0.0, 1.0, 0.0, 0.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0,
                            0.0, 1.0, 0.0, 0.0, 0.0,
                            0.0, 0.0, 0.0, 1.0, 0.0
                        },
                        { // Número 5
                            1.0, 1.0, 1.0, 1.0, 1.0,
                            0.0, 0.0, 0.0, 0.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 0.0, 0.0, 0.0, 0.0,
                            1.0, 1.0, 1.0, 1.0, 1.0
                        },
                        { // Número 6
                            1.0, 1.0, 1.0, 1.0, 1.0,
                            0.0, 0.0, 0.0, 0.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 0.0, 0.0, 0.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0
                        },

                        { // Número 7
                            1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 0.0, 0.0, 0.0, 0.0,
                            0.0, 0.0, 0.0, 1.0, 0.0,
                            0.0, 0.0, 1.0, 0.0, 0.0,
                            0.0, 1.0, 0.0, 0.0, 0.0
                        },
                        { // Número 8
                            1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 0.0, 0.0, 0.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 0.0, 0.0, 0.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0
                        },
                        { // Número 9
                            1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 0.0, 0.0, 0.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 0.0, 0.0, 0.0, 0.0,
                            1.0, 1.0, 1.0, 1.0, 1.0
                        }
};

// Função para converter as intensidades de cor RGB (0 a 1) para o formato de 8 bits usado pelos LEDs WS2812
uint32_t matrix_rgb (double r, double g, double b) { 
  unsigned char R, G, B;
  
  // Converte os valores de intensidade de 0 a 1 para 0 a 255
  R = r * 255;
  G = g * 255;
  B = b * 255;
  
  // Retorna o valor combinado das cores no formato G-R-B (ordem exigida pelos WS2812)
  return (G << 24) | (R << 16) | (B << 8);
}

// Função para desenhar o número na matriz de LEDs
void desenho (int n) {
    uint32_t valor_led;  // Variável para armazenar o valor do LED
    double *desenho = numeros[n];  // Seleciona o padrão para o número a ser exibido

    // Itera sobre os LEDs da matriz e desenha o número
    for (int16_t i = 0; i < NUM_PIXELS; i++) {
        // Converte o valor da matriz para o valor RGB do LED
        valor_led = matrix_rgb(r * desenho[24 - i], g * desenho[24 - i], b * desenho[24 - i]); 
        // Envia o valor RGB para o PIO controlar o LED
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}

// Função de interrupção para os botões A e B (com controle de tempo para evitar múltiplos eventos rápidos)
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());  // Obtém o tempo atual em microssegundos
    if (current_time - last_time > 300000) {  // Verifica se o tempo decorrido é superior a 300ms (debouncing)
        last_time = current_time;  // Atualiza o último tempo registrado

        // Se o botão A for pressionado, incrementa o número exibido
        if (gpio == button_A) {
            if (i < 9 && i >= 0) {
                i++;
                desenho(i);  // Atualiza a exibição do número
            }
        }
        // Se o botão B for pressionado, decrementa o número exibido
        else if (gpio == button_B) {
            if (i <= 9 && i > 0) {
                i--;
                desenho(i);  // Atualiza a exibição do número
            }            
        }
        // Se um outro pino (não os botões A ou B) gerar a interrupção, não faz nada
        else {
            // Não faz nada
        }   
    }
}

// Função de inicialização (configurações iniciais)
void comeco() {
    pio = pio0;  // Seleciona o PIO 0
    bool ok;
    i = 0;  // Começa com o número 0 sendo exibido

    ok = set_sys_clock_khz(128000, false);  // Configura o clock do sistema para 128MHz

    stdio_init_all();  // Inicializa a comunicação serial para debug

    // Adiciona o programa gerado pelo PIO e inicializa a máquina de estados
    uint offset = pio_add_program(pio, &controle_matriz_leds_5x5_program);
    sm = pio_claim_unused_sm(pio, true);  // Solicita uma máquina de estados não usada
    controle_matriz_leds_5x5_program_init(pio, sm, offset, OUT_PIN);  // Inicializa o programa do PIO

    // Inicializa o LED que pisca (usado para indicar que o sistema está funcionando)
    gpio_init(led_piscando);
    gpio_set_dir(led_piscando, GPIO_OUT);  // Define o pino como saída

    // Inicializa os botões A e B e configura como entradas com pull-up interno
    gpio_init(button_A);
    gpio_set_dir(button_A, GPIO_IN);
    gpio_pull_up(button_A);

    gpio_init(button_B);
    gpio_set_dir(button_B, GPIO_IN);
    gpio_pull_up(button_B);

    // Configura as interrupções para os botões A e B
    gpio_set_irq_enabled_with_callback(button_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(button_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Exibe o número 0 inicialmente
    desenho(0);
}

// Função principal (loop principal do programa)
int main() {
    comeco();  // Chama a função de inicialização

    while (true) {  // Piscar continuamente 5 vezes por segundo.
        gpio_put(led_piscando, true);  // Acende o LED que pisca
        sleep_ms(100);  // Espera 100ms
        gpio_put(led_piscando, false);  // Apaga o LED que pisca
        sleep_ms(100);  // Espera 100ms
    }
}
