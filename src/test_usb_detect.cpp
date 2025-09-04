// test_usb_detect.cpp
// Simple test for USB charging detection on ESP32 DevKit V1 (GPIO39/ADC1_CH3)
// Use voltage divider from USB 5V to GPIO39

#include <Arduino.h>

#define USB_DETECT_PIN 39
#define USB_THRESHOLD 2048 // Adjust based on your voltage divider

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== USB Charging Detection Test ===");
    Serial.println("Voltage divider from USB 5V to GPIO39 (ADC1_CH3)");
}

void loop() {
    int value = analogRead(USB_DETECT_PIN);
    Serial.print("ADC value: ");
    Serial.print(value);
    if (value > USB_THRESHOLD) {
        Serial.println("  (USB CONNECTED)");
    } else {
        Serial.println("  (USB NOT CONNECTED)");
    }
    delay(500);
}
