<h1 align="center">Controle de Matriz de LEDs 5x5 - Embarcatech 💻</h1>

<p>Este projeto utiliza um microcontrolador para controlar uma matriz de LEDs, exibindo números de 0 a 9, que podem ser alternados por meio de botões. O botão A incrementa o número, enquanto o botão B o decrementa. Além disso, um LED vermelho pisca continuamente. O objetivo principal é permitir a criação de uma sequência numérica em resposta à interação do usuário. 🚀</p> 

<h2>Funcionalidades</h2>

<ul>
<li><strong>Exibição de números:</strong> O projeto exibe os números de 0 a 9 em uma matriz de LEDs 5x5.</li>
<li><strong>Controle por botões:</strong> Utiliza os botões A e B para incrementar ou decrementar o número exibido na matriz.</li>
<li><strong>Indicador de funcionamento:</strong> Um LED adicional pisca periodicamente para indicar que o sistema está ativo.</li>
<li><strong>Controle eficiente com PIO:</strong> O controle da matriz de LEDs é feito utilizando o recurso PIO (Peripheral I/O) do Raspberry Pi Pico para garantir uma manipulação eficiente dos LEDs.</li>
 </ul>

<h2>Tecnologias</h2>
<ul>
  <li><strong>Raspberry Pi Pico W</strong> - A placa de desenvolvimento baseada no microcontrolador RP2040, que serve como base do projeto.</li>
  <li><strong>Pico SDK</strong> - O kit de desenvolvimento de software da Raspberry Pi Pico, utilizado para programar o microcontrolador RP2040.</li>
  <li><strong>Linguagem de Programação C</strong> - Utilizada para o desenvolvimento do código do projeto, aproveitando as capacidades do RP2040.</li>
  <li><strong>GPIO (General Purpose Input/Output)</strong> - Para interação com hardware externo, como os botões A e B, LEDs e o buzzer.</li>
  <li><strong>Botão A</strong> - Utilizado para controlar o incremento de valores no projeto.</li>
  <li><strong>Botão B</strong> - Utilizado para controlar o decremento de valores no projeto.</li>
  <li><strong>Git</strong> - Sistema de controle de versão para gerenciar o código-fonte do projeto.</li>
  <li><strong>GitHub</strong> - Plataforma para hospedar, versionar e colaborar no repositório do projeto.</li>
  <li><strong>Matriz de LEDs 5x5</strong> - Utilizada para exibir os números e criar animações no projeto.</li> 
  <li><strong>Wokwi Integrado ao Visual Studio Code</strong> - Ferramenta de simulação de circuitos, integrada ao Visual Studio Code, para testar o código e visualizar o circuito sem a necessidade de hardware físico.</li>
  <li><strong>CMake</strong> - Ferramenta de automação de construção usada para gerenciar o processo de compilação e configuração do código no Raspberry Pi Pico.</li>
</ul>

<h2>Diagrama de Conexões</h2>

<ul>
<li><strong>Matriz de LEDs:</strong> Conectada ao pino GPIO 7 do Raspberry Pi Pico.</li>
<li><strong>Botão A:</strong> Conectado ao pino GPIO 5.</li>
<li><strong>Botão B:</strong> Conectado ao pino GPIO 6.</li>
<li><strong>LED indicador:</strong> Conectado ao pino GPIO 13</li>
</ul>

 <h2>Imagem</h2>
   
  ![projeto](https://github.com/MarianaBarretto/Geracao-Animacoes-Led-5x5/blob/feature/estrutura-codigo/imagem.jpg)

  <h2>Definições de Teclas</h2>
<p>A interação com o sistema é feita através de dois botões com as seguintes definições de teclas:</p>
  <table>
  <thead>
    <tr>
      <th>Tecla</th>
      <th>Função</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>Botão A</td>
      <td>Se o botão A for pressionado, o número atual incrementa</td>
    </tr>
    <tr>
      <td>Botão B</td>
      <td>Se o botão B for pressionado, o número atual decrementa</td>
    </tr> 
    
  </tbody>
</table>


<h2>Explicação do código</h2>

<h3>1. Inclusão de bibliotecas</h3>

<pre><code>#include stdio.h
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/irq.h"
#include "numeros_led.pio.h"</code></pre>

<p>Essas bibliotecas fornecem funcionalidades essenciais para entrada/saída, controle de periféricos, interrupções e manipulação de LEDs usando a interface PIO (Programmable I/O) do Raspberry Pi Pico.</p>

<h3>2. Definições de constantes</h3>

<pre><code>#define NUM_PIXELS 25
#define OUT_PIN 7

#define led_piscando 13

#define button_A 5
#define button_B 6</code></pre>

<ul>
    <li><strong>NUM_PIXELS:</strong> Define o número de LEDs na matriz (5x5 = 25).</li>
    <li><strong>OUT_PIN:</strong> Define o pino que controla os LEDs da matriz de 5x5.</li>
    <li><strong>led_piscando:</strong> Define os pino o GPIO do LED individual vermelho.</li>
    <li><strong>button_A e button_B:</strong> Definem os pinos GPIO para os botões de controle.</li>
</ul>

<h3>3. Variáveis Globais</h3>

<pre><code>PIO pio;          // Ponteiro para o PIO
uint sm;          // Estado da máquina de estado do PIO
double r = 0.0, b = 0.5, g = 0.0;  // Definição de cores RGB
int i;            // Armazena o número atual
static volatile uint32_t last_time = 0;  // Controle de debounce dos botões</code></pre>

<ul>
	<li><strong>PIO:</strong> (Programável I/O) é usado para controlar os LEDs.</li>
	<li><strong>sm:</strong> (state machine) é uma máquina de estado para atualizar a matriz.</li>
	<li><strong>variável i:</strong> guarda o número atual exibido na matriz.</li>
	<li><strong>last_time:</strong> é usada para evitar múltiplas leituras erradas do botão (debounce).</li>
</ul>

<h3>4. Representação dos números na matriz</h3>

<pre><code>double numeros[10][25] = {
    { // Número 0
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0
    },
    // ... (demais números 1 a 9)
};</code></pre>

<p>A matriz numeros contém os padrões para os números de 0 a 9. Cada número é representado por 25 valores:</p>     

<ul>
    <li><strong>1.0:</strong> Representa um LED aceso.</li>
    <li><strong>0.0:</strong> Representa um LED apagado.</li>    
</ul>

<h3>5. Conversão de RGB para Formato WS2812</h3>

<pre><code>uint32_t matrix_rgb (double r, double g, double b) {
    return ((int)(g * 255) << 24) | ((int)(r * 255) << 16) | ((int)(b * 255) << 8);
}</code></pre>

<p>A função <code>matrix_rgb</code> converte valores de cor de 0.0 a 1.0 para um formato de 8 bits por cor, compatível com os LEDs WS2812, em que a ordem de bytes é G-R-B.</p>

<h3>6. Exibição de Números na Matriz</h3>

<pre><code>void desenho (int n) {
    double *desenho = numeros[n];
    for (int16_t i = 0; i < NUM_PIXELS; i++) {
        uint32_t valor_led = matrix_rgb(r * desenho[24 - i], g * desenho[24 - i], b * desenho[24 - i]);
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}</code></pre>

<ul>
    <li>Recebe um número <strong>n</strong> e atualiza os LEDs com a cor definida (verde predominante).</li>
    <li>A função utiliza o comando <code>pio_sm_put_blocking</code> para enviar os valores de cor para o PIO que controla a matriz de LEDs.</li>
</ul>

<h3>7. Interrupção para os Botões</h3>

<pre><code>void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if (current_time - last_time > 300000) {  // Debouncing de 300ms
        last_time = current_time;
        if (gpio == button_A && i < 9) {
            i++;
            desenho(i);
        } else if (gpio == button_B && i > 0) {
            i--;
            desenho(i);
        }
    }
}</code></pre>

<ul>
<li>Essa função detecta quando um botão é pressionado.</li>
<li>Evita leituras erradas com um tempo mínimo de 300ms (debounce).</li>
<li>Se <strong>button_A</strong> for pressionado, incrementa o número exibido.</li>
<li>Se <strong>button_B</strong> for pressionado, decrementa o número exibido.</li>
</ul>

<h3>8. Configuração Inicial</h3>

<pre><code>void comeco() {
    pio = pio0;
    i = 0;
    set_sys_clock_khz(128000, false);
    stdio_init_all();
    uint offset = pio_add_program(pio, &numeros_led_program);
    sm = pio_claim_unused_sm(pio, true);
    numeros_led_program_init(pio, sm, offset, OUT_PIN);
    
    gpio_init(pisca_led);
    gpio_set_dir(pisca_led, GPIO_OUT);
    
    gpio_init(button_A);
    gpio_set_dir(button_A, GPIO_IN);
    gpio_pull_up(button_A);
    gpio_set_irq_enabled_with_callback(button_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    
    gpio_init(button_B);
    gpio_set_dir(button_B, GPIO_IN);
    gpio_pull_up(button_B);
    gpio_set_irq_enabled_with_callback(button_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    
    desenho(0);
}</code></pre>

<ul>
	<li>A função <strong>comeco()</strong> inicializa o PIO, o LED indicador, os botões e exibe o número 0 na matriz.</li>
</ul>

<h3>9. Função principal <strong>main()</strong></h3>

<pre><code>int main() {
    comeco();
    while (true) {
        gpio_put(pisca_led, true);
        sleep_ms(100);
        gpio_put(pisca_led, false);
        sleep_ms(100);
    }
}</code></pre>

<ul>
<li>Chama <strong>comeco()</strong> para inicializar tudo.</li>    
<li>O <strong>LED indicador pisca</strong> para mostrar que o código está rodando.</li>    
<li>O controle dos números <strong>é feito via interrupção</strong> (não precisa de lógica extra no loop principal).</li>    
</ul>

<h2>Link do Vídeo do Projeto</h2>

<h3>Vídeo de Demostração</h3>

<p>Google Drive: https://drive.google.com/file/d/1RbZeAC_7gJb0Y4pCDj6avRk0sbkTeR7r/view?usp=sharing</p>
<p>YouTube: https://youtu.be/h6oNSjOqmwI</p>

   <h1>Como Executar o Projeto</h1>
        <h2>Clone o Repositório</h2>
    <ol>
        <li>Abra o <strong>Prompt de Comando</strong> ou o terminal de sua preferência.</li>
        <li>Clone o repositório usando o Git:
            <pre><code>git clone https://github.com/seu-usuario/seu-repositorio.git</code></pre>
        </li>
        <li>Entre no diretório do projeto:
            <pre><code>cd seu-repositorio</code></pre>
        </li>
    </ol>    
    <h2>Configuração do Ambiente Local</h2>
    <ol>
        <li>Baixe e instale o <a href="https://cmake.org/download/" target="_blank">CMake</a>.</li>
        <li>Configure o <strong>Pico SDK</strong> seguindo o guia oficial em 
            <a href="https://datasheets.raspberrypi.com/pico/raspberry-pi-pico-c-sdk.pdf" target="_blank">
                Raspberry Pi Pico SDK</a>.
        </li>
        <li>Crie um diretório de construção:
            <pre><code>mkdir build</code></pre>
            <pre><code>cd build</code></pre>
        </li>
        <li>Execute o CMake para gerar os arquivos de construção:
            <pre><code>cmake ..</code></pre>
        </li>
    </ol>
    <h2>Compilar o Projeto</h2>
    <p>Após configurar o ambiente, compile o projeto executando o seguinte comando dentro do diretório <code>build</code>:</p>
    <pre><code>make</code></pre>
    <p>Isso criará o arquivo binário do programa, geralmente no formato <code>.uf2</code>.</p>
   <h2>Testando o Projeto</h2>
    <ul>
        <li>Pressione os botões A e B para interagir com o sistema.</li>
        <li>Observe os LEDs para respostas visuais.</li>        
        <li>Verifique a interação com os componentes simulados e depure o código conforme necessário.</li>
    </ul>

<h4>Autor: <strong>Mariana Barretto</strong></h4>