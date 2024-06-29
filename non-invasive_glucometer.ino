#include <Wire.h>
#include <EEPROM.h>
#include "MAX30100_PulseOximeter.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define ENABLE_MAX30100 1
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#if ENABLE_MAX30100
#define REPORTING_PERIOD_MS 5000

PulseOximeter pox;
#endif

uint32_t tsLastReport = 0;
uint32_t tsLastSave = 0;

float glucose_records[3] = {0.0, 0.0, 0.0}; // Array to store the last three records

// Callback (registered below) fired when a pulse is detected
void onBeatDetected()
{
    Serial.println("Beat!");
}

void setup()
{
    Serial.begin(115200);
    Serial.println("SSD1306 128x32 OLED TEST");

    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;); // Don't proceed, loop forever
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(20, 18);
    display.print("Pulse OxiMeter");
    int temp1 = 0;
    int temp2 = 40;
    int temp3 = 80;
    heart_beat(&temp1);
    heart_beat(&temp2);
    heart_beat(&temp3);
    display.display();
    delay(2000); // Pause for 2 seconds
    display.cp437(true);
    display.clearDisplay();

    Serial.print("Initializing pulse oximeter..");
#if ENABLE_MAX30100
    if (!pox.begin()) {
        Serial.println("FAILED");
        for (;;);
    } else {
        Serial.println("SUCCESS");
    }

    pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
    pox.setOnBeatDetectedCallback(onBeatDetected);
#endif

    load_glucose_records();
    print_glucose_records(); // Print loaded records to Serial
}

void loop()
{
#if ENABLE_MAX30100
    pox.update();
    int bpm = 0;
    int spo2 = 0;
    float glucose_level = 0.0;

    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        bpm = pox.getHeartRate();
        spo2 = pox.getSpO2();

        if (bpm > 0 && spo2 > 0) {
            glucose_level = 16714.61 + 0.47 * bpm - 351.045 * spo2 + 1.85 * (spo2 * spo2);
        }

        Serial.print("Heart rate: ");
        Serial.println(bpm);
        Serial.print("SpO2: ");
        Serial.println(spo2);
        Serial.print("Glucose Level: ");
        Serial.println(glucose_level);

        tsLastReport = millis();
        display_data(bpm, spo2, glucose_level);
    }

    // Save the glucose level if it is smaller than 500 every 10 seconds
    if (glucose_level < 500.0 && glucose_level > 0 && millis() - tsLastSave > 10000) {
        save_glucose_level(glucose_level);
        tsLastSave = millis();
    }
#endif
}

void display_data(int bpm, int spo2, float glucose_level)
{
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print("BPM:");
    display.println(bpm);

    display.setCursor(0, 10);
    display.print("SpO2:");
    display.println(spo2);

    display.setCursor(0, 20);
    if (glucose_level > 500.0 || glucose_level < 0) {
        display.print("Glucose:MEAS");
    } else {
        display.print("Glucose:");
        display.println(glucose_level);
    }

    // Display last three glucose records on the right side
    display.drawLine(88, 0, 88, 32, WHITE);
    for (int i = 0; i < 3; i++) {
        display.setCursor(92, 0 + (i * 10));
        display.print(glucose_records[i]);
    }

    display.display();
}

void save_glucose_level(float glucose_level)
{
    // Shift the old records
    glucose_records[2] = glucose_records[1];
    glucose_records[1] = glucose_records[0];
    glucose_records[0] = glucose_level;

    // Save the records to EEPROM
    for (int i = 0; i < 3; i++) {
        EEPROM.put(i * sizeof(float), glucose_records[i]);
    }

    print_glucose_records(); // Print saved records to Serial for debugging
}

void load_glucose_records()
{
    // Load the records from EEPROM
    for (int i = 0; i < 3; i++) {
        EEPROM.get(i * sizeof(float), glucose_records[i]);
    }
}

void print_glucose_records()
{
    // Print glucose records to Serial for debugging
    Serial.println("Glucose Records:");
    for (int i = 0; i < 3; i++) {
        Serial.print(i);
        Serial.print(": ");
        Serial.println(glucose_records[i]);
    }
}

void heart_beat(int *x_pos) {
  /************************************************/
  //display.clearDisplay();
  display.fillRect(*x_pos, 0, 30, 15, SSD1306_BLACK);
  // Draw a single pixel in white
  display.drawPixel(*x_pos + 0, 8, SSD1306_WHITE);
  display.drawPixel(*x_pos + 1, 8, SSD1306_WHITE);
  display.drawPixel(*x_pos + 2, 8, SSD1306_WHITE);
  display.drawPixel(*x_pos + 3, 8, SSD1306_WHITE);
  display.drawPixel(*x_pos + 4, 8, BLACK); // -----
  //display.display();
  //delay(1);
  display.drawPixel(*x_pos + 5, 7, SSD1306_WHITE);
  display.drawPixel(*x_pos + 6, 6, SSD1306_WHITE);
  display.drawPixel(*x_pos + 7, 7, SSD1306_WHITE); // .~.
  //display.display();
  //delay(1);
  display.drawPixel(*x_pos + 8, 8, SSD1306_WHITE);
  display.drawPixel(*x_pos + 9, 8, SSD1306_WHITE); // --
  //display.display();
  //delay(1);
  /******************************************/
  display.drawPixel(*x_pos + 10, 8, SSD1306_WHITE);
  display.drawPixel(*x_pos + 10, 9, SSD1306_WHITE);
  display.drawPixel(*x_pos + 11, 10, SSD1306_WHITE);
  display.drawPixel(*x_pos + 11, 11, SSD1306_WHITE);
  //display.display();
  //delay(1);
  /******************************************/
  display.drawPixel(*x_pos + 12, 10, SSD1306_WHITE);
  display.drawPixel(*x_pos + 12, 9, SSD1306_WHITE);
  display.drawPixel(*x_pos + 12, 8, SSD1306_WHITE);
  display.drawPixel(*x_pos + 12, 7, SSD1306_WHITE);
  //display.display();
  //delay(1);
  display.drawPixel(*x_pos + 13, 6, SSD1306_WHITE);
  display.drawPixel(*x_pos + 13, 5, SSD1306_WHITE);
  display.drawPixel(*x_pos + 13, 4, SSD1306_WHITE);
  display.drawPixel(*x_pos + 13, 3, SSD1306_WHITE);
  //display.display();
  //delay(1);
  display.drawPixel(*x_pos + 14, 2, SSD1306_WHITE);
  display.drawPixel(*x_pos + 14, 1, SSD1306_WHITE);
  display.drawPixel(*x_pos + 14, 0, SSD1306_WHITE);
  display.drawPixel(*x_pos + 14, 0, SSD1306_WHITE);
  //display.display();
  //delay(1);
  /******************************************/
  display.drawPixel(*x_pos + 15, 0, SSD1306_WHITE);
  display.drawPixel(*x_pos + 15, 1, SSD1306_WHITE);
  display.drawPixel(*x_pos + 15, 2, SSD1306_WHITE);
  display.drawPixel(*x_pos + 15, 3, SSD1306_WHITE);
  //display.display();
  //delay(1);
  display.drawPixel(*x_pos + 15, 4, SSD1306_WHITE);
  display.drawPixel(*x_pos + 15, 5, SSD1306_WHITE);
  display.drawPixel(*x_pos + 16, 6, SSD1306_WHITE);
  display.drawPixel(*x_pos + 16, 7, SSD1306_WHITE);
  //display.display();
  //delay(1);
  display.drawPixel(*x_pos + 16, 8, SSD1306_WHITE);
  display.drawPixel(*x_pos + 16, 9, SSD1306_WHITE);
  display.drawPixel(*x_pos + 16, 10, SSD1306_WHITE);
  display.drawPixel(*x_pos + 16, 11, SSD1306_WHITE);
  //display.display();
  //delay(1);
  display.drawPixel(*x_pos + 17, 12, SSD1306_WHITE);
  display.drawPixel(*x_pos + 17, 13, SSD1306_WHITE);
  display.drawPixel(*x_pos + 17, 14, SSD1306_WHITE);
  display.drawPixel(*x_pos + 17, 15, SSD1306_WHITE);
  //display.display();
  //delay(1);
  display.drawPixel(*x_pos + 18, 15, SSD1306_WHITE);
  display.drawPixel(*x_pos + 18, 14, SSD1306_WHITE);
  display.drawPixel(*x_pos + 18, 13, SSD1306_WHITE);
  display.drawPixel(*x_pos + 18, 12, SSD1306_WHITE);
  //display.display();
  //delay(1);
  display.drawPixel(*x_pos + 19, 11, SSD1306_WHITE);
  display.drawPixel(*x_pos + 19, 10, SSD1306_WHITE);
  display.drawPixel(*x_pos + 19, 9, SSD1306_WHITE);
  display.drawPixel(*x_pos + 19, 8, SSD1306_WHITE);
  //display.display();
  //delay(1);
  /****************************************************/
  display.drawPixel(*x_pos + 20, 8, SSD1306_WHITE);
  display.drawPixel(*x_pos + 21, 8, SSD1306_WHITE);
  //display.display();
  //delay(1);
  /****************************************************/
  display.drawPixel(*x_pos + 22, 7, SSD1306_WHITE);
  display.drawPixel(*x_pos + 23, 6, SSD1306_WHITE);
  display.drawPixel(*x_pos + 24, 6, SSD1306_WHITE);
  display.drawPixel(*x_pos + 25, 7, SSD1306_WHITE);
  //display.display();
  //delay(1);
  /************************************************/
  display.drawPixel(*x_pos + 26, 8, SSD1306_WHITE);
  display.drawPixel(*x_pos + 27, 8, SSD1306_WHITE);
  display.drawPixel(*x_pos + 28, 8, SSD1306_WHITE);
  display.drawPixel(*x_pos + 29, 8, SSD1306_WHITE);
  display.drawPixel(*x_pos + 30, 8, SSD1306_WHITE); // -----
  *x_pos = *x_pos + 30;
  display.display();
  delay(1);
}