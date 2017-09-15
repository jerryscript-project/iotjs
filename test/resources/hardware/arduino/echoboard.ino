#include <SPI.h>

const int GPIO_IN = 7;
const int GPIO_OUT = 8;
const int MISO_PIN = 12;
const int PWM_PIN = 9;
const int PWM_MODE_1_PIN = 5;
const int PWM_MODE_2_PIN = 6;
const int ADC_PIN = A0;
const int ADC_MODE_ON_PIN = 4;
const int ADC_MODE_OK_PIN = 2;
const int ADC_MODE_FULL_PIN = 3;
const int LOOP_DELAY = 1000;
const int SERIAL_BUFFER_LENGTH = 12;
int gpioVal = 0;
int pwmMode1 = 0;
int pwmMode2 = 0;
int adcMode = 0;
int adcVal = 0;
char serialBuffer[SERIAL_BUFFER_LENGTH];
int serialBufferIndex = 0;
volatile boolean serialFinished = false;

void setup() {
  Serial.begin(115200);
  // GPIO TEST SETUP
  pinMode(GPIO_IN, INPUT);
  pinMode(GPIO_OUT, OUTPUT);

  // SPI TEST SETUP
  pinMode(MISO_PIN, OUTPUT);
  SPCR |= _BV(SPE);
  SPCR |= _BV(SPIE);
  SPI.attachInterrupt();

  // PWM TEST SETUP
  pinMode(PWM_PIN, OUTPUT);
  pinMode(PWM_MODE_1_PIN, INPUT);
  pinMode(PWM_MODE_2_PIN, INPUT);

  // ADC TEST SETUP
  pinMode(ADC_PIN, INPUT);
  pinMode(ADC_MODE_ON_PIN, INPUT);
  pinMode(ADC_MODE_FULL_PIN, OUTPUT);
  pinMode(ADC_MODE_OK_PIN, OUTPUT);
  analogReference(EXTERNAL);
}

// @TEST: SPI ECHO
ISR (SPI_STC_vect) {
  byte c = SPDR; // grab data
  SPDR = c; // send back
}

void serialEvent() {
  if (Serial.available()) {
    if (serialFinished) {
      serialFinished = false;
      serialBufferIndex = 0;
    }

    while (Serial.available()) {
      char in = (char) Serial.read();
      if (in != '\n') {
        serialBuffer[serialBufferIndex++] = in;
      } else {
        serialFinished = true;
      }
    }
  }
}

void loop(){
  // @TEST: GPIO ECHO
  gpioVal = digitalRead(GPIO_IN);
  if (gpioVal != 0) {
    digitalWrite(GPIO_OUT, gpioVal);
    delay(LOOP_DELAY);
    digitalWrite(GPIO_OUT, !gpioVal);
  }

  // @TEST: PWM AS VOLTAGE
  pwmMode1 = digitalRead(PWM_MODE_1_PIN);
  pwmMode2 = digitalRead(PWM_MODE_2_PIN);
  if (pwmMode1) {
    analogWrite(PWM_PIN, 128);
  } else if (pwmMode2) {
    analogWrite(PWM_PIN, 255);
  } else {
    analogWrite(PWM_PIN, 0);
  }

  // @TEST: VOLTAGE FROM ADC
  adcMode = digitalRead(ADC_MODE_ON_PIN);
  if (adcMode) { // turn of testing of adc
    adcVal = analogRead(ADC_PIN);
    if (adcVal > 450 && adcVal < 600) {
      digitalWrite(ADC_MODE_FULL_PIN, 1);
      delay(LOOP_DELAY);
      adcVal = analogRead(ADC_PIN);
      if (adcVal > 900) {
        digitalWrite(ADC_MODE_OK_PIN, 1);
        delay(LOOP_DELAY);
      }
    }
    adcMode = 0;
    digitalWrite(ADC_MODE_OK_PIN, 0);
    digitalWrite(ADC_MODE_FULL_PIN, 0);
  }

  if (serialFinished) {
    Serial.write(serialBuffer);
    serialFinished = false;
    serialBufferIndex = 0;
  }

}

