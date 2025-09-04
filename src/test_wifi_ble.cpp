// // test_wifi_ble.cpp
// // Simple test for WiFi scan/connect and BLE advertising (stub)

// #include <Arduino.h>
// #include <WiFi.h>
// // #include <BLEDevice.h> // Uncomment if using BLE

// void setup() {
//     Serial.begin(115200);
//     delay(500);
//     Serial.println("\n=== WiFi/BLE Connectivity Test ===");
//     Serial.println("Scanning WiFi networks...");
//     WiFi.mode(WIFI_STA);
//     int n = WiFi.scanNetworks();
//     for (int i = 0; i < n; i++) {
//         Serial.print(i + 1);
//         Serial.print(": ");
//         Serial.print(WiFi.SSID(i));
//         Serial.print(" (RSSI: ");
//         Serial.print(WiFi.RSSI(i));
//         Serial.println(")");
//     }
//     Serial.println("WiFi scan complete.");

//     // BLE stub
//     Serial.println("BLE advertising test (not implemented)");
//     // BLEDevice::init("SmartAlarmTest");
//     // BLEServer *pServer = BLEDevice::createServer();
//     // pServer->getAdvertising()->start();
// }

// void loop() {
//     // Nothing to do in loop
//     delay(2000);
// }
