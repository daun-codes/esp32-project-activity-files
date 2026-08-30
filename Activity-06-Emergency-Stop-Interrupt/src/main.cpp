#include <Arduino.h>


constexpr uint8_t BUTTON_PIN = 14;
constexpr uint8_t GREEN_LED_PIN = 18;
constexpr uint8_t RED_LED_PIN = 19;

volatile unsigned long prevTime = 0; 

volatile bool systemTripped = false;

constexpr uint16_t greenTime = 1000;
constexpr uint16_t timeReset = 2000;
bool greenLedOn = false;
bool emergencyStop = false;


void emergencyState() {
  
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, HIGH);
  

}


void normalState(bool ledState) {
  if (emergencyStop == false) {
    digitalWrite(GREEN_LED_PIN, ledState);
    digitalWrite(RED_LED_PIN, LOW);
  }
}


  

void IRAM_ATTR myEmergencyISR() {
  
  systemTripped = true;

}


void setup() {
  Serial.begin(115200);
  delay(1000);


  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), myEmergencyISR, FALLING);

  Serial.println("=== Emergency Stop System ===");
  Serial.println("Press the button to trigger emergency stop!");
}

void loop() {

  volatile unsigned long currTime = millis();

  if (systemTripped == false) {
    
    emergencyStop = false;
    if (currTime - prevTime >= greenTime) {
    prevTime = currTime;

    normalState(greenLedOn);
    greenLedOn = !greenLedOn;

  }

  }

  else if (systemTripped == true) {
    
    emergencyState();

    if (currTime - prevTime >= timeReset) {
      systemTripped = false;
      emergencyStop = true;
      prevTime = currTime;
    }
  }
}