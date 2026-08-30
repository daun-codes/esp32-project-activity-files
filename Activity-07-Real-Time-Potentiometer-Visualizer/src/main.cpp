#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)

#define analogPin 34 // Pin for analog input (e.g., potentiometer)


const uint8_t timeRefreshRate = 100; // Refresh rate for time display in milliseconds

// Create display object using I2C (&Wire)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);

  // Initialize display with I2C address 0x3C
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed!"));
    for (;;); // Don't proceed, loop forever
  }

  display.clearDisplay(); // Clear startup logo buffer


}

void loop() {
  static unsigned long prevTime = 0;

  unsigned long currTime = millis();

  
  

  if (currTime - prevTime >= timeRefreshRate) {
    prevTime = currTime;

    int analogValue = analogRead(analogPin); // Read analog value from pin
    float voltage = analogValue * (3.3 / 4095.0); // Convert raw analog value to voltage (assuming 12-bit ADC and 3.3V reference)


    display.clearDisplay(); // Clear the display buffer before updating

    // Display Text Setup
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.println("SENSORS DASHBOARD");
    

    display.setTextSize(2);
    display.setCursor(0, 30);
    display.print("RAW: ");
    display.print(analogValue); // Display raw analog value

    display.setTextSize(1);
    display.setCursor(0, 50);
    display.print("Volt: ");
    display.print(voltage, 2); // Display voltage with 2 decimal places
    display.print(" V");
    display.display();
  }
}
