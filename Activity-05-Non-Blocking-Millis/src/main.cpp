#include <Arduino.h>


constexpr uint8_t YELLOW_LED_PIN = 19;
constexpr uint8_t BLUE_LED_PIN = 18;
constexpr uint8_t BUTTON_PIN = 12;

const uint16_t timeInterval = 1000;

unsigned long prevTime = 0;

bool ledState = false;

void setup() {
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

}

void loop() {
  unsigned long currTime = millis();
  int buttonSensor = digitalRead(BUTTON_PIN);


  if (currTime - prevTime >= timeInterval) {
    
    prevTime = currTime;
    ledState = !ledState;
    digitalWrite(BLUE_LED_PIN, ledState);
  }

  if (buttonSensor == LOW) {
    digitalWrite(YELLOW_LED_PIN, HIGH);
  }

  else {
    digitalWrite(YELLOW_LED_PIN, LOW);
  }


}