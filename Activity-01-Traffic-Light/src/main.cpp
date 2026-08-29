#include <Arduino.h>

// Pin Definitions
const int RED_LED_PIN = 21;
const int ORANGE_LED_PIN = 19;
const int GREEN_LED_PIN = 18;

void setup() {
  // Initialize GPIO pins as outputs
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(ORANGE_LED_PIN, OUTPUT);
}

void loop() {
  // Red ON, Orange OFF, Green OFF
  digitalWrite(RED_LED_PIN, HIGH);
  digitalWrite(ORANGE_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
  delay(3000); // Wait 3s


  // Red OFF, Orange ON, Green OFF
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(ORANGE_LED_PIN, HIGH);
  digitalWrite(GREEN_LED_PIN, LOW);
  delay(1000); // Wait 1s


  // Red OFF, Orange OFF, Green ON
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(ORANGE_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, HIGH);
  delay(3000); // Wait 3s


}