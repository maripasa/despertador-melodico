#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "notes.h"

// Definição dos pinos utilizados
const uint BUZZER_A = 21; // Pino para o buzzer A
const uint BUZZER_B = 10; // Pino para o buzzer B
const uint BUTTON_A = 5;  // Pino para o botão A
const uint BUTTON_B = 6;  // Pino para o botão B
const uint LED = 12;      // Pino para o LED

// Constantes para configurações PWM e LED
const float DIVISOR_CLK_PWM = 16.0;      // Divisor de clock para o PWM
const uint16_t PERIOD = 2000;            // Período do PWM para o LED
const uint16_t LED_STEP = 100;           // Passo para controle de brilho do LED
const uint16_t MAX_WRAP_DIV_BUZZER = 16; // Valor máximo para wrap divisor do buzzer
const uint16_t MIN_WRAP_DIV_BUZZER = 2;  // Valor mínimo para wrap divisor do buzzer

// Estados para o gerenciamento dos botões
typedef enum
{
  IDLE,         // Estado inativo
  DEBOUNCING_A, // Debounce para o botão A
  RELEASE_A,    // Liberação do botão A
  DEBOUNCING_B, // Debounce para o botão B
  RELEASE_B,    // Liberação do botão B
  ACTION_A,     // Ação após o botão A ser pressionado
  ACTION_B      // Ação após o botão B ser pressionado
} state_button;

// Estados para o gerenciamento da música
typedef enum
{
  PLAY_NOTE,     // Estado para tocar uma nota
  DURATION_NOTE, // Estado para controlar a duração da nota
  PLAY_REST,     // Estado para tocar um descanso
  DURATION_REST  // Estado para controlar a duração do descanso
} state_song;

// Variáveis globais
uint16_t wrap_div_buzzer = 8; // Valor atual do divisor de wrap do buzzer
uint16_t led_level = 100;     // Nível de brilho do LED

// Função para tocar uma nota no buzzer
void play_note(uint pin, uint16_t wrap)
{
  int slice = pwm_gpio_to_slice_num(pin);          // Obtém o slice PWM correspondente ao pino
  pwm_set_wrap(slice, wrap);                       // Define o valor de wrap para o PWM
  pwm_set_gpio_level(pin, wrap / wrap_div_buzzer); // Ajusta o nível PWM com base no wrap
  pwm_set_enabled(slice, true);                    // Habilita o PWM no slice correspondente
}

// Função para tocar um descanso (silenciar o buzzer)
void play_rest(uint pin)
{
  int slice = pwm_gpio_to_slice_num(pin); // Obtém o slice PWM correspondente ao pino
  pwm_set_enabled(slice, false);          // Desabilita o PWM, silenciando o buzzer
}

// Função para tocar a música associada ao buzzer B
void play_songB(int pin)
{
  static uint i = 0;               // Índice da nota atual
  static uint cnt = 0;             // Contador de tempo para controlar duração
  static state_song s = PLAY_NOTE; // Estado atual da música

  int dur = 800 / durations[i]; // Calcula a duração da nota atual

  switch (s)
  {
  case PLAY_NOTE:              // Estado para tocar uma nota
    play_note(pin, melody[i]); // Toca a nota atual
    s = DURATION_NOTE;         // Muda para o estado de controle da duração da nota
    cnt = 0;                   // Reinicia o contador
    break;
  case DURATION_NOTE: // Estado para controlar a duração da nota
    sleep_ms(1);      // Espera 1ms
    if (cnt++ > dur)  // Se a duração excedeu, vai para o estado de descanso
      s = PLAY_REST;
    break;
  case PLAY_REST:      // Estado para tocar um descanso
    play_rest(pin);    // Silencia o buzzer
    s = DURATION_REST; // Muda para o estado de duração do descanso
    cnt = 0;           // Reinicia o contador
    break;
  case DURATION_REST: // Estado para controlar a duração do descanso
    sleep_ms(1);      // Espera 1ms
    if (cnt++ > dur)
    {                        // Se a duração do descanso excedeu
      s = PLAY_NOTE;         // Volta para o estado de tocar nota
      i++;                   // Avança para a próxima nota
      if (i > SONG_B_LENGTH) // Se chegou ao fim da música, reinicia
        i = 0;
    }
    break;
  }
}

// Função para ler o estado dos botões com debounce
void read_buttons()
{
  static state_button s = IDLE;    // Estado atual do botão
  static uint cnt = 0;             // Contador para debounce
  const uint DEBOUNCE_CYCLES = 50; // Ciclos de debounce

  // Máquina de estados para leitura dos botões
  switch (s)
  {
  case IDLE:                     // Estado inativo, monitora os botões
    if (gpio_get(BUTTON_A) == 0) // Se o botão A foi pressionado
      s = DEBOUNCING_A;          // Vai para o estado de debounce do botão A
    if (gpio_get(BUTTON_B) == 0) // Se o botão B foi pressionado
      s = DEBOUNCING_B;          // Vai para o estado de debounce do botão B
    cnt = 0;                     // Reinicia o contador
    break;
  case DEBOUNCING_A: // Estado de debounce do botão A
    if (gpio_get(BUTTON_A) == 0)
    {        // Se o botão A continua pressionado
      cnt++; // Incrementa o contador
      if (cnt > DEBOUNCE_CYCLES)
      {                // Se passou o tempo de debounce
        cnt = 0;       // Reinicia o contador
        s = RELEASE_A; // Vai para o estado de liberação do botão A
      }
    }
    else
    {
      s = IDLE; // Se o botão foi liberado, volta ao estado inativo
    }
    break;
  case DEBOUNCING_B: // Estado de debounce do botão B
    if (gpio_get(BUTTON_B) == 0)
    {        // Se o botão B continua pressionado
      cnt++; // Incrementa o contador
      if (cnt > DEBOUNCE_CYCLES)
      {                // Se passou o tempo de debounce
        cnt = 0;       // Reinicia o contador
        s = RELEASE_B; // Vai para o estado de liberação do botão B
      }
    }
    else
    {
      s = IDLE; // Se o botão foi liberado, volta ao estado inativo
    }
    break;
  case RELEASE_A:                // Estado de liberação do botão A
    if (gpio_get(BUTTON_A) == 1) // Se o botão A foi solto
      s = ACTION_A;              // Vai para a ação associada ao botão A
    break;
  case RELEASE_B:                // Estado de liberação do botão B
    if (gpio_get(BUTTON_B) == 1) // Se o botão B foi solto
      s = ACTION_B;              // Vai para a ação associada ao botão B
    break;
  case ACTION_A:            // Ação do botão A (aumenta brilho do LED e divisor do buzzer)
    s = IDLE;               // Volta para o estado inativo
    if (led_level < PERIOD) // Aumenta o brilho do LED
      led_level += LED_STEP;
    pwm_set_gpio_level(LED, led_level);        // Atualiza o nível do PWM do LED
    if (wrap_div_buzzer < MAX_WRAP_DIV_BUZZER) // Aumenta o divisor do buzzer
      wrap_div_buzzer += MIN_WRAP_DIV_BUZZER;
    break;
  case ACTION_B:              // Ação do botão B (diminui brilho do LED e divisor do buzzer)
    s = IDLE;                 // Volta para o estado inativo
    if (led_level > LED_STEP) // Diminui o brilho do LED
      led_level -= LED_STEP;
    pwm_set_gpio_level(LED, led_level);        // Atualiza o nível do PWM do LED
    if (wrap_div_buzzer > MIN_WRAP_DIV_BUZZER) // Diminui o divisor do buzzer
      wrap_div_buzzer -= MIN_WRAP_DIV_BUZZER;
    break;

  default: // Estado padrão (se houver erro, volta ao inativo)
    s = IDLE;
    cnt = 0;
  }
}

// Função de configuração inicial
void setup_audio()
{
  uint slice;
  stdio_init_all(); // Inicializa a comunicação padrão (UART)

  gpio_set_function(LED, GPIO_FUNC_PWM); // Configura o pino do LED como saída PWM

  gpio_set_function(BUTTON_A, GPIO_FUNC_SIO); // Configura o botão A como I/O padrão
  gpio_set_function(BUTTON_B, GPIO_FUNC_SIO); // Configura o botão B como I/O padrão

  gpio_set_dir(BUTTON_A, GPIO_IN); // Define o botão A como entrada
  gpio_set_dir(BUTTON_B, GPIO_IN); // Define o botão B como entrada

  gpio_pull_up(BUTTON_A); // Habilita o resistor pull-up no botão A
  gpio_pull_up(BUTTON_B); // Habilita o resistor pull-up no botão B

  slice = pwm_gpio_to_slice_num(LED);     // Obtém o slice PWM para o LED
  pwm_set_clkdiv(slice, DIVISOR_CLK_PWM); // Define o divisor de clock para o PWM do LED
  pwm_set_wrap(slice, PERIOD);            // Define o período do PWM para o LED
  pwm_set_gpio_level(LED, led_level);     // Ajusta o nível de brilho inicial do LED
  pwm_set_enabled(slice, true);           // Habilita o PWM no slice correspondente

  gpio_set_function(BUZZER_A, GPIO_FUNC_PWM); // Configura o pino do buzzer A como PWM
  slice = pwm_gpio_to_slice_num(BUZZER_A);    // Obtém o slice PWM para o buzzer A
  pwm_set_clkdiv(slice, DIVISOR_CLK_PWM);     // Define o divisor de clock para o PWM do buzzer A
}

// Função principal
int main_audio()
{

  play_songB(BUZZER_A); // Toca a música no buzzer A
  read_buttons();       // Lê o estado dos botões
}
