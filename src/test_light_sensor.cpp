// // // test_light_sensor.cpp
// // // Simple test for LDR light sensor using SensorManager class
// // // Connect LDR+resistor voltage divider to GPIO36

// #include <Arduino.h>
// #include "Logger.h"
// #include "SensorManager.h"

// Logger logger;
// SensorManager sensor(&logger);

// void setup() {
//     Serial.begin(115200);
//     delay(500);
//     Serial.println("\n=== Light Sensor (LDR) Test (Class-based) ===");
//     logger.begin();
//     sensor.begin();
// }

// void loop() {
//     sensor.update();
//     int value = sensor.getLightLevel();
//     Serial.print("ADC value: ");
//     Serial.print(value);
//     if (sensor.isDarkEnvironment()) {
//         Serial.print("  (DARK)");
//     } else if (value > 3000) {
//         Serial.print("  (BRIGHT)");
//     }
//     Serial.println("");
//     delay(500);
// }

// // Original direct test (kept for reference):
// // #define LIGHT_SENSOR_PIN 36

// // void setup() {
// //     Serial.begin(115200);
// //     delay(500);
// //     Serial.println("\n=== Light Sensor (LDR) Test ===");
// //     Serial.println("LDR voltage divider to GPIO36 (ADC1_CH0)");
// // }

// // void loop() {
// //     int value = analogRead(LIGHT_SENSOR_PIN);
// //     Serial.print("ADC value: ");
// //     Serial.print(value);
// //     if (value < 500) {
// //         Serial.print("  (DARK)");
// //     } else if (value > 3000) {
// //         Serial.print("  (BRIGHT)");
// //     }
// //     Serial.println("");
// //     delay(500);
// // }
