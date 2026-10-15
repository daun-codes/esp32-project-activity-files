#include <Arduino.h>
constexpr uint8_t LED_PIN = 18;
constexpr uint8_t BUTTON_PIN = 15;
constexpr uint8_t LEDC_CHANNEL = 0;
constexpr uint32_t PWM_FREQ = 5000; // 5 kHz frequency
constexpr uint8_t PWM_RESOLUTION = 8; // 8-bit resolution (0-255)

byte state = 0;

void setup() {
  
  ledcSetup(LEDC_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED_PIN, LEDC_CHANNEL);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.begin(115200);
}

void loop() {
  

  byte buttonSignal = digitalRead(BUTTON_PIN);

  if (buttonSignal == LOW) {
    state = (state + 1) % 4;

    switch(state) {
      case 0:
        ledcWrite(LEDC_CHANNEL, 0);
        Serial.println("LED POWERED OFF");
        break;

      case 1:
        ledcWrite(LEDC_CHANNEL, 50);
        Serial.println("LED POWER LEVEL 1");
        break;

      case 2:
        ledcWrite(LEDC_CHANNEL, 150);
        Serial.println("LED POWER LEVEL 2");
        break;

      case 3:
        ledcWrite(LEDC_CHANNEL, 255);
        Serial.println("LED POWER LEVEL 3");
        break;
    }

    delay(200);
  }

}