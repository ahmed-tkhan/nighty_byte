/**
 * @file Logger.h
 * @brief Event logging system for ESP32 Smart Alarm
 * @author Nighty Byte Team
 * @date 2025-08-25
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <vector>
#include <Preferences.h>
#include "config.h"

// Log levels
enum LogLevel {
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_WARNING = 2,
    LOG_ERROR = 3
};

// Log event types
enum LogEventType {
    EVENT_SYSTEM_START = 0,
    EVENT_ALARM_SET = 1,
    EVENT_ALARM_TRIGGERED = 2,
    EVENT_ALARM_STOPPED = 3,
    EVENT_ALARM_SNOOZED = 4,
    EVENT_PILL_BOX_OPENED = 5,
    EVENT_PILL_BOX_CLOSED = 6,
    EVENT_BEDTIME_REMINDER = 7,
    EVENT_USB_CONNECTED = 8,
    EVENT_USB_DISCONNECTED = 9,
    EVENT_WIFI_CONNECTED = 10,
    EVENT_WIFI_DISCONNECTED = 11,
    EVENT_OTA_START = 12,
    EVENT_OTA_SUCCESS = 13,
    EVENT_OTA_FAILED = 14,
    EVENT_LOW_BATTERY = 15,
    EVENT_SENSOR_ERROR = 16
};

struct LogEntry {
    unsigned long timestamp;
    LogLevel level;
    LogEventType eventType;
    String message;
    String data;
};

class Logger {
private:
    std::vector<LogEntry> logBuffer;
    Preferences preferences;
    bool flashLoggingEnabled;
    bool serialLoggingEnabled;
    
    String levelToString(LogLevel level);
    String eventTypeToString(LogEventType eventType);
    void writeToFlash(const LogEntry& entry);
    void printToSerial(const LogEntry& entry);

public:
    Logger();
    ~Logger();
    
    bool begin();
    void log(LogLevel level, LogEventType eventType, const String& message, const String& data = "");
    void logDebug(LogEventType eventType, const String& message, const String& data = "");
    void logInfo(LogEventType eventType, const String& message, const String& data = "");
    void logWarning(LogEventType eventType, const String& message, const String& data = "");
    void logError(LogEventType eventType, const String& message, const String& data = "");
    
    std::vector<LogEntry> getRecentLogs(int count = 10);
    void clearLogs();
    void enableFlashLogging(bool enable);
    void enableSerialLogging(bool enable);
    String getLogsSummary();
    void exportLogsToString(String& output);
};

#endif // LOGGER_H
