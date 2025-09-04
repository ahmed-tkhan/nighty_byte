/**
 * @file AlarmManager.cpp
 * @brief Alarm scheduling and management implementation for ESP32 Smart Alarm
 * @author Nighty Byte Team
 * @date 2025-08-25
 */

#include "AlarmManager.h"

AlarmManager::AlarmManager(Logger* log, ESP32Time* rtcInstance) {
    logger = log;
    rtc = rtcInstance;
    currentState = ALARM_IDLE;
    activeAlarmId = 0;
    alarmStartTime = 0;
    snoozeStartTime = 0;
    buzzerActive = false;
    alarms.reserve(MAX_ALARMS);
}

AlarmManager::~AlarmManager() {
    preferences.end();
}

bool AlarmManager::begin() {
    if (!preferences.begin("alarms", false)) {
        if (logger) logger->logError(EVENT_SYSTEM_START, "Failed to initialize alarm preferences");
        return false;
    }
    
    loadAlarmsFromFlash();
    
    if (logger) {
        logger->logInfo(EVENT_SYSTEM_START, "AlarmManager initialized", 
                       "Loaded " + String(alarms.size()) + " alarms");
    }
    
    return true;
}

void AlarmManager::update() {
    unsigned long currentTime = millis();
    struct tm timeinfo;
    
    // Get current time from RTC
    if (!rtc || !getLocalTime(&timeinfo)) {
        // If RTC is not available, we can't process alarms
        return;
    }
    
    switch (currentState) {
        case ALARM_IDLE:
            // Check if any alarm should trigger
            for (const auto& alarm : alarms) {
                if (alarm.enabled && isAlarmTimeMatched(alarm, timeinfo)) {
                    triggerAlarm(alarm.id);
                    break;
                }
            }
            break;
            
        case ALARM_TRIGGERED:
            // Check if pill box is opened
            if (pillBoxCallback && pillBoxCallback()) {
                onPillBoxOpened();
            }
            
            // Check if maximum buzzer time exceeded
            if (currentTime - alarmStartTime >= ALARM_BUZZER_DURATION_MS) {
                if (logger) {
                    logger->logWarning(EVENT_ALARM_STOPPED, 
                                     "Alarm auto-stopped after maximum duration",
                                     "AlarmId: " + String(activeAlarmId));
                }
                stopAlarm();
            }
            
            // Keep buzzer active
            if (buzzerCallback && !buzzerActive) {
                buzzerCallback(true);
                buzzerActive = true;
            }
            break;
            
        case ALARM_SNOOZED:
            // Check if snooze period is over
            if (currentTime - snoozeStartTime >= ALARM_SNOOZE_DURATION_MS) {
                // Re-trigger the alarm
                currentState = ALARM_TRIGGERED;
                alarmStartTime = currentTime;
                if (logger) {
                    logger->logInfo(EVENT_ALARM_TRIGGERED, 
                                   "Alarm re-triggered after snooze",
                                   "AlarmId: " + String(activeAlarmId));
                }
            }
            break;
            
        case ALARM_WAITING_FOR_PILL_BOX:
            // This state is used when pill box was opened but we want to wait
            // for it to be closed again before going back to idle
            if (pillBoxCallback && !pillBoxCallback()) {
                // Pill box is closed, go back to idle
                currentState = ALARM_IDLE;
                activeAlarmId = 0;
                if (logger) {
                    logger->logInfo(EVENT_PILL_BOX_CLOSED, "Pill box closed, alarm cycle complete");
                }
            }
            break;
    }
}

bool AlarmManager::addAlarm(uint8_t hour, uint8_t minute, uint8_t dayMask, const String& label) {
    if (alarms.size() >= MAX_ALARMS) {
        if (logger) logger->logError(EVENT_ALARM_SET, "Cannot add alarm: maximum limit reached");
        return false;
    }
    
    if (hour > 23 || minute > 59) {
        if (logger) logger->logError(EVENT_ALARM_SET, "Invalid time format", 
                                   "Hour: " + String(hour) + ", Minute: " + String(minute));
        return false;
    }
    
    Alarm newAlarm;
    newAlarm.id = alarms.size() + 1; // Simple ID assignment
    newAlarm.hour = hour;
    newAlarm.minute = minute;
    newAlarm.dayMask = dayMask;
    newAlarm.enabled = true;
    newAlarm.label = label;
    newAlarm.repeating = true;
    
    alarms.push_back(newAlarm);
    saveAlarmsToFlash();
    
    if (logger) {
        logger->logInfo(EVENT_ALARM_SET, "Alarm added", 
                       "ID: " + String(newAlarm.id) + ", Time: " + formatTime(hour, minute) +
                       ", Days: " + dayMaskToString(dayMask) + ", Label: " + label);
    }
    
    return true;
}

bool AlarmManager::addOneTimeAlarm(uint8_t hour, uint8_t minute, time_t date, const String& label) {
    if (alarms.size() >= MAX_ALARMS) {
        if (logger) logger->logError(EVENT_ALARM_SET, "Cannot add one-time alarm: maximum limit reached");
        return false;
    }
    
    if (hour > 23 || minute > 59) {
        if (logger) logger->logError(EVENT_ALARM_SET, "Invalid time format for one-time alarm");
        return false;
    }
    
    Alarm newAlarm;
    newAlarm.id = alarms.size() + 1;
    newAlarm.hour = hour;
    newAlarm.minute = minute;
    newAlarm.dayMask = 0; // Not used for one-time alarms
    newAlarm.enabled = true;
    newAlarm.label = label;
    newAlarm.repeating = false;
    newAlarm.oneTimeDate = date;
    
    alarms.push_back(newAlarm);
    saveAlarmsToFlash();
    
    if (logger) {
        logger->logInfo(EVENT_ALARM_SET, "One-time alarm added", 
                       "ID: " + String(newAlarm.id) + ", Time: " + formatTime(hour, minute));
    }
    
    return true;
}

bool AlarmManager::removeAlarm(uint8_t alarmId) {
    for (auto it = alarms.begin(); it != alarms.end(); ++it) {
        if (it->id == alarmId) {
            if (logger) {
                logger->logInfo(EVENT_ALARM_SET, "Alarm removed", "ID: " + String(alarmId));
            }
            alarms.erase(it);
            saveAlarmsToFlash();
            return true;
        }
    }
    return false;
}

bool AlarmManager::enableAlarm(uint8_t alarmId, bool enabled) {
    for (auto& alarm : alarms) {
        if (alarm.id == alarmId) {
            alarm.enabled = enabled;
            saveAlarmsToFlash();
            if (logger) {
                logger->logInfo(EVENT_ALARM_SET, 
                               enabled ? "Alarm enabled" : "Alarm disabled",
                               "ID: " + String(alarmId));
            }
            return true;
        }
    }
    return false;
}

bool AlarmManager::modifyAlarm(uint8_t alarmId, uint8_t hour, uint8_t minute, uint8_t dayMask) {
    if (hour > 23 || minute > 59) {
        return false;
    }
    
    for (auto& alarm : alarms) {
        if (alarm.id == alarmId) {
            alarm.hour = hour;
            alarm.minute = minute;
            alarm.dayMask = dayMask;
            saveAlarmsToFlash();
            if (logger) {
                logger->logInfo(EVENT_ALARM_SET, "Alarm modified", 
                               "ID: " + String(alarmId) + ", Time: " + formatTime(hour, minute));
            }
            return true;
        }
    }
    return false;
}

void AlarmManager::clearAllAlarms() {
    alarms.clear();
    preferences.clear();
    if (logger) {
        logger->logInfo(EVENT_ALARM_SET, "All alarms cleared");
    }
}

bool AlarmManager::snoozeCurrentAlarm() {
    if (currentState != ALARM_TRIGGERED) {
        return false;
    }
    
    currentState = ALARM_SNOOZED;
    snoozeStartTime = millis();
    
    // Turn off buzzer
    if (buzzerCallback) {
        buzzerCallback(false);
        buzzerActive = false;
    }
    
    if (logger) {
        logger->logInfo(EVENT_ALARM_SNOOZED, "Alarm snoozed", 
                       "AlarmId: " + String(activeAlarmId) + 
                       ", Duration: " + String(ALARM_SNOOZE_DURATION_MS / 1000) + "s");
    }
    
    return true;
}

void AlarmManager::dismissCurrentAlarm() {
    stopAlarm();
}

void AlarmManager::onPillBoxOpened() {
    if (currentState == ALARM_TRIGGERED) {
        stopAlarm();
        if (logger) {
            logger->logInfo(EVENT_PILL_BOX_OPENED, "Pill box opened, alarm dismissed",
                           "AlarmId: " + String(activeAlarmId));
        }
        
        // Transition to waiting state to detect when pill box is closed
        currentState = ALARM_WAITING_FOR_PILL_BOX;
    }
}

Alarm* AlarmManager::getAlarm(uint8_t alarmId) {
    for (auto& alarm : alarms) {
        if (alarm.id == alarmId) {
            return &alarm;
        }
    }
    return nullptr;
}

String AlarmManager::getAlarmsStatus() {
    String status = "Alarms (" + String(alarms.size()) + "/" + String(MAX_ALARMS) + "):\n";
    
    for (const auto& alarm : alarms) {
        status += "ID " + String(alarm.id) + ": ";
        status += formatTime(alarm.hour, alarm.minute) + " ";
        status += dayMaskToString(alarm.dayMask) + " ";
        status += (alarm.enabled ? "[ON]" : "[OFF]");
        if (!alarm.label.isEmpty()) {
            status += " '" + alarm.label + "'";
        }
        status += "\n";
    }
    
    return status;
}

unsigned long AlarmManager::getAlarmDuration() const {
    if (currentState == ALARM_TRIGGERED || currentState == ALARM_WAITING_FOR_PILL_BOX) {
        return millis() - alarmStartTime;
    }
    return 0;
}

void AlarmManager::triggerAlarm(uint8_t alarmId) {
    currentState = ALARM_TRIGGERED;
    activeAlarmId = alarmId;
    alarmStartTime = millis();
    
    // Turn on buzzer
    if (buzzerCallback) {
        buzzerCallback(true);
        buzzerActive = true;
    }
    
    if (logger) {
        Alarm* alarm = getAlarm(alarmId);
        String alarmInfo = alarm ? ("'" + alarm->label + "'") : "";
        logger->logInfo(EVENT_ALARM_TRIGGERED, "Alarm triggered", 
                       "AlarmId: " + String(alarmId) + " " + alarmInfo);
    }
}

void AlarmManager::stopAlarm() {
    currentState = ALARM_IDLE;
    
    // Turn off buzzer
    if (buzzerCallback) {
        buzzerCallback(false);
        buzzerActive = false;
    }
    
    if (logger && activeAlarmId != 0) {
        logger->logInfo(EVENT_ALARM_STOPPED, "Alarm stopped", 
                       "AlarmId: " + String(activeAlarmId));
    }
    
    activeAlarmId = 0;
    alarmStartTime = 0;
}

void AlarmManager::saveAlarmsToFlash() {
    preferences.putUInt("count", alarms.size());
    
    for (size_t i = 0; i < alarms.size(); i++) {
        String prefix = "alarm_" + String(i) + "_";
        preferences.putUChar((prefix + "hour").c_str(), alarms[i].hour);
        preferences.putUChar((prefix + "minute").c_str(), alarms[i].minute);
        preferences.putUChar((prefix + "days").c_str(), alarms[i].dayMask);
        preferences.putBool((prefix + "enabled").c_str(), alarms[i].enabled);
        preferences.putString((prefix + "label").c_str(), alarms[i].label);
        preferences.putBool((prefix + "repeat").c_str(), alarms[i].repeating);
        preferences.putULong64((prefix + "date").c_str(), alarms[i].oneTimeDate);
    }
}

void AlarmManager::loadAlarmsFromFlash() {
    alarms.clear();
    
    uint32_t count = preferences.getUInt("count", 0);
    
    for (uint32_t i = 0; i < count && i < MAX_ALARMS; i++) {
        String prefix = "alarm_" + String(i) + "_";
        
        Alarm alarm;
        alarm.id = i + 1;
        alarm.hour = preferences.getUChar((prefix + "hour").c_str(), 0);
        alarm.minute = preferences.getUChar((prefix + "minute").c_str(), 0);
        alarm.dayMask = preferences.getUChar((prefix + "days").c_str(), 0);
        alarm.enabled = preferences.getBool((prefix + "enabled").c_str(), true);
        alarm.label = preferences.getString((prefix + "label").c_str(), "");
        alarm.repeating = preferences.getBool((prefix + "repeat").c_str(), true);
        alarm.oneTimeDate = preferences.getULong64((prefix + "date").c_str(), 0);
        
        alarms.push_back(alarm);
    }
}

bool AlarmManager::isAlarmTimeMatched(const Alarm& alarm, struct tm& timeinfo) {
    // Check time match
    if (timeinfo.tm_hour != alarm.hour || timeinfo.tm_min != alarm.minute) {
        return false;
    }
    
    // For one-time alarms, check date
    if (!alarm.repeating) {
        time_t currentTime = mktime(&timeinfo);
        struct tm alarmTime;
        localtime_r(&alarm.oneTimeDate, &alarmTime);
        
        return (timeinfo.tm_year == alarmTime.tm_year &&
                timeinfo.tm_mon == alarmTime.tm_mon &&
                timeinfo.tm_mday == alarmTime.tm_mday);
    }
    
    // For repeating alarms, check day mask
    return isDayMatched(alarm.dayMask, timeinfo.tm_wday);
}

bool AlarmManager::isDayMatched(uint8_t dayMask, int weekday) {
    if (dayMask == 0) return true; // Daily alarm
    return (dayMask & (1 << weekday)) != 0;
}

String AlarmManager::dayMaskToString(uint8_t dayMask) {
    if (dayMask == 0) return "Daily";
    if (dayMask == 0x7F) return "Daily";
    if (dayMask == 0x3E) return "Weekdays";
    if (dayMask == 0x41) return "Weekends";
    
    String days = "";
    const char* dayNames[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    
    for (int i = 0; i < 7; i++) {
        if (dayMask & (1 << i)) {
            if (!days.isEmpty()) days += ",";
            days += dayNames[i];
        }
    }
    
    return days.isEmpty() ? "None" : days;
}

uint8_t AlarmManager::stringToDayMask(const String& days) {
    if (days.equalsIgnoreCase("daily")) return 0x7F;
    if (days.equalsIgnoreCase("weekdays")) return 0x3E;
    if (days.equalsIgnoreCase("weekends")) return 0x41;
    
    uint8_t mask = 0;
    if (days.indexOf("sun") != -1) mask |= 0x01;
    if (days.indexOf("mon") != -1) mask |= 0x02;
    if (days.indexOf("tue") != -1) mask |= 0x04;
    if (days.indexOf("wed") != -1) mask |= 0x08;
    if (days.indexOf("thu") != -1) mask |= 0x10;
    if (days.indexOf("fri") != -1) mask |= 0x20;
    if (days.indexOf("sat") != -1) mask |= 0x40;
    
    return mask;
}

String AlarmManager::formatTime(uint8_t hour, uint8_t minute) {
    char buffer[6];
    sprintf(buffer, "%02d:%02d", hour, minute);
    return String(buffer);
}
