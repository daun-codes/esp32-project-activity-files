#include <Arduino.h>
const int GREEN_LED = 2;
const int YELLOW_LED = 17;

volatile bool secondTimerEvent = false;
volatile bool fiveTimerEvent = false;

// LED TOGGLE

bool greenToggle = false;
bool yellowToggle = false;

unsigned long counter = 0;
unsigned long prevCounter = 0;

unsigned long prevTime = 0;


hw_timer_t *secondTimer = NULL;
hw_timer_t *fiveTimer = NULL;


// -------------------------
// Timer Interrupt
// -------------------------
void ARDUINO_ISR_ATTR onSecondTimer() {
    secondTimerEvent = true;

}

void ARDUINO_ISR_ATTR onFiveTimer() {
    fiveTimerEvent = true;

}


void setup() {

    Serial.begin(115200);

    pinMode(GREEN_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);



    // -------------------------
    // 1-second timer
    // -------------------------

    secondTimer = timerBegin(0, 80, true);
    timerAttachInterrupt(secondTimer, &onSecondTimer, true);
    timerAlarmWrite(secondTimer, 1000000, true);
    timerAlarmEnable(secondTimer);

    // -------------------------
    // 5-second timer
    // -------------------------

    fiveTimer = timerBegin(1, 80, true);
    timerAttachInterrupt(fiveTimer, &onFiveTimer, true);
    timerAlarmWrite(fiveTimer, 5000000, true);
    timerAlarmEnable(fiveTimer);

    Serial.println("System started.");
}


void loop() {


    unsigned long currTime = millis();

    if (secondTimerEvent) {

        // Clear event flag
        secondTimerEvent = false;


        Serial.println("");
        Serial.println("1-Second Event");
        Serial.print("Counter: ");
        counter++;
        Serial.print(counter);

    }

    if (fiveTimerEvent) {

      fiveTimerEvent = false;

      Serial.println("");
      Serial.println("_______________");
      Serial.println("5-Second Event");
      Serial.println("===== STATUS =====");
      Serial.print("Uptime events: ");
      Serial.println(counter);
      Serial.println("==================");

    }

    if (currTime - prevTime >= 500) {
        
        prevTime = currTime;

        greenToggle = !greenToggle;


        digitalWrite(GREEN_LED, greenToggle);
        

        if (counter - prevCounter == 5) {
            
            yellowToggle = !yellowToggle;
            digitalWrite(YELLOW_LED, yellowToggle);
            prevCounter = counter;

        }

        else {
            yellowToggle = false;
            digitalWrite(YELLOW_LED, yellowToggle);
        
        }

            
    }
    
}