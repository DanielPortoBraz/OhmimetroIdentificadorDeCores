# Ohmímetro Identificador de Cores

Este projeto consiste no desenvolvimento de um **ohmímetro** baseado na placa **BitDogLab** (RP2040), capaz de medir resistores desconhecidos e exibir, além do valor da resistência, o respectivo **código de cores** segundo a **série E24** no display **OLED SSD1306**.

## 📋 Descrição do Projeto

O sistema realiza a medição de resistência utilizando o princípio de **divisão de tensão**, fazendo uso do conversor analógico-digital (ADC) interno da BitDogLab. A partir do valor de resistência obtido, o projeto:

- Aplica uma correção de +5% no valor calculado.
- Identifica a primeira, segunda e terceira faixa de cores correspondentes ao resistor.
- Exibe em tempo real o valor de resistência e as faixas de cor no display OLED.

O projeto é voltado para resistores da série E24 (5% de tolerância).

## 🛠️ Materiais Utilizados

- Placa **BitDogLab** (baseada no RP2040)
- Display OLED **SSD1306** (com comunicação via I2C)
- **Protoboard**
- **Jumpers**
- **Resistores**:
  - 1 Resistor conhecido de **10 kΩ**
  - Diversos resistores para teste (série E24)

## Pinagem e Conexões

| Componente | Pino BitDogLab |
|:-----------|:---------------|
| Resistor desconhecido (entrada ADC) | GPIO 28 (ADC2) |
| I2C SDA (Display OLED) | GPIO 14 |
| I2C SCL (Display OLED) | GPIO 15 |
| Botão A | GPIO 5 |
| Botão B (BOOTSEL) | GPIO 6 |

## ⚙️ Funcionamento

1. O valor analógico lido no ADC é convertido para uma resistência utilizando a fórmula:

   $\[R_x = \frac{R_{conhecido} \times ADC_{encontrado}}{4095 - ADC_{encontrado}}
   \]$

2. Aplica-se uma correção de **+5%** para compensar imprecisões.
3. A resistência é aproximada para o valor mais próximo da **série E24**.
4. O sistema determina:
   - Primeira faixa (dezena)
   - Segunda faixa (unidade)
   - Terceira faixa (multiplicador)
5. As informações são exibidas no display SSD1306.

## 🔢 Estrutura de Código

- `Ohmimetro01.c` - Código principal que realiza medições, cálculos e atualização do display.
- `val_resistores.h` - Definições dos valores padrão da série E24 e das cores associadas.

## 🚀 Como Compilar e Rodar

1. Abra o projeto utilizando o ambiente de desenvolvimento para RP2040 (ex: **VSCode** com **Pico SDK** configurado).
2. Compile o projeto para gerar o arquivo `.uf2`.
3. Conecte a BitDogLab no modo BOOTSEL (Botão B + conectar USB).
4. Arraste o arquivo `.uf2` para a unidade de armazenamento que aparecerá.

## 🗅 Vídeos de Demonstração

https://www.youtube.com/playlist?list=PLaN_cHSVjBi96Iduv6GiRwuyblX05Qa1h

## ✍️ Autor

- [Daniel Porto Braz](https://github.com/DanielPortoBraz)


