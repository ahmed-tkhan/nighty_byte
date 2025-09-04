/**
 * @file SensorManager.cpp
 * @brief Sensor management implementation for ESP32 Smart Alarm
 * @author Nighty Byte Team
 * @date 2025-08-25
 */

#include "SensorManager.h"

SensorManager::SensorManager(Logger* log) {
    logger = log;
    
    // Initialize sensor states
    currentLightLevel = 0;
    currentUsbState = false;
    currentPillBoxState = false;
    previousPillBoxState = false;
    
    // Initialize timing
    lastLightRead = 0;
    lastUsbRead = 0;
    lastPillBoxRead = 0;
    pillBoxDebounceTime = 0;
    pillBoxRawState = false;
    
    // Initialize light sensor filtering
    lightReadIndex = 0;
    lightTotal = 0;
    lightSamplesInitialized = false;
    for (int i = 0; i < LIGHT_SAMPLES; i++) {
        lightReadings[i] = 0;
    }
}

SensorManager::~SensorManager() {
    // Cleanup if needed
}

bool SensorManager::begin() {
    // Configure pill box switch pin (input with internal pullup)
    pinMode(PILL_BOX_SWITCH_PIN, INPUT_PULLUP);
    
    // ADC pins don't need pinMode configuration on ESP32
    // GPIO36 and GPIO39 are input-only pins, perfect for ADC
    
    // Initialize light sensor samples
    initializeLightSamples();
    
    // Read initial states
    readLightSensor();
    readUsbState();
    readPillBoxState();
    
    if (logger) {
        logger->logInfo(EVENT_SYSTEM_START, "SensorManager initialized",
                       "Light: " + String(currentLightLevel) + 
                       ", USB: " + (currentUsbState ? "Connected" : "Disconnected") +
                       ", PillBox: " + (currentPillBoxState ? "Open" : "Closed"));
    }
    
    return true;
}

void SensorManager::update() {
    unsigned long currentTime = millis();
    
    // Read light sensor at specified interval
    if (currentTime - lastLightRead >= LIGHT_SENSOR_INTERVAL_MS) {
        readLightSensor();
        lastLightRead = currentTime;
    }
    
    // Read USB state at specified interval
    if (currentTime - lastUsbRead >= USB_DETECT_INTERVAL_MS) {
        readUsbState();
        lastUsbRead = currentTime;
    }
    
    // Read pill box state frequently (important for alarm dismissal)
    if (currentTime - lastPillBoxRead >= PILL_BOX_CHECK_INTERVAL_MS) {
        readPillBoxState();
        lastPillBoxRead = currentTime;
    }
}

void SensorManager::readLightSensor() {
    // Read raw ADC value
    int rawReading = analogRead(LIGHT_SENSOR_PIN);
    
    // Add to rolling average buffer
    lightTotal = lightTotal - lightReadings[lightReadIndex];
    lightReadings[lightReadIndex] = rawReading;
    lightTotal = lightTotal + lightReadings[lightReadIndex];
    lightReadIndex = (lightReadIndex + 1) % LIGHT_SAMPLES;
    
    // Calculate average
    int newLightLevel = lightTotal / LIGHT_SAMPLES;
    
    // Check for significant change or first reading
    if (!lightSamplesInitialized || abs(newLightLevel - currentLightLevel) > 50) {
        bool wasDark = currentLightLevel < BEDTIME_LIGHT_THRESHOLD;
        currentLightLevel = newLightLevel;
        bool isDark = currentLightLevel < BEDTIME_LIGHT_THRESHOLD;
        
        // Trigger bedtime callback if transition from light to dark
        if (!wasDark && isDark && bedtimeCallback) {
            bedtimeCallback(true);
        }
        
        if (logger && lightSamplesInitialized) {
            logger->logDebug(EVENT_SENSOR_ERROR, "Light level changed",
                           "Level: " + String(currentLightLevel) + 
                           " (" + (isDark ? "Dark" : "Light") + ")");
        }
    }
    
    lightSamplesInitialized = true;
}

void SensorManager::readUsbState() {
    // Read ADC value from USB detection pin
    int usbReading = analogRead(USB_DETECT_PIN);
    bool newUsbState = usbReading > USB_VOLTAGE_THRESHOLD;
    
    // Check for state change
    if (newUsbState != currentUsbState) {
        currentUsbState = newUsbState;
        
        // Trigger callback
        if (usbStateCallback) {
            usbStateCallback(currentUsbState);
        }
        
        if (logger) {
            logger->logInfo(currentUsbState ? EVENT_USB_CONNECTED : EVENT_USB_DISCONNECTED,
                           currentUsbState ? "USB charging connected" : "USB charging disconnected",
                           "ADC Reading: " + String(usbReading));
        }
    }
}

void SensorManager::readPillBoxState() {
    unsigned long currentTime = millis();
    
    // Read raw digital state (LOW = pressed/closed, HIGH = open due to pullup)
    bool rawState = digitalRead(PILL_BOX_SWITCH_PIN) == HIGH;
    
    // Debouncing logic
    if (rawState != pillBoxRawState) {
        pillBoxDebounceTime = currentTime;
        pillBoxRawState = rawState;
    }
    
    // Check if debounce period has passed
    if (currentTime - pillBoxDebounceTime >= PILL_BOX_DEBOUNCE_MS) {
        if (pillBoxRawState != currentPillBoxState) {
            previousPillBoxState = currentPillBoxState;
            currentPillBoxState = pillBoxRawState;
            
            // Trigger callback
            if (pillBoxCallback) {
                pillBoxCallback(currentPillBoxState);
            }
            
            if (logger) {
                logger->logInfo(currentPillBoxState ? EVENT_PILL_BOX_OPENED : EVENT_PILL_BOX_CLOSED,
                               currentPillBoxState ? "Pill box opened" : "Pill box closed");
            }
        }
    }
}

void SensorManager::initializeLightSamples() {
    // Fill the light sensor buffer with initial readings
    for (int i = 0; i < LIGHT_SAMPLES; i++) {
        lightReadings[i] = analogRead(LIGHT_SENSOR_PIN);
        lightTotal += lightReadings[i];
        delay(LIGHT_SAMPLE_DELAY_MS);
    }
    currentLightLevel = lightTotal / LIGHT_SAMPLES;
}

int SensorManager::getAverageLightLevel() {
    return lightSamplesInitialized ? (lightTotal / LIGHT_SAMPLES) : currentLightLevel;
}

SensorReadings SensorManager::getCurrentReadings() {
    SensorReadings readings;
    readings.lightLevel = currentLightLevel;
    readings.usbConnected = currentUsbState;
    readings.pillBoxOpen = currentPillBoxState;
    readings.batteryVoltage = 0.0; // TODO: Implement battery voltage reading
    readings.timestamp = millis();
    
    return readings;
}

void SensorManager::calibrateLightSensor() {
    if (logger) {
        logger->logInfo(EVENT_SYSTEM_START, "Starting light sensor calibration");
    }
    
    // Reset the light sensor buffer
    lightTotal = 0;
    lightReadIndex = 0;
    lightSamplesInitialized = false;
    
    // Take new samples
    initializeLightSamples();
    
    if (logger) {
        logger->logInfo(EVENT_SYSTEM_START, "Light sensor calibrated",
                       "New baseline: " + String(currentLightLevel));
    }
}

void SensorManager::setLightThreshold(int threshold) {
    // Note: This would require modifying the config or storing in preferences
    // For now, we'll just log the request
    if (logger) {
        logger->logInfo(EVENT_SYSTEM_START, "Light threshold change requested",
                       "New threshold: " + String(threshold));
    }
}

String SensorManager::getSensorStatus() {
    String status = "Sensor Status:\n";
    status += "Light Level: " + String(currentLightLevel) + "/4095";
    status += " (" + (isDarkEnvironment() ? "Dark" : "Light") + ")\n";
    status += "USB Connected: " + String(currentUsbState ? "Yes" : "No") + "\n";
    status += "Pill Box: " + String(currentPillBoxState ? "Open" : "Closed") + "\n";
    status += "Last Update: " + String(millis()) + "ms\n";
    
    return status;
}

void SensorManager::performSensorTest() {
    if (logger) {
        logger->logInfo(EVENT_SYSTEM_START, "Starting sensor diagnostic test");
    }
    
    // Test light sensor
    int lightMin = 4095, lightMax = 0;
    for (int i = 0; i < 10; i++) {
        int reading = analogRead(LIGHT_SENSOR_PIN);
        lightMin = min(lightMin, reading);
        lightMax = max(lightMax, reading);
        delay(100);
    }
    
    if (logger) {
        logger->logInfo(EVENT_SYSTEM_START, "Light sensor test complete",
                       "Min: " + String(lightMin) + ", Max: " + String(lightMax) +
                       ", Range: " + String(lightMax - lightMin));
    }
    
    // Test USB detection
    int usbReading = analogRead(USB_DETECT_PIN);
    if (logger) {
        logger->logInfo(EVENT_SYSTEM_START, "USB detection test",
                       "ADC Reading: " + String(usbReading) + 
                       " (" + (usbReading > USB_VOLTAGE_THRESHOLD ? "Connected" : "Disconnected") + ")");
    }
    
    // Test pill box switch
    bool switchState = digitalRead(PILL_BOX_SWITCH_PIN) == HIGH;
    if (logger) {
        logger->logInfo(EVENT_SYSTEM_START, "Pill box switch test",
                       "State: " + String(switchState ? "Open" : "Closed"));
    }
    
    if (logger) {
        logger->logInfo(EVENT_SYSTEM_START, "Sensor diagnostic test completed");
    }
}
