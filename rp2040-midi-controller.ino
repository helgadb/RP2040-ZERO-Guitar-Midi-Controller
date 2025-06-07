#include <Adafruit_TinyUSB.h>

Adafruit_USBD_MIDI usb_midi;

// Configuração dos pinos
const uint8_t LYELLOW_BUTTON_PIN = 14;  // Botão para CC#23
const uint8_t LBLUE_BUTTON_PIN = 15;    // Botão para CC#24
const uint8_t LGREEN_BUTTON_PIN = 26;
const uint8_t LRED_BUTTON_PIN = 27;
const int LFADER_PIN = 28;
const int LDISTANCE_SENSOR_PIN = 29;

const uint8_t RRED_BUTTON_PIN = 3;
const uint8_t RGREEN_BUTTON_PIN = 4;
const uint8_t RWHITE_BUTTON_PIN = 5;
const uint8_t RBLACK_BUTTON_PIN = 6;
const uint8_t RYELLOW_BUTTON_PIN = 7;
const uint8_t RBLUE_BUTTON_PIN = 8;

const uint8_t BLED_RED_PIN = 9;
const uint8_t BLED_GREEN_PIN = 10;
const uint8_t BLED_BLUE_PIN = 11;

// Definições MIDI
const uint8_t MIDI_CC_VOLUME = 7;
const uint8_t MIDI_CC_MODULATION = 1;
const uint8_t MIDI_CHANNEL = 0;

// Estrutura para botões com controle MIDI personalizado
struct Button {
  uint8_t pin;
  bool isCC;          // true para CC, false para Note
  uint8_t controlNumber; // Número do controle ou nota
  bool lastState;
  bool currentState;
  unsigned long lastDebounceTime;
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

// Inicialização dos botões
Button lYellowButton = {
  LYELLOW_BUTTON_PIN, // pin
  true,               // isCC (envia mensagem CC)
  23,                 // controlNumber (CC#23)
  HIGH, HIGH, 0,      // lastState, currentState, lastDebounceTime
  255, 255, 0,        // Amarelo (R, G, B)
  30                  // ledFlashDuration (ms)
};

Button lBlueButton = {
  LBLUE_BUTTON_PIN,   // pin
  true,               // isCC (envia mensagem CC)
  24,                 // controlNumber (CC#24)
  HIGH, HIGH, 0,      // lastState, currentState, lastDebounceTime
  0, 0, 255,          // Azul (R, G, B)
  30                  // ledFlashDuration (ms)
};

Button lGreenButton = {
  LGREEN_BUTTON_PIN,  // pin
  true,              // isCC (envia nota)
  26,                 // controlNumber (nota MIDI)
  HIGH, HIGH, 0,      // lastState, currentState, lastDebounceTime
  0, 255, 0,          // Verde (R, G, B)
  30                  // ledFlashDuration (ms)
};

Button lRedButton = {
  LRED_BUTTON_PIN,    // pin
  true,              // isCC (envia nota)
  27,                 // controlNumber (nota MIDI)
  HIGH, HIGH, 0,      // lastState, currentState, lastDebounceTime
  255, 0, 0,          // Vermelho (R, G, B)
  30                  // ledFlashDuration (ms)
};

// Botões da direita (pinos 3-8) agora enviam CC 16-21
Button rRedButton = {
  RRED_BUTTON_PIN,    // pin
  true,               // isCC (envia mensagem CC)
  17,                 // controlNumber (CC#17)
  HIGH, HIGH, 0,      // lastState, currentState, lastDebounceTime
  255, 0, 0,          // Vermelho (R, G, B)
  30                  // ledFlashDuration (ms)
};

Button rGreenButton = {
  RGREEN_BUTTON_PIN,  // pin
  true,               // isCC (envia mensagem CC)
  16,                 // controlNumber (CC#16)
  HIGH, HIGH, 0,      // lastState, currentState, lastDebounceTime
  0, 255, 0,          // Verde (R, G, B)
  30                  // ledFlashDuration (ms)
};

Button rWhiteButton = {
  RWHITE_BUTTON_PIN,  // pin
  true,               // isCC (envia mensagem CC)
  19,                 // controlNumber (CC#19)
  HIGH, HIGH, 0,      // lastState, currentState, lastDebounceTime
  255, 255, 255,      // Branco (R, G, B)
  30                  // ledFlashDuration (ms)
};

Button rBlackButton = {
  RBLACK_BUTTON_PIN,  // pin
  true,               // isCC (envia mensagem CC)
  18,                 // controlNumber (CC#18)
  HIGH, HIGH, 0,      // lastState, currentState, lastDebounceTime
  128, 128, 128,      // Cinza 
  30                  // ledFlashDuration (ms)
};

Button rYellowButton = {
  RYELLOW_BUTTON_PIN, // pin
  true,               // isCC (envia mensagem CC)
  20,                 // controlNumber (CC#20)
  HIGH, HIGH, 0,      // lastState, currentState, lastDebounceTime
  255, 255, 0,        // Amarelo (R, G, B)
  30                  // ledFlashDuration (ms)
};

Button rBlueButton = {
  RBLUE_BUTTON_PIN,   // pin
  true,               // isCC (envia mensagem CC)
  21,                 // controlNumber (CC#21)
  HIGH, HIGH, 0,      // lastState, currentState, lastDebounceTime
  0, 0, 255,          // Azul (R, G, B)
  30                  // ledFlashDuration (ms)
};

// Configuração dos sensores analógicos
AnalogConfig distanceSensor = {
  LDISTANCE_SENSOR_PIN, // pin
  65, 260,            // minRaw, maxRaw
  true,               // invertMapping
  true,               // useEMA
  0.8,                // alpha
  0,                  // lastValue
  0,                  // lastReadTime
  10,                 // debounceDelay (ms)
  MIDI_CC_MODULATION, // ccNumber
  -1,                 // lastSentValue
  7,                  // sendThreshold
  128, 0, 128,          // Roxo (R, G, B)
  15                  // ledFlashDuration (ms)
};

AnalogConfig fader = {
  LFADER_PIN,         // pin
  10, 1015,          // minRaw, maxRaw
  false,              // invertMapping
  false,              // useEMA
  0.8,                // alpha
  0,                  // lastValue
  0,                  // lastReadTime
  10,                 // debounceDelay (ms)
  MIDI_CC_VOLUME,     // ccNumber
  -1,                 // lastSentValue
  1,                  // sendThreshold
  255, 127, 0,        // Laranja (R, G, B)
  15                  // ledFlashDuration (ms)
};

const unsigned long buttonDebounceDelay = 50;

void setup() {
  // Inicialização MIDI USB
  TinyUSB_Device_Init(0);
  usb_midi.begin();
  
  // Configuração de todos os botões
  pinMode(lYellowButton.pin, INPUT_PULLUP);
  pinMode(lBlueButton.pin, INPUT_PULLUP);
  pinMode(lGreenButton.pin, INPUT_PULLUP);
  pinMode(lRedButton.pin, INPUT_PULLUP);
  pinMode(rRedButton.pin, INPUT_PULLUP);
  pinMode(rGreenButton.pin, INPUT_PULLUP);
  pinMode(rWhiteButton.pin, INPUT_PULLUP);
  pinMode(rBlackButton.pin, INPUT_PULLUP);
  pinMode(rYellowButton.pin, INPUT_PULLUP);
  pinMode(rBlueButton.pin, INPUT_PULLUP);
  
  // Configuração dos LEDs RGB
  pinMode(BLED_RED_PIN, OUTPUT);
  pinMode(BLED_GREEN_PIN, OUTPUT);
  pinMode(BLED_BLUE_PIN, OUTPUT);
  digitalWrite(BLED_RED_PIN, LOW);
  digitalWrite(BLED_GREEN_PIN, LOW);
  digitalWrite(BLED_BLUE_PIN, LOW);
  
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
  rawValue = constrain(rawValue, config.minRaw, config.maxRaw);
  int mappedValue = config.invertMapping 
    ? map(rawValue, config.minRaw, config.maxRaw, 127, 0)
    : map(rawValue, config.minRaw, config.maxRaw, 0, 127);
  
  config.lastValue = mappedValue;
  config.lastReadTime = currentTime;
  
  return mappedValue;
}

void flashRGBLed(uint8_t red, uint8_t green, uint8_t blue, unsigned long duration) {
  analogWrite(BLED_RED_PIN, red);
  analogWrite(BLED_GREEN_PIN, green);
  analogWrite(BLED_BLUE_PIN, blue);
  delay(duration);
  digitalWrite(BLED_RED_PIN, LOW);
  digitalWrite(BLED_GREEN_PIN, LOW);
  digitalWrite(BLED_BLUE_PIN, LOW);
}

void sendControlChange(AnalogConfig &config) {
  int currentValue = readAnalog(config);
  if (abs(currentValue - config.lastSentValue) > config.sendThreshold) {
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
        
        // Envia mensagem MIDI conforme o tipo do botão
        if (btn.isCC) {
          // Envia Control Change com valor 127 (0x7F)
          sendMidiMessage(0xB0 | MIDI_CHANNEL, btn.controlNumber, 0x7F);
        } else {
          // Comportamento original para notas
          sendMidiMessage(0x90 | MIDI_CHANNEL, btn.controlNumber, 100);
          delay(20);
          sendMidiMessage(0x80 | MIDI_CHANNEL, btn.controlNumber, 0);
        }
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
  // Processa todos os botões
  handleButton(lYellowButton);  // Envia CC#23 (0xB0 0x17 0x7F)
  handleButton(lBlueButton);    // Envia CC#24 (0xB0 0x18 0x7F)
  handleButton(lGreenButton);
  handleButton(lRedButton);
  handleButton(rRedButton);     // Envia CC#17
  handleButton(rGreenButton);   // Envia CC#16
  handleButton(rWhiteButton);   // Envia CC#19
  handleButton(rBlackButton);   // Envia CC#18
  handleButton(rYellowButton);  // Envia CC#20
  handleButton(rBlueButton);    // Envia CC#21
  
  // Processa controles analógicos
  sendControlChange(fader);
  sendControlChange(distanceSensor);
  
  // Limpa buffer MIDI de entrada
  while (usb_midi.available()) {
    usb_midi.read();
  }
  
  delay(10);
}
