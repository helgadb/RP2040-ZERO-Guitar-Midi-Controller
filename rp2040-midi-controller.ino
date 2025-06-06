#include <Adafruit_TinyUSB.h>

Adafruit_USBD_MIDI usb_midi;

// Configuração dos pinos
const uint8_t RED_BUTTON_PIN = 13;
const uint8_t GREEN_BUTTON_PIN = 12;
const uint8_t LED_RED_PIN = 8;
const uint8_t LED_GREEN_PIN = 9;
const uint8_t LED_BLUE_PIN = 10;
const int DISTANCE_SENSOR_PIN = 26;
const int FADER_PIN = 27;

// Definições MIDI
const uint8_t MIDI_CC_VOLUME = 7;
const uint8_t MIDI_CC_MODULATION = 1;
const uint8_t MIDI_CHANNEL = 0;

// Estrutura aprimorada para botões com controle RGB
struct Button {
  uint8_t pin;
  uint8_t note;
  bool lastState;
  bool currentState;
  unsigned long lastDebounceTime;
  
  // Configuração de cor RGB
  uint8_t redIntensity;
  uint8_t greenIntensity;
  uint8_t blueIntensity;
  unsigned long ledFlashDuration;
};

// Estrutura para controles analógicos
struct AnalogConfig {
  uint8_t pin;
  int minRaw;
  int maxRaw;
  bool invertMapping;
  bool useEMA;
  float alpha;
  int lastValue;
  unsigned long lastReadTime;
  unsigned long debounceDelay;
  uint8_t ccNumber;
  int lastSentValue;
  int sendThreshold;
  uint8_t redIntensity;
  uint8_t greenIntensity;
  uint8_t blueIntensity;
  unsigned long ledFlashDuration;
};

// Inicialização dos botões com cores RGB
Button redButton = {
  RED_BUTTON_PIN, // pin
  60,             // note
  HIGH, HIGH, 0,  // lastState, currentState, lastDebounceTime
  255, 0, 0,      // redIntensity, greenIntensity, blueIntensity (Vermelho)
  30              // ledFlashDuration (ms)
};

Button greenButton = {
  GREEN_BUTTON_PIN, // pin
  62,               // note
  HIGH, HIGH, 0,    // lastState, currentState, lastDebounceTime
  0, 255, 0,        // redIntensity, greenIntensity, blueIntensity (Verde)
  30                // ledFlashDuration (ms)
};

// Configuração dos sensores analógicos
AnalogConfig distanceSensor = {
  DISTANCE_SENSOR_PIN, 65, 260, true, true, 0.8,
  0, 0, 10, MIDI_CC_MODULATION, -1, 7,
  0, 0, 255, 15  // Azul
};

AnalogConfig fader = {
  FADER_PIN, 10, 1015, false, false, 0.8,
  0, 0, 10, MIDI_CC_VOLUME, -1, 1,
  255, 255, 0, 15  // Amarelo
};

const unsigned long buttonDebounceDelay = 50;

void setup() {
  // Inicialização MIDI USB
  TinyUSB_Device_Init(0);
  usb_midi.begin();
  
  // Configuração dos botões
  pinMode(redButton.pin, INPUT_PULLUP);
  pinMode(greenButton.pin, INPUT_PULLUP);
  
  // Configuração dos LEDs RGB
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, LOW);
  digitalWrite(LED_BLUE_PIN, LOW);
  
  // Aguarda conexão USB
  while (!TinyUSBDevice.mounted()) delay(10);
}

int readAnalog(AnalogConfig &config) {
  unsigned long currentTime = millis();
  if (currentTime - config.lastReadTime < config.debounceDelay) {
    return config.lastValue;
  }
  
  int rawValue = analogRead(config.pin);
  if (config.useEMA) {
    rawValue = config.alpha * rawValue + (1 - config.alpha) * config.lastValue;
  }
  rawValue = constrain(rawValue,  config.minRaw,  config.maxRaw);
  int mappedValue = config.invertMapping 
    ? map(rawValue, config.minRaw, config.maxRaw, 127, 0)
    : map(rawValue, config.minRaw, config.maxRaw, 0, 127);
  
  config.lastValue = mappedValue;
  config.lastReadTime = currentTime;
  
  return mappedValue;
}

void flashRGBLed(uint8_t red, uint8_t green, uint8_t blue, unsigned long duration) {
  analogWrite(LED_RED_PIN, red);
  analogWrite(LED_GREEN_PIN, green);
  analogWrite(LED_BLUE_PIN, blue);
  delay(duration);
  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, LOW);
  digitalWrite(LED_BLUE_PIN, LOW);
}

void sendControlChange(AnalogConfig &config) {
  int currentValue = readAnalog(config);
  if (abs(currentValue - config.lastSentValue) > config.sendThreshold ) {
    sendMidiMessage(0xB0 | MIDI_CHANNEL, config.ccNumber, currentValue);
    config.lastSentValue = currentValue;
    flashRGBLed(config.redIntensity, config.greenIntensity, config.blueIntensity, config.ledFlashDuration);
  }
}

void handleButton(Button &btn) {
  bool reading = digitalRead(btn.pin);
  
  if (reading != btn.lastState) {
    btn.lastDebounceTime = millis();
  }
  
  if ((millis() - btn.lastDebounceTime) > buttonDebounceDelay) {
    if (reading != btn.currentState) {
      btn.currentState = reading;
      
      if (btn.currentState == LOW) {
        // Aciona LED com a cor específica do botão
        flashRGBLed(btn.redIntensity, btn.greenIntensity, btn.blueIntensity, btn.ledFlashDuration);
        
        // Envia mensagens MIDI
        sendMidiMessage(0x90 | MIDI_CHANNEL, btn.note, 100);
        delay(20);
        sendMidiMessage(0x80 | MIDI_CHANNEL, btn.note, 0);
      }
    }
  }
  btn.lastState = reading;
}

void sendMidiMessage(uint8_t status, uint8_t data1, uint8_t data2) {
  usb_midi.write(status);
  usb_midi.write(data1);
  usb_midi.write(data2);
  delay(1);
  usb_midi.flush();
}

void loop() {
  handleButton(redButton);    // Botão vermelho (LED vermelho)
  handleButton(greenButton);  // Botão verde (LED verde)
  
  sendControlChange(fader);         // Fader (LED amarelo)
  sendControlChange(distanceSensor); // Sensor (LED azul)
  
  while (usb_midi.available()) {
    usb_midi.read();
  }
  
  delay(10);
}
