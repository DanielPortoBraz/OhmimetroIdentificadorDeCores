/*
 * Por: Daniel Porto Braz
 *    Ohmímetro utilizando o ADC da BitDogLab
 *
 * Código Original: Wilton Lacerda
 *  
 * Este projeto é baseado na solução proposta por Wilton Lacerda, em que se utiliza
 * o valor ADC da BitDogLab para calcular e exibir valores de resistência no display
 * SSD 1306. Adicionalmente, há a exibição do código de cores associado à resistência 
 * calculada.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "lib/val_resistores.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

// Biblioteca gerada pelo arquivo .pio durante compilação.
#include "ws2818b.pio.h"

// Definição do número de LEDs e pino.
#define LED_COUNT 25
#define LED_PIN 7

// Definição de pixel GRB
struct pixel_t {
  uint8_t G, R, B; // Três valores de 8-bits compõem um pixel.
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t; // Mudança de nome de "struct pixel_t" para "npLED_t" por clareza.

// Declaração do buffer de pixels que formam a matriz.
npLED_t leds[LED_COUNT];

// Variáveis para uso da máquina PIO.
PIO np_pio;
uint sm;

/**
 * Inicializa a máquina PIO para controle da matriz de LEDs.
 */
void npInit(uint pin) {

  // Cria programa PIO.
  uint offset = pio_add_program(pio0, &ws2818b_program);
  np_pio = pio0;

  // Toma posse de uma máquina PIO.
  sm = pio_claim_unused_sm(np_pio, false);
  if (sm < 0) {
    np_pio = pio1;
    sm = pio_claim_unused_sm(np_pio, true); // Se nenhuma máquina estiver livre, panic!
  }

  // Inicia programa na máquina PIO obtida.
  ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);

  // Limpa buffer de pixels.
  for (uint i = 0; i < LED_COUNT; ++i) {
    leds[i].R = 0;
    leds[i].G = 0;
    leds[i].B = 0;
  }
}

/**
 * Atribui uma cor RGB a um LED.
 */
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
  leds[index].R = r;
  leds[index].G = g;
  leds[index].B = b;
}

/**
 * Limpa o buffer de pixels.
 */
void npClear() {
  for (uint i = 0; i < LED_COUNT; ++i)
    npSetLED(i, 0, 0, 0);
}

/**
 * Escreve os dados do buffer nos LEDs.
 */
void npWrite() {
  // Escreve cada dado de 8-bits dos pixels em sequência no buffer da máquina PIO.
  for (uint i = 0; i < LED_COUNT; ++i) {
    pio_sm_put_blocking(np_pio, sm, leds[i].G);
    pio_sm_put_blocking(np_pio, sm, leds[i].R);
    pio_sm_put_blocking(np_pio, sm, leds[i].B);
  }
  sleep_us(100); // Espera 100us, sinal de RESET do datasheet.
}

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define ADC_PIN 28 // GPIO para o voltímetro
#define Botao_A 5  // GPIO para botão A

int R_conhecido = 10000;   // Resistor de 10k ohm
float R_x = 0.0;           // Resistor desconhecido
float ADC_VREF = 3.31;     // Tensão de referência do ADC
int ADC_RESOLUTION = 4095; // Resolução do ADC (12 bits)

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
  reset_usb_boot(0, 0);
}

// Calcula o multiplicador (3° faixa) do resistor
int calcular_mult(int valor){
  if (valor / 100000 >= 1) // 100k
    return 4;

  else if (valor / 10000 >= 1) // 10k
    return 3;

  else if (valor / 1000 >= 1) // 1k
    return 2;

  else if (valor / 100 >= 1) // 100
    return 1;
  
  return 0;
}


// Busca o valor mais próximo da série E24 para o valor passado e retorna o inteiro para as 2 primeiras faixas do resistor
int aprox_e24(int valor){ 
  int inicio = 0;
  int meio;
  int fim = 23;
  
  while(inicio <= fim){
    meio = (fim + inicio) / 2;

    if (serie_E24[meio] == valor)
      return serie_E24[meio];

    else if (serie_E24[meio] > valor)
      fim = meio - 1;
    else 
      inicio = meio + 1;
  }

  return serie_E24[meio];
}

int main()
{
  stdio_init_all();

  // Inicializa matriz de LEDs NeoPixel.
  npInit(LED_PIN);
  npClear();

  // Para ser utilizado o modo BOOTSEL com botão B
  gpio_init(botaoB);
  gpio_set_dir(botaoB, GPIO_IN);
  gpio_pull_up(botaoB);
  gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
  // Aqui termina o trecho para modo BOOTSEL com botão B

  gpio_init(Botao_A);
  gpio_set_dir(Botao_A, GPIO_IN);
  gpio_pull_up(Botao_A);

  // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400 * 1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
  gpio_pull_up(I2C_SDA);                                        // Pull up the data line
  gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
  ssd1306_t ssd;                                                // Inicializa a estrutura do display
  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
  ssd1306_config(&ssd);                                         // Configura o display
  ssd1306_send_data(&ssd);                                      // Envia os dados para o display

  // Limpa o display. O display inicia com todos os pixels apagados.
  ssd1306_fill(&ssd, false);
  ssd1306_send_data(&ssd);

  adc_init();
  adc_gpio_init(ADC_PIN); // GPIO 28 como entrada analógica

  float tensao;
  char str_x[5]; // Buffer para armazenar a string
  char str_y[5]; // Buffer para armazenar a string

  int faixas[3]; // Guarda os valores das 3 faixas do resistor
  int R_x_aprox; // Valor com correção de +5% do valor encontrado 

  bool cor = true;
  while (true)
  {
    adc_select_input(2); // Seleciona o ADC para eixo X. O pino 28 como entrada analógica

    float soma = 0.0f;
    for (int i = 0; i < 500; i++)
    {
      soma += adc_read();
      sleep_ms(1);
    }
    float media = soma / 500.0f;

      // Fórmula simplificada: R_x = R_conhecido * ADC_encontrado /(ADC_RESOLUTION - adc_encontrado)
    R_x = (R_conhecido * media) / (ADC_RESOLUTION - media); 
    R_x_aprox = R_x + (R_x * 0.05); // Atualiza a correção de +5%, apenas para cálculos das faixas

    faixas[2] = calcular_mult(R_x_aprox);
    faixas[0] = aprox_e24(R_x_aprox / pow(10, faixas[2])) / 10; // Retorna a 1° casa do valor na serie E24
    faixas[1] = aprox_e24(R_x_aprox / pow(10, faixas[2])) % 10; // Retorna a 2° casa do valor na serie E24

    sprintf(str_x, "%1.0f", media); // Converte o inteiro em string
    sprintf(str_y, "%1.0f", R_x);   // Converte o float em string

    // Acende os LEDs RGB com o padrão das cores das faixas do resistor medido
    for (int i = 0; i < 3; i++){
      npSetLED(13, rgb_cores[faixas[0]][0], rgb_cores[faixas[0]][1], rgb_cores[faixas[0]][2]);
      npSetLED(12, rgb_cores[faixas[1]][0], rgb_cores[faixas[1]][1], rgb_cores[faixas[1]][2]);
      npSetLED(11, rgb_cores[faixas[2]][0], rgb_cores[faixas[2]][1], rgb_cores[faixas[2]][2]);
    }
    npWrite(); // Escreve os dados nos LEDs.

    // cor = !cor;
    //  Atualiza o conteúdo do display com animações
    ssd1306_fill(&ssd, !cor);                          // Limpa o display
    ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);      // Desenha um retângulo
    ssd1306_line(&ssd, 3, 37, 123, 37, cor);           // Desenha uma linha
    ssd1306_draw_string(&ssd, "1oF", 8, 6); // Desenha uma string
    ssd1306_draw_string(&ssd, cores[faixas[0]], 40, 6); // Desenha uma string
    ssd1306_draw_string(&ssd, "2oF", 8, 14); // Desenha uma string
    ssd1306_draw_string(&ssd, cores[faixas[1]], 40, 14); // Desenha uma string
    ssd1306_draw_string(&ssd, "3oF", 8, 22); // Desenha uma string
    ssd1306_draw_string(&ssd, cores[faixas[2]], 40, 22); // Desenha uma string
    ssd1306_draw_string(&ssd, "ADC", 13, 41);          // Desenha uma string
    ssd1306_draw_string(&ssd, "Resisten.", 50, 41);    // Desenha uma string
    ssd1306_line(&ssd, 44, 37, 44, 60, cor);           // Desenha uma linha vertical
    ssd1306_draw_string(&ssd, str_x, 8, 52);           // Desenha uma string
    ssd1306_draw_string(&ssd, str_y, 59, 52);          // Desenha uma string
    ssd1306_send_data(&ssd);                           // Atualiza o display
    sleep_ms(700);
  }
}