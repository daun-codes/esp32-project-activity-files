#include <Arduino.h>

constexpr uint8_t RED_LED_PIN = 21; // We use uint8_t to store the led pin number to a variable in a constant expression to save more memory and storage 
constexpr uint8_t GREEN_LED_PIN = 18;
constexpr uint8_t BUTTON_PIN = 17;

void setTrafficLight(bool red, bool green) {
  digitalWrite(RED_LED_PIN, red);
  digitalWrite(GREEN_LED_PIN, green);
}

void setup() {
  Serial.begin(115200);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Default state: Green light for traffic
  setTrafficLight(LOW, HIGH);
  Serial.println("Traffic: GO (Green)");
}

void loop() {
  int buttonState = digitalRead(BUTTON_PIN);

  // Check if pedestrian pressed the button
  if (buttonState == LOW) {
    Serial.println("Pedestrian button pressed! Changing lights in 1s...");
    delay(1000); // 1-second reaction delay for vehicles

    // Switch to STOP for traffic
    setTrafficLight(HIGH, LOW);
    Serial.println("Traffic: STOP (Red) - Pedestrians Crossing...");
    delay(4000); // Allow 4 seconds to cross

    // Reset back to normal GO state
    setTrafficLight(LOW, HIGH);
    Serial.println("Traffic: GO (Green)");
  }
}

