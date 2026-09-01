#include <Arduino.h>


constexpr uint8_t BUTTON_PIN = 14;
constexpr uint8_t GREEN_LED_PIN = 18;
constexpr uint8_t RED_LED_PIN = 19;

volatile unsigned long prevTime = 0; 

volatile bool systemTripped = false;

constexpr uint16_t greenTime = 1000;
constexpr uint16_t timeReset = 5000;
constexpr uint16_t resetCountdown = 1000;


int resetCounter = 0;
bool greenLedOn = false;
bool displayCountdown = true;

void emergencyState() {
  
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, HIGH);
  

}


void normalState(bool ledState) {
  digitalWrite(GREEN_LED_PIN, ledState);
  digitalWrite(RED_LED_PIN, LOW);

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
    
    if (currTime - prevTime >= greenTime) {
    prevTime = currTime;

    normalState(greenLedOn);
    greenLedOn = !greenLedOn;

  }

  }

  else if (systemTripped == true) {
    
    emergencyState();
  

    if (currTime - prevTime >= resetCountdown) {
      
      
      if (displayCountdown == true) {
        Serial.println("");
        Serial.println("EMERGENCY TRIGGERED!");
        Serial.println("RESET IN 5 SECONDS");
        displayCountdown = false;
      }

      Serial.print("RESETTING IN: ");
      Serial.println((timeReset - resetCounter) / 1000);
      resetCounter = resetCounter + 1000;

      if (resetCounter >= timeReset) {
        displayCountdown = true;
        resetCounter = 0;
        systemTripped = false;
        
      }
    
      prevTime = currTime;

      

    }
  }
}