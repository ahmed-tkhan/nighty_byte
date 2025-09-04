// test_buzzer.cpp
// Simple test for 3.3V buzzer on ESP32 DevKit V1 (GPIO2)
// Connect buzzer (+) to GPIO2, (-) to GND

#include <Arduino.h>

#define BUZZER_PIN 2
#define BUZZER_CHANNEL 0
#define BUZZER_FREQ 2000

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== Buzzer Test ===");
    Serial.println("Buzzer pin: GPIO2");
    ledcSetup(BUZZER_CHANNEL, BUZZER_FREQ, 8); // 8-bit PWM
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
}

void loop() {
    Serial.println("Buzzer ON (continuous tone)");
    ledcWrite(BUZZER_CHANNEL, 128); // 50% duty
    delay(1000);
    Serial.println("Buzzer OFF");
    ledcWrite(BUZZER_CHANNEL, 0);
    delay(1000);

    // Test beep pattern
    Serial.println("Beep pattern: 3 short beeps");
    for (int i = 0; i < 3; i++) {
        ledcWrite(BUZZER_CHANNEL, 128);
        delay(150);
        ledcWrite(BUZZER_CHANNEL, 0);
        delay(150);
    }
    delay(2000);

    // Test different frequencies
    Serial.println("Sweep frequencies: 1kHz to 3kHz");
    for (int f = 1000; f <= 3000; f += 500) {
        ledcSetup(BUZZER_CHANNEL, f, 8);
        ledcWrite(BUZZER_CHANNEL, 128);
        Serial.print("Freq: "); Serial.println(f);
        delay(400);
        ledcWrite(BUZZER_CHANNEL, 0);
        delay(100);
    }
    delay(3000);
}
