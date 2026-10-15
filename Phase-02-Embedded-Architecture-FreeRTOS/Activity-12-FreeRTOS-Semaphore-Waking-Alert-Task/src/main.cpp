#include <Arduino.h>

constexpr uint8_t BUTTON_PIN = 14;
constexpr uint8_t RESET_PIN  = 13;
constexpr uint8_t LED_PIN    = 23;


constexpr uint8_t BUZZER_PIN = 18;
constexpr uint8_t BUZZER_CHANNEL = 0;
constexpr uint16_t BUZZER_FREQ   = 500; // 500 Hz
constexpr uint8_t BUZZER_RES     = 8;

volatile bool isActive = false;
SemaphoreHandle_t mySemaphore;



void buzzerOn(uint16_t freq =500) {
    ledcWriteTone(BUZZER_CHANNEL, freq);
}


void buzzerOff() {
    ledcWriteTone(BUZZER_CHANNEL, 0);
}
// Task 1: Producer (Start/Toggle Button on GPIO 14)
void taskProducer(void *pvParameters) {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    bool lastState = HIGH;

    for (;;) {
        bool currentState = digitalRead(BUTTON_PIN);

        // Falling edge: Button Press (HIGH to LOW)
        if (lastState == HIGH && currentState == LOW) {
            Serial.println("[Producer] Toggle Button Pressed! Giving semaphore...");
            isActive = true;
            xSemaphoreGive(mySemaphore);
            vTaskDelay(pdMS_TO_TICKS(200)); // Debounce
        }

        lastState = currentState;
        vTaskDelay(pdMS_TO_TICKS(20)); // Small delay to prevent CPU hogging
    }
}

// Task 2: Reset Task (Forces System OFF on GPIO 13)
void resetTask(void *pvParameters) {
    pinMode(RESET_PIN, INPUT_PULLUP);
    bool lastState = HIGH;


    for (;;) {
        bool currentState = digitalRead(RESET_PIN);

        // Falling edge: Button Press (HIGH to LOW)
        if (lastState == HIGH && currentState == LOW) {
            Serial.println("[Reset] Reset Button Pressed! Forcing system OFF...");
            
            isActive = false;
            
            
            
            buzzerOn(500);
            vTaskDelay(pdMS_TO_TICKS(100));
            buzzerOff();

           

            digitalWrite(LED_PIN, LOW); // Turn off LED immediately

            
            
            vTaskDelay(pdMS_TO_TICKS(200)); // Debounce
        }

        lastState = currentState;
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

// Task 3: Consumer (Handles Non-Blocking LED Blinking State)
void taskConsumer(void *pvParameters) {
    unsigned long prevTime = 0;
    const unsigned long interval = 500;

    pinMode(LED_PIN, OUTPUT);

    for (;;) {
        // Non-blocking check for semaphore (timeout = 0 ticks)
        if (xSemaphoreTake(mySemaphore, 0) == pdTRUE) {
            Serial.println("  --> [Consumer] Semaphore taken! Toggling active state...\n");

            if (!isActive) {
                isActive = true;
            }
            
            
        }

        // Background non-blocking LED blinking
        if (isActive) {
            unsigned long currTime = millis();
            if (currTime - prevTime >= interval) {
                prevTime = currTime;
                digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Toggle LED pin state

                if (digitalRead(LED_PIN) == HIGH) {
                    buzzerOn(BUZZER_FREQ);
                } else {
                    buzzerOff();
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10)); // Required for FreeRTOS scheduler yield
    }
}

void setup() {
    Serial.begin(115200);
    vTaskDelay(pdMS_TO_TICKS(1000));

    Serial.println("\n--- Multi-Task Semaphore & Reset Controller ---");

    // Initialize LEDC para sa Buzzer
    ledcSetup(BUZZER_CHANNEL, BUZZER_FREQ, BUZZER_RES);
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
    buzzerOff();

    mySemaphore = xSemaphoreCreateCounting(5, 0);

    if (mySemaphore != NULL) {
        // Task 1: Producer
        xTaskCreatePinnedToCore(taskProducer, "ProducerTask", 2048, NULL, 1, NULL, 1);

        // Task 2: Reset Task
        xTaskCreatePinnedToCore(resetTask, "ResetTask", 2048, NULL, 1, NULL, 1);

        // Task 3: Consumer
        xTaskCreatePinnedToCore(taskConsumer, "ConsumerTask", 2048, NULL, 2, NULL, 1);
    } else {
        Serial.println("Failed to create Semaphore!");
    }
}

void loop() {
    vTaskDelete(NULL);
}