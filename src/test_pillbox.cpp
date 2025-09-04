// // // test_pillbox.cpp
// // // Simple test for pill box contact switch using SensorManager class
// // // Connect one side of switch to GPIO4, other to GND
// // // Internal pullup is enabled

// #include <Arduino.h>
// #include "Logger.h"
// #include "SensorManager.h"

// Logger logger;
// SensorManager sensor(&logger);

// void setup() {
//     Serial.begin(115200);
//     delay(500);
//     Serial.println("\n=== Pill Box Switch Test (Class-based) ===");
//     logger.begin();
//     sensor.begin();
// }

// void loop() {
//     sensor.update();
//     bool state = sensor.isPillBoxOpen();
//     Serial.print("Switch state: ");
//     if (state) {
//         Serial.println("OPEN (released)");
//     } else {
//         Serial.println("CLOSED (pressed)");
//     }
//     delay(200);
// }

// // // Original direct test (kept for reference):
// // // test_pillbox.cpp
// // // Simple test for pill box contact switch on ESP32 DevKit V1 (GPIO4)
// // // Connect one side of switch to GPIO4, other to GND
// // // Internal pullup is enabled

// // #include <Arduino.h>

// // #define PILLBOX_PIN 4

// // void setup() {
// //     Serial.begin(115200);
// //     delay(500);
// //     Serial.println("\n=== Pill Box Switch Test ===");
// //     Serial.println("Switch pin: GPIO4 (internal pullup enabled)");
// //     pinMode(PILLBOX_PIN, INPUT_PULLUP);
// // }

// // void loop() {
// //     bool state = digitalRead(PILLBOX_PIN);
// //     Serial.print("Switch state: ");
// //     if (state == LOW) {
// //         Serial.println("CLOSED (pressed)");
// //     } else {
// //         Serial.println("OPEN (released)");
// //     }
// //     delay(200);
// // }
