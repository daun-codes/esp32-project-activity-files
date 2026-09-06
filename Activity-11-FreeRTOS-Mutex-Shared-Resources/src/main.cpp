#include <Arduino.h>

SemaphoreHandle_t resourceMutex;


unsigned long prevTime = 0;


void taskA(void *parameter) {
    while (true) {

        if (xSemaphoreTake(resourceMutex, portMAX_DELAY)) {


            unsigned long currTime = millis();

            Serial.println("Machine A: Using resource...");
            vTaskDelay(pdMS_TO_TICKS(500));
            
            Serial.println("Machine A: Finished.");

            xSemaphoreGive(resourceMutex);

        }

        vTaskDelay(pdMS_TO_TICKS(2000));

        
    }
}

void taskB(void *parameter) {
    while (true) {

        if (xSemaphoreTake(resourceMutex, portMAX_DELAY)) {

            Serial.println("Machine B: Using resource...");
            vTaskDelay(pdMS_TO_TICKS(500));
            Serial.println("Machine B: Finished.");

            xSemaphoreGive(resourceMutex);


        }

        
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void setup() {
    Serial.begin(115200);

    resourceMutex = xSemaphoreCreateMutex();

    if (resourceMutex == NULL) {
        Serial.println("Mutex creation failed!");
        while (true);
    }

    xTaskCreate(taskA, "TaskA", 2048, NULL, 1, NULL);
    xTaskCreate(taskB, "TaskB", 2048, NULL, 1, NULL);
}

void loop() {}