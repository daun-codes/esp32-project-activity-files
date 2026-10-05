#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ============================================================
// SMART GREENHOUSE CONTROLLER
// Combines ESP32 Lessons 1–9
// ============================================================

// =========================
// PIN DEFINITIONS
// =========================

#define DHT_PIN         4
#define DHT_TYPE        DHT22

#define SOIL_PIN        32
#define LDR_PIN         35

#define BUTTON_PIN      27

#define FAN_PIN         25
#define PUMP_PIN        26

#define LED_PIN         2
#define BUZZER_PIN      18

#define OLED_SDA        21
#define OLED_SCL        22

#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define OLED_RESET      -1

// =========================
// OBJECTS
// =========================

DHT dht(DHT_PIN, DHT_TYPE);

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    OLED_RESET
);

// =========================
// PWM CONFIGURATION
// =========================

const int FAN_PWM_FREQ = 25000;
const int FAN_PWM_RES= 8;

// =========================
// THRESHOLDS
// =========================

const float FAN_ON_TEMP = 30.0;
const float FAN_OFF_TEMP = 28.0;

const float WARNING_TEMP = 35.0;

const int SOIL_DRY_THRESHOLD = 30;
const int SOIL_WET_THRESHOLD = 45;

const int MAX_WATERING_SECONDS = 5;

// =========================
// SENSOR DATA STRUCTURE
// =========================

struct SensorData
{
    float temperature;
    float humidity;

    int soilRaw;
    int soilPercent;

    int lightRaw;
    int lightPercent;
};

// =========================
// SYSTEM STATE
// =========================

struct SystemState
{
    float temperature = 0;
    float humidity = 0;

    int soilPercent = 0;
    int lightPercent = 0;

    int fanSpeed = 0;

    bool fanOn = false;
    bool pumpOn = false;
    bool warning = false;
};

// =========================
// FREERTOS OBJECTS
// =========================

QueueHandle_t sensorQueue;

SemaphoreHandle_t buttonSemaphore;
SemaphoreHandle_t displayMutex;

// =========================
// GLOBAL STATE
// =========================

SystemState systemState;

volatile bool timerEvent = false;

volatile bool buttonInterrupt = false;

int wateringSeconds = 0;

// =========================
// HARDWARE TIMER
// ESP32 Arduino Core 2.0.14
// =========================

hw_timer_t *systemTimer = NULL;

// ============================================================
// HARDWARE TIMER ISR
// ============================================================

void IRAM_ATTR onSystemTimer()
{
    timerEvent = true;
}

// ============================================================
// BUTTON ISR
// ============================================================

void IRAM_ATTR onButtonPress()
{
    buttonInterrupt = true;
}

// ============================================================
// FAN CONTROL
// ============================================================

void setFanSpeed(int speedPercent)
{
    speedPercent = constrain(speedPercent, 0, 100);

    int duty = map(
        speedPercent,
        0,
        100,
        0,
        255
    );

    ledcWrite(
        FAN_PIN,
        duty
    );

    systemState.fanSpeed = speedPercent;

    systemState.fanOn = speedPercent > 0;
}

// ============================================================
// PUMP CONTROL
// ============================================================

void setPump(bool state)
{
    digitalWrite(
        PUMP_PIN,
        state ? HIGH : LOW
    );

    systemState.pumpOn = state;
}

// ============================================================
// SENSOR TASK
// ============================================================

void sensorTask(void *parameter)
{
    SensorData data;

    while (true)
    {
        // ----------------------------------------
        // Read DHT sensor
        // ----------------------------------------

        float temperature = dht.readTemperature();
        float humidity = dht.readHumidity();

        // ----------------------------------------
        // Read analog sensors
        // ----------------------------------------

        int soilRaw = analogRead(SOIL_PIN);
        int lightRaw = analogRead(LDR_PIN);

        // ----------------------------------------
        // Convert soil reading
        //
        // NOTE:
        // This mapping depends on your actual sensor.
        // Calibrate it for your hardware.
        // ----------------------------------------

        int soilPercent = map(
            soilRaw,
            4095,
            1200,
            0,
            100
        );

        soilPercent = constrain(
            soilPercent,
            0,
            100
        );

        // ----------------------------------------
        // Convert light reading
        // ----------------------------------------

        int lightPercent = map(
            lightRaw,
            0,
            4095,
            0,
            100
        );

        lightPercent = constrain(
            lightPercent,
            0,
            100
        );

        // ----------------------------------------
        // Check DHT reading
        // ----------------------------------------

        if (!isnan(temperature) &&
            !isnan(humidity))
        {
            data.temperature = temperature;
            data.humidity = humidity;
        }
        else
        {
            data.temperature = systemState.temperature;
            data.humidity = systemState.humidity;
        }

        data.soilRaw = soilRaw;
        data.soilPercent = soilPercent;

        data.lightRaw = lightRaw;
        data.lightPercent = lightPercent;

        // ----------------------------------------
        // Send sensor data to queue
        // ----------------------------------------

        if (xQueueSend(
                sensorQueue,
                &data,
                pdMS_TO_TICKS(100)
            ) == pdPASS)
        {
            Serial.println(
                "[Sensor Task] Data sent to queue"
            );
        }

        // Sensor sampling interval
        vTaskDelay(
            pdMS_TO_TICKS(2000)
        );
    }
}

// ============================================================
// CONTROL TASK
// ============================================================

void controlTask(void *parameter)
{
    SensorData data;

    while (true)
    {
        // ----------------------------------------
        // Wait for sensor data
        // ----------------------------------------

        if (xQueueReceive(
                sensorQueue,
                &data,
                portMAX_DELAY
            ))
        {
            // ------------------------------------
            // Update shared system state
            // ------------------------------------

            systemState.temperature =
                data.temperature;

            systemState.humidity =
                data.humidity;

            systemState.soilPercent =
                data.soilPercent;

            systemState.lightPercent =
                data.lightPercent;

            // ------------------------------------
            // FAN CONTROL
            // ------------------------------------

            if (data.temperature >= 32.0)
            {
                setFanSpeed(100);
            }
            else if (data.temperature >= 30.0)
            {
                setFanSpeed(70);
            }
            else if (data.temperature >= FAN_ON_TEMP)
            {
                setFanSpeed(40);
            }
            else if (data.temperature <= FAN_OFF_TEMP)
            {
                setFanSpeed(0);
            }

            // ------------------------------------
            // AUTOMATIC IRRIGATION
            // ------------------------------------

            if (data.soilPercent <= SOIL_DRY_THRESHOLD &&
                !systemState.pumpOn)
            {
                Serial.println(
                    "[Control] Soil is dry. Starting pump."
                );

                setPump(true);

                wateringSeconds = 0;
            }

            // ------------------------------------
            // STOP PUMP WHEN SOIL IS WET
            // ------------------------------------

            if (data.soilPercent >= SOIL_WET_THRESHOLD &&
                systemState.pumpOn)
            {
                Serial.println(
                    "[Control] Soil moisture sufficient. Pump OFF."
                );

                setPump(false);

                wateringSeconds = 0;
            }

            // ------------------------------------
            // TEMPERATURE WARNING
            // ------------------------------------

            if (data.temperature >= WARNING_TEMP)
            {
                systemState.warning = true;

                digitalWrite(
                    LED_PIN,
                    HIGH
                );

                tone(
                    BUZZER_PIN,
                    2000
                );
            }
            else
            {
                systemState.warning = false;

                noTone(
                    BUZZER_PIN
                );

                digitalWrite(
                    LED_PIN,
                    LOW
                );
            }

            // ------------------------------------
            // DEBUG OUTPUT
            // ------------------------------------

            Serial.println();
            Serial.println(
                "========== GREENHOUSE =========="
            );

            Serial.print(
                "Temperature: "
            );
            Serial.print(
                data.temperature
            );
            Serial.println(" C");

            Serial.print(
                "Humidity: "
            );
            Serial.print(
                data.humidity
            );
            Serial.println(" %");

            Serial.print(
                "Soil: "
            );
            Serial.print(
                data.soilPercent
            );
            Serial.println(" ");

            Serial.print(
                "Light: "
            );
            Serial.print(
                data.lightPercent
            );
            Serial.println(" %");

            Serial.print(
                "Fan: "
            );
            Serial.print(
                systemState.fanSpeed
            );
            Serial.println(" %");

            Serial.print(
                "Pump: "
            );
            Serial.println(
                systemState.pumpOn
                    ? "ON"
                    : "OFF"
            );

            Serial.println(
                "================================"
            );
        }
    }
}

// ============================================================
// DISPLAY TASK
// ============================================================

void displayTask(void *parameter)
{
    bool screen = false;

    while (true)
    {
        if (xSemaphoreTake(
                displayMutex,
                pdMS_TO_TICKS(100)
            ))
        {
            display.clearDisplay();

            display.setTextColor(
                SSD1306_WHITE
            );

            display.setTextSize(1);

            display.setCursor(0, 0);

            if (!screen)
            {
                // --------------------------------
                // SCREEN 1
                // --------------------------------

                display.println(
                    "SMART GREENHOUSE"
                );

                display.println();

                display.print(
                    "Temp: "
                );
                display.print(
                    systemState.temperature,
                    1
                );
                display.println(" C");

                display.print(
                    "Hum : "
                );
                display.print(
                    systemState.humidity,
                    1
                );
                display.println(" %");

                display.print(
                    "Soil: "
                );
                display.print(
                    systemState.soilPercent
                );
                display.println(" %");

                display.print(
                    "Light:"
                );
                display.print(
                    systemState.lightPercent
                );
                display.println(" %");
            }
            else
            {
                // --------------------------------
                // SCREEN 2
                // --------------------------------

                display.println(
                    "ACTUATOR STATUS"
                );

                display.println();

                display.print(
                    "Fan : "
                );
                display.print(
                    systemState.fanSpeed
                );
                display.println("%");

                display.print(
                    "Pump: "
                );
                display.println(
                    systemState.pumpOn
                        ? "ON"
                        : "OFF"
                );

                display.print(
                    "Mode: AUTO"
                );

                display.println();

                display.println();

                if (systemState.warning)
                {
                    display.println(
                        "!!! WARNING !!!"
                    );
                }
                else
                {
                    display.println(
                        "SYSTEM NORMAL"
                    );
                }
            }

            display.display();

            xSemaphoreGive(
                displayMutex
            );
        }

        screen = !screen;

        vTaskDelay(
            pdMS_TO_TICKS(3000)
        );
    }
}

// ============================================================
// IRRIGATION TASK
// ============================================================

void irrigationTask(void *parameter)
{
    while (true)
    {
        // ----------------------------------------
        // Wait for button event
        // ----------------------------------------

        if (xSemaphoreTake(
                buttonSemaphore,
                portMAX_DELAY
            ))
        {
            Serial.println();
            Serial.println(
                "[Irrigation] Manual watering requested!"
            );

            // ------------------------------------
            // Start pump
            // ------------------------------------

            setPump(true);

            wateringSeconds = 0;

            // ------------------------------------
            // Manual watering duration
            // ------------------------------------

            while (wateringSeconds < 3)
            {
                vTaskDelay(
                    pdMS_TO_TICKS(1000)
                );

                wateringSeconds++;

                Serial.print(
                    "[Irrigation] "
                );

                Serial.print(
                    wateringSeconds
                );

                Serial.println(
                    " second(s)"
                );
            }

            // ------------------------------------
            // Stop pump
            // ------------------------------------

            setPump(false);

            wateringSeconds = 0;

            Serial.println(
                "[Irrigation] Manual watering complete."
            );
        }
    }
}

// ============================================================
// MONITORING TASK
// ============================================================

void monitoringTask(void *parameter)
{
    while (true)
    {
        // ----------------------------------------
        // Check hardware timer event
        // ----------------------------------------

        if (timerEvent)
        {
            timerEvent = false;

            Serial.println(
                "[Timer] 1-second monitoring tick"
            );

            // ------------------------------------
            // Pump safety timer
            // ------------------------------------

            if (systemState.pumpOn)
            {
                wateringSeconds++;

                if (wateringSeconds >=
                    MAX_WATERING_SECONDS)
                {
                    Serial.println(
                        "[Safety] Maximum watering time reached!"
                    );

                    setPump(false);

                    wateringSeconds = 0;
                }
            }
        }

        // ----------------------------------------
        // Check button interrupt event
        // ----------------------------------------

        if (buttonInterrupt)
        {
            buttonInterrupt = false;

            xSemaphoreGive(
                buttonSemaphore
            );
        }

        vTaskDelay(
            pdMS_TO_TICKS(50)
        );
    }
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(115200);

    // ========================================================
    // GPIO
    // ========================================================

    pinMode(
        PUMP_PIN,
        OUTPUT
    );

    pinMode(
        LED_PIN,
        OUTPUT
    );

    pinMode(
        BUZZER_PIN,
        OUTPUT
    );

    pinMode(
        BUTTON_PIN,
        INPUT_PULLUP
    );

    // ========================================================
    // ADC
    // ========================================================

    analogReadResolution(12);

    // ========================================================
    // DHT
    // ========================================================

    dht.begin();

    // ========================================================
    // OLED
    // ========================================================

    Wire.begin(
        OLED_SDA,
        OLED_SCL
    );

    if (!display.begin(
            SSD1306_SWITCHCAPVCC,
            0x3C
        ))
    {
        Serial.println(
            "OLED initialization failed!"
        );

        while (true)
        {
            delay(1000);
        }
    }

    display.clearDisplay();

    display.setTextColor(
        SSD1306_WHITE
    );

    display.setTextSize(1);

    display.setCursor(0, 0);

    display.println(
        "SMART GREENHOUSE"
    );

    display.println();

    display.println(
        "System starting..."
    );

    display.display();

    delay(1000);

    // ========================================================
    // FAN PWM
    // Arduino-ESP32 2.0.14
    // ========================================================

    ledcSetup(0, FAN_PWM_FREQ, FAN_PWM_RES);
    ledcAttachPin(FAN_PIN, 0);
    ledcWrite(0, 0);

    // ========================================================
    // INITIAL ACTUATOR STATES
    // ========================================================

    digitalWrite(
        PUMP_PIN,
        LOW
    );

    digitalWrite(
        LED_PIN,
        LOW
    );

    // ========================================================
    // CREATE QUEUE
    // ========================================================

    sensorQueue = xQueueCreate(
        5,
        sizeof(SensorData)
    );

    if (sensorQueue == NULL)
    {
        Serial.println(
            "Sensor queue creation failed!"
        );

        while (true);
    }

    // ========================================================
    // CREATE SEMAPHORE
    // ========================================================

    buttonSemaphore =
        xSemaphoreCreateBinary();

    if (buttonSemaphore == NULL)
    {
        Serial.println(
            "Button semaphore creation failed!"
        );

        while (true);
    }

    // ========================================================
    // CREATE MUTEX
    // ========================================================

    displayMutex =
        xSemaphoreCreateMutex();

    if (displayMutex == NULL)
    {
        Serial.println(
            "Display mutex creation failed!"
        );

        while (true);
    }

    // ========================================================
    // BUTTON INTERRUPT
    // ========================================================

    attachInterrupt(
        digitalPinToInterrupt(BUTTON_PIN),
        onButtonPress,
        FALLING
    );

    // ========================================================
    // HARDWARE TIMER
    //
    // 80 MHz / 80 = 1 MHz
    // 1 tick = 1 microsecond
    //
    // 1,000,000 us = 1 second
    // ========================================================

    systemTimer = timerBegin(
        0,
        80,
        true
    );

    timerAttachInterrupt(
        systemTimer,
        &onSystemTimer,
        true
    );

    timerAlarmWrite(
        systemTimer,
        1000000,
        true
    );

    timerAlarmEnable(
        systemTimer
    );

    // ========================================================
    // CREATE FREERTOS TASKS
    // ========================================================

    xTaskCreate(
        sensorTask,
        "SensorTask",
        4096,
        NULL,
        2,
        NULL
    );

    xTaskCreate(
        controlTask,
        "ControlTask",
        4096,
        NULL,
        2,
        NULL
    );

    xTaskCreate(
        displayTask,
        "DisplayTask",
        4096,
        NULL,
        1,
        NULL
    );

    xTaskCreate(
        irrigationTask,
        "IrrigationTask",
        4096,
        NULL,
        2,
        NULL
    );

    xTaskCreate(
        monitoringTask,
        "MonitoringTask",
        4096,
        NULL,
        3,
        NULL
    );

    Serial.println();
    Serial.println(
        "================================"
    );

    Serial.println(
        " SMART GREENHOUSE READY"
    );

    Serial.println(
        "================================"
    );
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    // FreeRTOS handles the application tasks.
    // No periodic application work here.

    vTaskDelay(
        pdMS_TO_TICKS(1000)
    );
}