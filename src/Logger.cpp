/**
 * @file Logger.cpp
 * @brief Event logging system implementation for ESP32 Smart Alarm
 * @author Nighty Byte Team
 * @date 2025-08-25
 */

#include "Logger.h"
#include <time.h>

Logger::Logger() {
    flashLoggingEnabled = LOG_TO_FLASH;
    serialLoggingEnabled = LOG_TO_SERIAL;
    logBuffer.reserve(MAX_LOG_ENTRIES);
}

Logger::~Logger() {
    preferences.end();
}

bool Logger::begin() {
    if (flashLoggingEnabled) {
        if (!preferences.begin("logger", false)) {
            Serial.println("Failed to initialize NVS for logging");
            return false;
        }
    }
    
    logInfo(EVENT_SYSTEM_START, "Logger initialized", 
           String("Flash: ") + (flashLoggingEnabled ? "ON" : "OFF") + 
           ", Serial: " + (serialLoggingEnabled ? "ON" : "OFF"));
    
    return true;
}

void Logger::log(LogLevel level, LogEventType eventType, const String& message, const String& data) {
    LogEntry entry;
    entry.timestamp = millis();
    entry.level = level;
    entry.eventType = eventType;
    entry.message = message;
    entry.data = data;
    
    // Add to buffer
    if (logBuffer.size() >= MAX_LOG_ENTRIES) {
        logBuffer.erase(logBuffer.begin()); // Remove oldest entry
    }
    logBuffer.push_back(entry);
    
    // Write to serial if enabled
    if (serialLoggingEnabled) {
        printToSerial(entry);
    }
    
    // Write to flash if enabled
    if (flashLoggingEnabled) {
        writeToFlash(entry);
    }
}

void Logger::logDebug(LogEventType eventType, const String& message, const String& data) {
    log(LOG_DEBUG, eventType, message, data);
}

void Logger::logInfo(LogEventType eventType, const String& message, const String& data) {
    log(LOG_INFO, eventType, message, data);
}

void Logger::logWarning(LogEventType eventType, const String& message, const String& data) {
    log(LOG_WARNING, eventType, message, data);
}

void Logger::logError(LogEventType eventType, const String& message, const String& data) {
    log(LOG_ERROR, eventType, message, data);
}

String Logger::levelToString(LogLevel level) {
    switch (level) {
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO: return "INFO";
        case LOG_WARNING: return "WARN";
        case LOG_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

String Logger::eventTypeToString(LogEventType eventType) {
    switch (eventType) {
        case EVENT_SYSTEM_START: return "SYSTEM_START";
        case EVENT_ALARM_SET: return "ALARM_SET";
        case EVENT_ALARM_TRIGGERED: return "ALARM_TRIGGERED";
        case EVENT_ALARM_STOPPED: return "ALARM_STOPPED";
        case EVENT_ALARM_SNOOZED: return "ALARM_SNOOZED";
        case EVENT_PILL_BOX_OPENED: return "PILL_BOX_OPENED";
        case EVENT_PILL_BOX_CLOSED: return "PILL_BOX_CLOSED";
        case EVENT_BEDTIME_REMINDER: return "BEDTIME_REMINDER";
        case EVENT_USB_CONNECTED: return "USB_CONNECTED";
        case EVENT_USB_DISCONNECTED: return "USB_DISCONNECTED";
        case EVENT_WIFI_CONNECTED: return "WIFI_CONNECTED";
        case EVENT_WIFI_DISCONNECTED: return "WIFI_DISCONNECTED";
        case EVENT_OTA_START: return "OTA_START";
        case EVENT_OTA_SUCCESS: return "OTA_SUCCESS";
        case EVENT_OTA_FAILED: return "OTA_FAILED";
        case EVENT_LOW_BATTERY: return "LOW_BATTERY";
        case EVENT_SENSOR_ERROR: return "SENSOR_ERROR";
        default: return "UNKNOWN";
    }
}

void Logger::printToSerial(const LogEntry& entry) {
    // Format: [TIMESTAMP] LEVEL EVENT: MESSAGE (DATA)
    Serial.print("[");
    Serial.print(entry.timestamp);
    Serial.print("] ");
    Serial.print(levelToString(entry.level));
    Serial.print(" ");
    Serial.print(eventTypeToString(entry.eventType));
    Serial.print(": ");
    Serial.print(entry.message);
    if (!entry.data.isEmpty()) {
        Serial.print(" (");
        Serial.print(entry.data);
        Serial.print(")");
    }
    Serial.println();
}

void Logger::writeToFlash(const LogEntry& entry) {
    // TODO: Implement flash storage for logs
    // For now, we'll store a limited number of recent logs in preferences
    // In a production system, consider using SPIFFS or LittleFS for better log management
    
    static int logCounter = 0;
    String key = "log_" + String(logCounter % 20); // Keep only last 20 logs in flash
    
    String logString = String(entry.timestamp) + "," + 
                      String(entry.level) + "," + 
                      String(entry.eventType) + "," + 
                      entry.message + "," + 
                      entry.data;
    
    preferences.putString(key.c_str(), logString);
    logCounter++;
}

std::vector<LogEntry> Logger::getRecentLogs(int count) {
    int startIndex = max(0, (int)logBuffer.size() - count);
    return std::vector<LogEntry>(logBuffer.begin() + startIndex, logBuffer.end());
}

void Logger::clearLogs() {
    logBuffer.clear();
    if (flashLoggingEnabled) {
        preferences.clear();
    }
    logInfo(EVENT_SYSTEM_START, "Log buffer cleared");
}

void Logger::enableFlashLogging(bool enable) {
    flashLoggingEnabled = enable;
    logInfo(EVENT_SYSTEM_START, "Flash logging", enable ? "enabled" : "disabled");
}

void Logger::enableSerialLogging(bool enable) {
    serialLoggingEnabled = enable;
    logInfo(EVENT_SYSTEM_START, "Serial logging", enable ? "enabled" : "disabled");
}

String Logger::getLogsSummary() {
    String summary = "Logs Summary:\n";
    summary += "Total entries: " + String(logBuffer.size()) + "\n";
    
    int errorCount = 0, warningCount = 0, infoCount = 0, debugCount = 0;
    
    for (const auto& entry : logBuffer) {
        switch (entry.level) {
            case LOG_ERROR: errorCount++; break;
            case LOG_WARNING: warningCount++; break;
            case LOG_INFO: infoCount++; break;
            case LOG_DEBUG: debugCount++; break;
        }
    }
    
    summary += "Errors: " + String(errorCount) + "\n";
    summary += "Warnings: " + String(warningCount) + "\n";
    summary += "Info: " + String(infoCount) + "\n";
    summary += "Debug: " + String(debugCount) + "\n";
    
    return summary;
}

void Logger::exportLogsToString(String& output) {
    output = "";
    for (const auto& entry : logBuffer) {
        output += "[" + String(entry.timestamp) + "] ";
        output += levelToString(entry.level) + " ";
        output += eventTypeToString(entry.eventType) + ": ";
        output += entry.message;
        if (!entry.data.isEmpty()) {
            output += " (" + entry.data + ")";
        }
        output += "\n";
    }
}
