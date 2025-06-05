#include <Adafruit_TinyUSB.h>

Adafruit_USBD_MIDI usb_midi;

// Configuração dos botões e LEDs
const uint8_t RED_BUTTON_PIN = 13;
const uint8_t GREEN_BUTTON_PIN = 12;
const uint8_t LED_RED = 8;
const uint8_t LED_GREEN = 9;

// Estrutura para armazenar o estado de cada botão
struct Button {
  uint8_t pin;
  uint8_t ledPin;
  uint8_t note;
  bool lastState;
  bool currentState;
  unsigned long lastDebounceTime;
};

// Inicialização dos botões
Button redButton = {RED_BUTTON_PIN, LED_RED, 60, HIGH, HIGH, 0};
Button greenButton = {GREEN_BUTTON_PIN, LED_GREEN, 62, HIGH, HIGH, 0};

const unsigned long debounceDelay = 50;

void setup() {
  // Inicialização MIDI USB - ESSA ORDEM É IMPORTANTE!
  TinyUSB_Device_Init(0);
  usb_midi.begin();
  
  // Configuração dos botões (pull-up interno)
  pinMode(redButton.pin, INPUT_PULLUP);
  pinMode(greenButton.pin, INPUT_PULLUP);
  
  // Configuração dos LEDs
  pinMode(redButton.ledPin, OUTPUT);
  pinMode(greenButton.ledPin, OUTPUT);
  digitalWrite(redButton.ledPin, LOW);
  digitalWrite(greenButton.ledPin, LOW);
  
  // Aguarda conexão USB
  while (!TinyUSBDevice.mounted()) delay(10);
}

void sendMidiMessage(uint8_t status, uint8_t data1, uint8_t data2) {
  // Envia os três bytes MIDI em sequência
  usb_midi.write(status);
  usb_midi.write(data1);
  usb_midi.write(data2);
  
  // Delay crítico para garantir o envio
  delay(1);
  usb_midi.flush();
}

void flashLed(uint8_t ledPin) {
  digitalWrite(ledPin, HIGH);
  delay(30);
  digitalWrite(ledPin, LOW);
}

void handleButton(Button &btn) {
  bool reading = digitalRead(btn.pin);
  
  // Debounce
  if (reading != btn.lastState) {
    btn.lastDebounceTime = millis();
  }
  
  if ((millis() - btn.lastDebounceTime) > debounceDelay) {
    if (reading != btn.currentState) {
      btn.currentState = reading;
      
      if (btn.currentState == LOW) { // Botão pressionado
        // Feedback visual
        flashLed(btn.ledPin);
        
        // Envia Note On
        sendMidiMessage(0x90, btn.note, 100);
        
        // Delay antes do Note Off
        delay(20);
        
        // Envia Note Off
        sendMidiMessage(0x80, btn.note, 0);
      }
    }
  }
  btn.lastState = reading;
}

void loop() {
  // Processa cada botão
  handleButton(redButton);
  handleButton(greenButton);
  
  // Limpa buffer MIDI de entrada
  while (usb_midi.available()) {
    usb_midi.read();
  }
  
  // Pequeno delay para estabilidade
  delay(10);
}
