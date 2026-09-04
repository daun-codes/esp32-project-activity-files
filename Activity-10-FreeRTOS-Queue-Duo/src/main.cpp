#include <Arduino.h>

QueueHandle_t numberQueue;

// -------------------------
// Producer Task
// -------------------------
void producerTask(void *parameter) {

    int number = 25;

    while (true) {

        xQueueSend(
            numberQueue,
            &number,
            portMAX_DELAY
        );
        Serial.print("Sent: ");
        Serial.println(number);


        number++;

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// -------------------------
// Consumer Task
// -------------------------
void consumerTask(void *parameter) {

    int receivedNumber;

    while (true) {

        if (xQueueReceive(
                numberQueue,
                &receivedNumber,
                portMAX_DELAY
            )) {
            

            Serial.print("Received: ");
            Serial.println(receivedNumber);

        }
    }
}

// -------------------------
// Setup
// -------------------------
void setup() {

    Serial.begin(115200);

    numberQueue = xQueueCreate(
        5,
        sizeof(int)
    );

    if (numberQueue == NULL) {
        Serial.println("Queue creation failed!");
        while (true);
    }

    xTaskCreate(
        producerTask,
        "Producer",
        2048,
        NULL,
        2,
        NULL
    );

    xTaskCreate(
        consumerTask,
        "Consumer",
        2048,
        NULL,
        1,
        NULL
    );
}

void loop() {
}