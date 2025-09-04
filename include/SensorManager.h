/**
 * @file SensorManager.h
 * @brief Sensor management for ESP32 Smart Alarm (Light, USB detection, Pill box)
 * @author Nighty Byte Team
 * @date 2025-08-25
 */

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include "config.h"
#include "Logger.h"

struct SensorReadings {
    int lightLevel;          // 0-4095 ADC reading
    bool usbConnected;       // USB charging state
    bool pillBoxOpen;        // Pill box contact switch state
    float batteryVoltage;    // Battery voltage (if available)
    unsigned long timestamp; // When readings were taken
};

class SensorManager {
private:
    Logger* logger;
    
    // Sensor states
    int currentLightLevel;
    bool currentUsbState;
    bool currentPillBoxState;
    bool previousPillBoxState;
    
    // Timing for non-blocking sensor reads
    unsigned long lastLightRead;
    unsigned long lastUsbRead;
    unsigned long lastPillBoxRead;
    
    // Debouncing for pill box switch
    unsigned long pillBoxDebounceTime;
    bool pillBoxRawState;
    
    // Light sensor filtering
    int lightReadings[LIGHT_SAMPLES];
    int lightReadIndex;
    int lightTotal;
    bool lightSamplesInitialized;
    
    // Callbacks for events
    std::function<void(bool)> bedtimeCallback;
    std::function<void(bool)> usbStateCallback;
    std::function<void(bool)> pillBoxCallback;
    
    void readLightSensor();
    void readUsbState();
    void readPillBoxState();
    int getAverageLightLevel();
    void initializeLightSamples();

public:
    SensorManager(Logger* log);
    ~SensorManager();
    
    bool begin();
    void update(); // Call frequently in main loop
    
    // Getters
    SensorReadings getCurrentReadings();
    int getLightLevel() const { return currentLightLevel; }
    bool isUsbConnected() const { return currentUsbState; }
    bool isPillBoxOpen() const { return currentPillBoxState; }
    bool isDarkEnvironment() const { return currentLightLevel < BEDTIME_LIGHT_THRESHOLD; }
    
    // Calibration and configuration
    void calibrateLightSensor();
    void setLightThreshold(int threshold);
    int getLightThreshold() const { return BEDTIME_LIGHT_THRESHOLD; }
    
    // Event callbacks
    void setBedtimeCallback(std::function<void(bool)> callback) { bedtimeCallback = callback; }
    void setUsbStateCallback(std::function<void(bool)> callback) { usbStateCallback = callback; }
    void setPillBoxCallback(std::function<void(bool)> callback) { pillBoxCallback = callback; }
    
    // Diagnostic functions
    String getSensorStatus();
    void performSensorTest();
};

#endif // SENSOR_MANAGER_H
