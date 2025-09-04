// test_buzzer.cpp
// Simple test for 3.3V buzzer on ESP32 DevKit V1 (GPIO2)
// Connect buzzer (+) to GPIO2, (-) to GND

#include <Arduino.h>
#include "Logger.h"
#include "BuzzerController.h"

Logger logger;
BuzzerController buzzer(&logger);

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== Buzzer Test (Class-based) ===");
    logger.begin();
    buzzer.begin();
}

void loop() {
    Serial.println("Buzzer ON (continuous tone)");
    buzzer.setBuzzer(true);
    delay(1000);
    Serial.println("Buzzer OFF");
    buzzer.setBuzzer(false);
    delay(1000);

    // Test beep pattern
    Serial.println("Beep pattern: 3 short beeps");
    buzzer.playTripleBeep();
    delay(2000);

    // Test different patterns
    Serial.println("Pattern: ALARM");
    buzzer.playPattern(PATTERN_ALARM);
    delay(2500);
    buzzer.stopPattern();
    delay(500);

    Serial.println("Pattern: SUCCESS");
    buzzer.playPattern(PATTERN_SUCCESS);
    delay(1500);
    buzzer.stopPattern();
    delay(500);

    Serial.println("Pattern: ERROR");
    buzzer.playPattern(PATTERN_ERROR);
    delay(1500);
    buzzer.stopPattern();
    delay(500);

    Serial.println("Pattern: NOTIFICATION");
    buzzer.playPattern(PATTERN_NOTIFICATION);
    delay(1500);
    buzzer.stopPattern();
    delay(1000);

    // Test frequency sweep using class
    Serial.println("Sweep frequencies: 1kHz to 3kHz");
    for (int f = 1000; f <= 3000; f += 500) {
        buzzer.playTone(f, 0);
        Serial.print("Freq: "); Serial.println(f);
        delay(400);
        buzzer.stopTone();
        delay(100);
    }
    delay(3000);

    // Uncomment below to use direct PWM (original test)
    // ledcSetup(BUZZER_CHANNEL, BUZZER_FREQ, 8); // 8-bit PWM
    // ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
}
