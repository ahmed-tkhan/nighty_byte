// test_pillbox.cpp
// Simple test for pill box contact switch on ESP32 DevKit V1 (GPIO4)
// Connect one side of switch to GPIO4, other to GND
// Internal pullup is enabled

#include <Arduino.h>

#define PILLBOX_PIN 4

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== Pill Box Switch Test ===");
    Serial.println("Switch pin: GPIO4 (internal pullup enabled)");
    pinMode(PILLBOX_PIN, INPUT_PULLUP);
}

void loop() {
    bool state = digitalRead(PILLBOX_PIN);
    Serial.print("Switch state: ");
    if (state == LOW) {
        Serial.println("CLOSED (pressed)");
    } else {
        Serial.println("OPEN (released)");
    }
    delay(200);
}
