#include <Arduino.h>

int counter = 1;
int prevCounter = 0;


void taskCounter(void *parameter) {

    while (true) {

        
        vTaskDelay(pdMS_TO_TICKS(1000));
        if (counter - prevCounter >= 3) {

            prevCounter = counter;
        }

        else {
            Serial.println("");
            Serial.print("Counter: ");
            Serial.print(counter);
        }

        counter++;
        

        
    }
}

void taskStatus(void *parameter) {

    while (true) {

        vTaskDelay(pdMS_TO_TICKS(3000));
        Serial.println("");
        Serial.println("");
        Serial.println("--- SYSTEM STATUS ---");
        Serial.println(" System is running.. ");
        Serial.println(" Counter: " + String(counter - 1));
        Serial.println("---------------------");
        

    }
}

void setup() {

    Serial.begin(115200);

    
    
    xTaskCreate(
        taskCounter,
        "HelloTask",
        2048,
        NULL,
        1,
        NULL
    );

    xTaskCreate(
        taskStatus,
        "StatusTask",
        2048,
        NULL,
        1,
        NULL
    );


}

void loop() {

}
