// /**
//  * @file main.cpp
//  * @brief Main application for ESP32 Night Routine Smart Alarm
//  * @author Nighty Byte Team
//  * @date 2025-08-25
//  * 
//  * Hardware Setup:
//  * - ESP32 DevKit V1
//  * - 3.3V Buzzer connected to GPIO2
//  * - Pill box contact switch connected to GPIO4 (with internal pullup)
//  * - Light sensor (LDR) connected to GPIO36 (ADC1_CH0)
//  * - USB charging detection connected to GPIO39 (ADC1_CH3)
//  */

// #include <Arduino.h>
// #include <ESP32Time.h>
// #include "config.h"
// #include "Logger.h"
// #include "AlarmManager.h"
// #include "SensorManager.h"
// #include "BuzzerController.h"
// #include "NetworkManager.h"

// // Global instances
// Logger* logger;
// ESP32Time* rtc;
// AlarmManager* alarmManager;
// SensorManager* sensorManager;
// BuzzerController* buzzerController;
// NetworkManager* networkManager;

// // System state
// bool systemInitialized = false;
// unsigned long lastBedtimeCheck = 0;
// unsigned long lastSystemUpdate = 0;

// // Function prototypes
// void initializeSystem();
// void updateSystem();
// void checkBedtimeReminder();
// void onBuzzerControl(bool enabled);
// bool onPillBoxCheck();
// void onBedtimeDetected(bool isDark);
// void onUsbStateChanged(bool connected);
// void onNetworkStateChanged(bool connected);
// void onNetworkCommand(const String& command, const String& data);
// void printSystemStatus();
// void handleSerialCommands();

// void setup() {
//     // Initialize serial communication
//     Serial.begin(SERIAL_BAUD_RATE);
//     delay(1000); // Give serial time to initialize
    
//     Serial.println("\n" + String('=', 50));
//     Serial.println("ESP32 Night Routine Smart Alarm v" + String(FIRMWARE_VERSION));
//     Serial.println("Hardware: " + String(HARDWARE_VERSION));
//     Serial.println("Build Date: " + String(__DATE__) + " " + String(__TIME__));
//     Serial.println(String('=', 50) + "\n");
    
//     // Initialize system components
//     initializeSystem();
    
//     Serial.println("Setup complete. System running...\n");
    
//     // Print initial status
//     printSystemStatus();
// }

// void loop() {
//     if (!systemInitialized) {
//         delay(1000);
//         return;
//     }
    
//     // Update all system components
//     updateSystem();
    
//     // Check bedtime reminder
//     checkBedtimeReminder();
    
//     // Handle serial commands for debugging/testing
//     handleSerialCommands();
    
//     // Small delay to prevent watchdog issues
//     delay(10);
// }

// void initializeSystem() {
//     Serial.println("Initializing system components...");
    
//     // Initialize RTC
//     rtc = new ESP32Time(0); // UTC initially, will be updated from NTP
//     if (rtc) {
//         Serial.println("✓ RTC initialized");
//     } else {
//         Serial.println("✗ RTC initialization failed");
//         return;
//     }
    
//     // Initialize Logger
//     logger = new Logger();
//     if (logger && logger->begin()) {
//         Serial.println("✓ Logger initialized");
//         logger->logInfo(EVENT_SYSTEM_START, "System startup", 
//                        "Firmware: " + String(FIRMWARE_VERSION));
//     } else {
//         Serial.println("✗ Logger initialization failed");
//         return;
//     }
    
//     // Initialize Buzzer Controller
//     buzzerController = new BuzzerController(logger);
//     if (buzzerController && buzzerController->begin()) {
//         Serial.println("✓ Buzzer controller initialized");
//     } else {
//         Serial.println("✗ Buzzer controller initialization failed");
//         if (logger) logger->logError(EVENT_SYSTEM_START, "Buzzer controller init failed");
//         return;
//     }
    
//     // Initialize Sensor Manager
//     sensorManager = new SensorManager(logger);
//     if (sensorManager && sensorManager->begin()) {
//         Serial.println("✓ Sensor manager initialized");
        
//         // Set up sensor callbacks
//         sensorManager->setBedtimeCallback(onBedtimeDetected);
//         sensorManager->setUsbStateCallback(onUsbStateChanged);
//     } else {
//         Serial.println("✗ Sensor manager initialization failed");
//         if (logger) logger->logError(EVENT_SYSTEM_START, "Sensor manager init failed");
//         return;
//     }
    
//     // Initialize Alarm Manager
//     alarmManager = new AlarmManager(logger, rtc);
//     if (alarmManager && alarmManager->begin()) {
//         Serial.println("✓ Alarm manager initialized");
        
//         // Set up alarm callbacks
//         alarmManager->setBuzzerCallback(onBuzzerControl);
//         alarmManager->setPillBoxCallback(onPillBoxCheck);
//     } else {
//         Serial.println("✗ Alarm manager initialization failed");
//         if (logger) logger->logError(EVENT_SYSTEM_START, "Alarm manager init failed");
//         return;
//     }
    
//     // Initialize Network Manager
//     networkManager = new NetworkManager(logger, rtc);
//     if (networkManager && networkManager->begin()) {
//         Serial.println("✓ Network manager initialized");
        
//         // Set up network callbacks
//         networkManager->setConnectionCallback(onNetworkStateChanged);
//         networkManager->setCommandCallback(onNetworkCommand);
//     } else {
//         Serial.println("✗ Network manager initialization failed");
//         if (logger) logger->logError(EVENT_SYSTEM_START, "Network manager init failed");
//         return;
//     }
    
//     systemInitialized = true;
//     Serial.println("\n🎉 All systems initialized successfully!");
    
//     if (logger) {
//         logger->logInfo(EVENT_SYSTEM_START, "System fully initialized", 
//                        "Free heap: " + String(ESP.getFreeHeap()) + " bytes");
//     }
    
//     // Play startup sound
//     if (buzzerController) {
//         buzzerController->playStartupTone();
//     }
// }

// void updateSystem() {
//     unsigned long currentTime = millis();
    
//     // Update components at different intervals to balance performance
//     if (currentTime - lastSystemUpdate >= 50) { // 20Hz update rate
//         // High priority updates
//         if (alarmManager) alarmManager->update();
//         if (sensorManager) sensorManager->update();
//         if (buzzerController) buzzerController->update();
        
//         lastSystemUpdate = currentTime;
//     }
    
//     // Lower priority updates
//     if (networkManager) networkManager->update();
    
//     // Watchdog reset (ESP32 has hardware watchdog that we need to feed)
//     yield();
// }

// void checkBedtimeReminder() {
//     unsigned long currentTime = millis();
    
//     // Check bedtime reminder every minute
//     if (currentTime - lastBedtimeCheck >= BEDTIME_CHECK_INTERVAL_MS) {
//         if (rtc && sensorManager) {
//             struct tm timeinfo;
//             if (getLocalTime(&timeinfo)) {
//                 // Check if it's bedtime hour and environment is dark
//                 if (timeinfo.tm_hour == BEDTIME_REMINDER_HOUR && 
//                     timeinfo.tm_min == BEDTIME_REMINDER_MINUTE &&
//                     sensorManager->isDarkEnvironment()) {
                    
//                     // Trigger bedtime reminder
//                     if (buzzerController) {
//                         buzzerController->playPattern(PATTERN_NOTIFICATION);
//                     }
                    
//                     if (logger) {
//                         logger->logInfo(EVENT_BEDTIME_REMINDER, "Bedtime reminder triggered",
//                                        "Light level: " + String(sensorManager->getLightLevel()));
//                     }
//                 }
//             }
//         }
        
//         lastBedtimeCheck = currentTime;
//     }
// }

// // Callback functions
// void onBuzzerControl(bool enabled) {
//     if (buzzerController) {
//         if (enabled) {
//             buzzerController->playPattern(PATTERN_ALARM);
//         } else {
//             buzzerController->stopPattern();
//         }
//     }
// }

// bool onPillBoxCheck() {
//     return sensorManager ? sensorManager->isPillBoxOpen() : false;
// }

// void onBedtimeDetected(bool isDark) {
//     if (isDark && logger) {
//         logger->logInfo(EVENT_BEDTIME_REMINDER, "Dark environment detected");
//     }
// }

// void onUsbStateChanged(bool connected) {
//     if (logger) {
//         logger->logInfo(connected ? EVENT_USB_CONNECTED : EVENT_USB_DISCONNECTED,
//                        connected ? "Phone charging started" : "Phone charging stopped");
//     }
    
//     // Optional: Play notification sound
//     if (buzzerController && connected) {
//         buzzerController->playBeep(1000, 100);
//     }
// }

// void onNetworkStateChanged(bool connected) {
//     if (connected) {
//         // Network connected - sync time
//         if (networkManager) {
//             networkManager->syncTime();
//         }
        
//         // Play success sound
//         if (buzzerController) {
//             buzzerController->playPattern(PATTERN_SUCCESS);
//         }
//     } else {
//         // Network disconnected
//         if (buzzerController) {
//             buzzerController->playBeep(500, 200);
//         }
//     }
// }

// void onNetworkCommand(const String& command, const String& data) {
//     if (command == "SETALARM") {
//         // Parse alarm data: "SETALARM:hour:minute:days:label"
//         int firstColon = data.indexOf(':', 9); // Skip "SETALARM:"
//         int secondColon = data.indexOf(':', firstColon + 1);
//         int thirdColon = data.indexOf(':', secondColon + 1);
//         int fourthColon = data.indexOf(':', thirdColon + 1);
        
//         if (firstColon > 0 && secondColon > 0 && thirdColon > 0) {
//             int hour = data.substring(firstColon + 1, secondColon).toInt();
//             int minute = data.substring(secondColon + 1, thirdColon).toInt();
//             String days = data.substring(thirdColon + 1, fourthColon > 0 ? fourthColon : data.length());
//             String label = fourthColon > 0 ? data.substring(fourthColon + 1) : "";
            
//             if (alarmManager) {
//                 uint8_t dayMask = AlarmManager::stringToDayMask(days);
//                 if (alarmManager->addAlarm(hour, minute, dayMask, label)) {
//                     if (logger) {
//                         logger->logInfo(EVENT_ALARM_SET, "Alarm added via network",
//                                        "Time: " + String(hour) + ":" + String(minute) +
//                                        ", Days: " + days + ", Label: " + label);
//                     }
//                 }
//             }
//         }
//     }
//     // Add more network commands as needed
// }

// void printSystemStatus() {
//     Serial.println("\n" + String('-', 40));
//     Serial.println("SYSTEM STATUS");
//     Serial.println(String('-', 40));
    
//     // Memory info
//     Serial.println("Free Heap: " + String(ESP.getFreeHeap()) + " bytes");
//     Serial.println("Uptime: " + String(millis() / 1000) + " seconds");
    
//     // Component status
//     if (networkManager) {
//         Serial.println("Network: " + networkManager->getNetworkInfo());
//     }
    
//     if (sensorManager) {
//         Serial.println(sensorManager->getSensorStatus());
//     }
    
//     if (alarmManager) {
//         Serial.println(alarmManager->getAlarmsStatus());
//     }
    
//     // Time info
//     if (rtc) {
//         Serial.println("Current Time: " + rtc->getTime("%Y-%m-%d %H:%M:%S"));
//     }
    
//     Serial.println(String('-', 40) + "\n");
// }

// void handleSerialCommands() {
//     if (Serial.available() > 0) {
//         String command = Serial.readStringUntil('\n');
//         command.trim();
//         command.toUpperCase();
        
//         if (command == "STATUS") {
//             printSystemStatus();
//         }
//         else if (command == "TEST_BUZZER") {
//             if (buzzerController) {
//                 Serial.println("Testing buzzer...");
//                 buzzerController->performBuzzerTest();
//             }
//         }
//         else if (command == "TEST_SENSORS") {
//             if (sensorManager) {
//                 Serial.println("Testing sensors...");
//                 sensorManager->performSensorTest();
//             }
//         }
//         else if (command == "TEST_NETWORK") {
//             if (networkManager) {
//                 Serial.println("Testing network...");
//                 networkManager->performNetworkTest();
//             }
//         }
//         else if (command == "ADD_ALARM") {
//             if (alarmManager) {
//                 // Add a test alarm for 1 minute from now
//                 struct tm timeinfo;
//                 if (getLocalTime(&timeinfo)) {
//                     int hour = timeinfo.tm_hour;
//                     int minute = timeinfo.tm_min + 1;
//                     if (minute >= 60) {
//                         minute = 0;
//                         hour++;
//                         if (hour >= 24) hour = 0;
//                     }
                    
//                     if (alarmManager->addAlarm(hour, minute, 0, "Test Alarm")) {
//                         Serial.println("Test alarm added for " + String(hour) + ":" + String(minute));
//                     }
//                 }
//             }
//         }
//         else if (command == "CLEAR_ALARMS") {
//             if (alarmManager) {
//                 alarmManager->clearAllAlarms();
//                 Serial.println("All alarms cleared");
//             }
//         }
//         else if (command == "BEEP") {
//             if (buzzerController) {
//                 buzzerController->playTripleBeep();
//             }
//         }
//         else if (command == "LOGS") {
//             if (logger) {
//                 Serial.println(logger->getLogsSummary());
//             }
//         }
//         else if (command == "HELP") {
//             Serial.println("Available commands:");
//             Serial.println("  STATUS - Show system status");
//             Serial.println("  TEST_BUZZER - Test buzzer functionality");
//             Serial.println("  TEST_SENSORS - Test all sensors");
//             Serial.println("  TEST_NETWORK - Test network connectivity");
//             Serial.println("  ADD_ALARM - Add test alarm");
//             Serial.println("  CLEAR_ALARMS - Clear all alarms");
//             Serial.println("  BEEP - Play test beep");
//             Serial.println("  LOGS - Show log summary");
//             Serial.println("  HELP - Show this help");
//         }
//         else if (!command.isEmpty()) {
//             Serial.println("Unknown command: " + command + " (type HELP for available commands)");
//         }
//     }
// }
