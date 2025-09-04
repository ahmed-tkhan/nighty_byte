// // test_rtc.cpp
// // Simple test for ESP32Time RTC library with Logger
// // Prints current time every second

// #include <Arduino.h>
// #include <ESP32Time.h>
// #include "Logger.h"

// Logger logger;
// ESP32Time rtc;

// void setup() {
//     Serial.begin(115200);
//     delay(500);
//     Serial.println("\n=== RTC/Timekeeping Test (Class-based) ===");
//     logger.begin();
//     Serial.println("Setting time to 2025-09-04 12:00:00");
//     rtc.setTime(0, 0, 12, 4, 9, 2025); // sec, min, hour, day, month, year
//     logger.logInfo(EVENT_SYSTEM_START, "RTC set", rtc.getTime("%Y-%m-%d %H:%M:%S"));
// }

// void loop() {
//     String now = rtc.getTime("%Y-%m-%d %H:%M:%S");
//     Serial.print("Current time: ");
//     Serial.println(now);
//     delay(1000);
// }

// // Original direct test (kept for reference):
// // #include <Arduino.h>
// // #include <ESP32Time.h>

// // ESP32Time rtc;

// // void setup() {
// //     Serial.begin(115200);
// //     delay(500);
// //     Serial.println("\n=== RTC/Timekeeping Test ===");
// //     Serial.println("Setting time to 2025-09-04 12:00:00");
// //     rtc.setTime(0, 0, 12, 4, 9, 2025); // sec, min, hour, day, month, year
// // }

// // void loop() {
// //     Serial.print("Current time: ");
// //     Serial.println(rtc.getTime("%Y-%m-%d %H:%M:%S"));
// //     delay(1000);
// // }
