// test_logging.cpp
// Simple test for Logger class (flash/NVS)

#include <Arduino.h>
#include "Logger.h"

Logger logger;

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== Logging Test ===");
    if (logger.begin()) {
        Serial.println("Logger initialized");
        logger.logInfo(EVENT_SYSTEM_START, "Test log entry", "Hello from logger!");
        logger.logWarning(EVENT_SYSTEM_START, "Warning log entry");
        logger.logError(EVENT_SYSTEM_START, "Error log entry");
    } else {
        Serial.println("Logger init failed");
    }
}

void loop() {
    Serial.println("Recent logs:");
    auto logs = logger.getRecentLogs(5);
    for (auto& entry : logs) {
        Serial.print("["); Serial.print(entry.timestamp); Serial.print("] ");
        Serial.print(entry.message); Serial.print(" ("); Serial.print(entry.level); Serial.println(")");
    }
    delay(3000);
}
