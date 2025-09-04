/**
 * @file AlarmManager.h
 * @brief Alarm scheduling and management for ESP32 Smart Alarm
 * @author Nighty Byte Team
 * @date 2025-08-25
 */

#ifndef ALARM_MANAGER_H
#define ALARM_MANAGER_H

#include <Arduino.h>
#include <vector>
#include <ESP32Time.h>
#include <Preferences.h>
#include "config.h"
#include "Logger.h"

// Alarm structure
struct Alarm {
    uint8_t id;
    uint8_t hour;           // 0-23
    uint8_t minute;         // 0-59
    uint8_t dayMask;        // Bit mask: bit 0=Sunday, bit 1=Monday, etc.
    bool enabled;
    String label;
    bool repeating;         // True for recurring alarms, false for one-time
    time_t oneTimeDate;     // Unix timestamp for one-time alarms
    
    Alarm() : id(0), hour(0), minute(0), dayMask(0), enabled(false), 
              label(""), repeating(true), oneTimeDate(0) {}
};

enum AlarmState {
    ALARM_IDLE,
    ALARM_TRIGGERED,
    ALARM_SNOOZED,
    ALARM_WAITING_FOR_PILL_BOX
};

class AlarmManager {
private:
    std::vector<Alarm> alarms;
    Preferences preferences;
    Logger* logger;
    ESP32Time* rtc;
    
    // Current alarm state
    AlarmState currentState;
    uint8_t activeAlarmId;
    unsigned long alarmStartTime;
    unsigned long snoozeStartTime;
    bool buzzerActive;
    
    // Hardware interaction callbacks
    std::function<void(bool)> buzzerCallback;
    std::function<bool()> pillBoxCallback;
    
    void saveAlarmsToFlash();
    void loadAlarmsFromFlash();
    bool isAlarmTimeMatched(const Alarm& alarm, struct tm& timeinfo);
    bool isDayMatched(uint8_t dayMask, int weekday);
    void triggerAlarm(uint8_t alarmId);
    void stopAlarm();

public:
    AlarmManager(Logger* log, ESP32Time* rtcInstance);
    ~AlarmManager();
    
    bool begin();
    void update(); // Call this frequently in main loop
    
    // Alarm management
    bool addAlarm(uint8_t hour, uint8_t minute, uint8_t dayMask, const String& label = "");
    bool addOneTimeAlarm(uint8_t hour, uint8_t minute, time_t date, const String& label = "");
    bool removeAlarm(uint8_t alarmId);
    bool enableAlarm(uint8_t alarmId, bool enabled);
    bool modifyAlarm(uint8_t alarmId, uint8_t hour, uint8_t minute, uint8_t dayMask);
    void clearAllAlarms();
    
    // State management
    AlarmState getState() const { return currentState; }
    uint8_t getActiveAlarmId() const { return activeAlarmId; }
    bool snoozeCurrentAlarm();
    void dismissCurrentAlarm();
    void onPillBoxOpened();
    
    // Getters
    std::vector<Alarm> getAlarms() const { return alarms; }
    Alarm* getAlarm(uint8_t alarmId);
    String getAlarmsStatus();
    unsigned long getAlarmDuration() const;
    
    // Hardware callbacks
    void setBuzzerCallback(std::function<void(bool)> callback) { buzzerCallback = callback; }
    void setPillBoxCallback(std::function<bool()> callback) { pillBoxCallback = callback; }
    
    // Utility
    static String dayMaskToString(uint8_t dayMask);
    static uint8_t stringToDayMask(const String& days);
    static String formatTime(uint8_t hour, uint8_t minute);
};

#endif // ALARM_MANAGER_H
