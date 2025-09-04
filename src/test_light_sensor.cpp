// test_light_sensor.cpp
// Simple test for LDR light sensor on ESP32 DevKit V1 (GPIO36/ADC1_CH0)
// Connect LDR+resistor voltage divider to GPIO36

#include <Arduino.h>

#define LIGHT_SENSOR_PIN 36

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== Light Sensor (LDR) Test ===");
    Serial.println("LDR voltage divider to GPIO36 (ADC1_CH0)");
}

void loop() {
    int value = analogRead(LIGHT_SENSOR_PIN);
    Serial.print("ADC value: ");
    Serial.print(value);
    if (value < 500) {
        Serial.print("  (DARK)");
    } else if (value > 3000) {
        Serial.print("  (BRIGHT)");
    }
    Serial.println("");
    delay(500);
}
