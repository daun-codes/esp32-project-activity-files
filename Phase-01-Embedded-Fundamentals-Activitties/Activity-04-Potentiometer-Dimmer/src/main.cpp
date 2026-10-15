#include <Arduino.h>


constexpr uint8_t POT_PIN = 34;
constexpr uint8_t RED_LED_PIN = 21;
constexpr uint8_t GREEN_LED_PIN = 18;

constexpr uint32_t PWM_FREQ = 5000;
constexpr uint8_t PWM_RESOLUTION = 8;

void setup() {
  pinMode(POT_PIN, INPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  ledcAttach(GREEN_LED_PIN, PWM_FREQ, PWM_RESOLUTION);
  Serial.begin(115200);

}

void loop() {
  
  int rawValue = analogRead(POT_PIN);

  int greenBrightness = map(rawValue, 0, 4095, 0, 255);

  ledcWrite(GREEN_LED_PIN, greenBrightness);
  
  if (rawValue > 3000) {
    Serial.println("WARNING! TOO MUCH BRIGHTNESS!");
    digitalWrite(RED_LED_PIN, HIGH);
  }

  else {
    digitalWrite(RED_LED_PIN, LOW);
  }

  delay(100);

}
