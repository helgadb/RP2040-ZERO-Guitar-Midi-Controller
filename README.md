# RP2040-ZERO-Guitar-Midi-Controller
RP2040-ZERO GUITAR MIDI CONTROLLER

**Código para configuração do RP2040-zero para operar como dispositivo MIDI USB. **

Através da porta USB, o microcontrolador é reconhecido pelo sistema como dispositivo MIDI e pode ser utilizado como controlador MIDI.
O código foi implementado com funções e é facilmente adaptável.
No meu caso, utilizei para criar um controlador com 10 push buttons que enviam comandos MIDI CC; um fader que envia MIDI CC código 7 de volume; e um sensor de distância TCRT5000 que envia MIDI CC 1 de modulação. 
O Fader é utilizado para controlar volume de saída da pedaleira no computador, e o TCRT5000 foi adaptado em um pedal de máquina de costura esvaziado para ser utilizado como pedal de Wah-Wah. 

**Configurações do Arduino IDE: (utilizei a versão 2.3.6 no Windows 11)**

1) Instalar o core Raspberry Pi Pico/RP2040/RP2350 de Earle Philhower de https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
Nos testes utilizei a versão 4.5.4
2) Tools -> Board -> "Waveshare RP2040 Zero"
3) Tools -> USB Stack -> "Adafruit TinyUSB"
4) Escolha a porta adequada ( no meu caso era "UF2_Board")
5) Compilar e enviar o código para o microcontrolador.
