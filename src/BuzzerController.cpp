/**
 * @file BuzzerController.cpp
 * @brief PWM-based buzzer controller implementation for ESP32
 * @author Nighty Byte Team
 * @date 2025-08-25
 */

#include "BuzzerController.h"

// Pattern definitions (frequency in Hz, duration in ms, pause in ms)
const BuzzerTone BuzzerController::alarmPattern[] = {
    {2000, 500, 200},
    {2500, 500, 200},
    {2000, 500, 200},
    {2500, 500, 1000},
    {0, 0, 0} // End marker
};

const BuzzerTone BuzzerController::successPattern[] = {
    {1000, 100, 50},
    {1500, 100, 50},
    {2000, 200, 0},
    {0, 0, 0} // End marker
};

const BuzzerTone BuzzerController::errorPattern[] = {
    {500, 300, 100},
    {400, 300, 100},
    {300, 500, 0},
    {0, 0, 0} // End marker
};

const BuzzerTone BuzzerController::notificationPattern[] = {
    {1500, 200, 200},
    {1500, 200, 0},
    {0, 0, 0} // End marker
};

BuzzerController::BuzzerController(Logger* log) {
    logger = log;
    buzzerPin = BUZZER_PIN;
    pwmChannel = BUZZER_CHANNEL;
    currentFrequency = BUZZER_FREQUENCY;
    
    // Initialize state
    currentPattern = PATTERN_OFF;
    isActive = false;
    patternStartTime = 0;
    lastToggleTime = 0;
    patternStep = 0;
    toneOn = false;
}

BuzzerController::~BuzzerController() {
    stopPattern();
}

bool BuzzerController::begin(int pin, int channel) {
    buzzerPin = pin;
    pwmChannel = channel;
    
    // Configure PWM for buzzer
    // ESP32 PWM: frequency, resolution (bits), channel
    ledcSetup(pwmChannel, currentFrequency, 8); // 8-bit resolution (0-255)
    ledcAttachPin(buzzerPin, pwmChannel);
    
    // Start with buzzer off
    ledcWrite(pwmChannel, 0);
    
    if (logger) {
        logger->logInfo(EVENT_SYSTEM_START, "BuzzerController initialized",
                       "Pin: " + String(buzzerPin) + ", Channel: " + String(pwmChannel) +
                       ", Freq: " + String(currentFrequency) + "Hz");
    }
    
    // Play startup tone to confirm buzzer is working
    playStartupTone();
    
    return true;
}

void BuzzerController::update() {
    if (currentPattern != PATTERN_OFF && currentPattern != PATTERN_CONTINUOUS) {
        updatePattern();
    }
}

void BuzzerController::setBuzzer(bool enabled) {
    if (enabled) {
        playPattern(PATTERN_CONTINUOUS);
    } else {
        stopPattern();
    }
}

void BuzzerController::playPattern(BuzzerPattern pattern) {
    currentPattern = pattern;
    patternStartTime = millis();
    lastToggleTime = millis();
    patternStep = 0;
    toneOn = false;
    
    switch (pattern) {
        case PATTERN_OFF:
            stopTone();
            isActive = false;
            break;
            
        case PATTERN_CONTINUOUS:
            playTone(currentFrequency);
            isActive = true;
            break;
            
        case PATTERN_BEEP_FAST:
        case PATTERN_BEEP_SLOW:
        case PATTERN_PULSE:
            isActive = true;
            break;
            
        case PATTERN_ALARM:
            isActive = true;
            executePatternStep(alarmPattern, sizeof(alarmPattern) / sizeof(BuzzerTone));
            break;
            
        case PATTERN_SUCCESS:
            isActive = true;
            executePatternStep(successPattern, sizeof(successPattern) / sizeof(BuzzerTone));
            break;
            
        case PATTERN_ERROR:
            isActive = true;
            executePatternStep(errorPattern, sizeof(errorPattern) / sizeof(BuzzerTone));
            break;
            
        case PATTERN_NOTIFICATION:
            isActive = true;
            executePatternStep(notificationPattern, sizeof(notificationPattern) / sizeof(BuzzerTone));
            break;
    }
    
    if (logger && pattern != PATTERN_OFF) {
        logger->logDebug(EVENT_SYSTEM_START, "Buzzer pattern started", String(pattern));
    }
}

void BuzzerController::stopPattern() {
    currentPattern = PATTERN_OFF;
    stopTone();
    isActive = false;
    
    if (logger) {
        logger->logDebug(EVENT_SYSTEM_START, "Buzzer pattern stopped");
    }
}

void BuzzerController::playTone(int frequency, int duration) {
    if (frequency > 0) {
        // Update PWM frequency and start tone
        ledcChangeFrequency(pwmChannel, frequency, 8);
        ledcWrite(pwmChannel, 128); // 50% duty cycle (volume control)
        currentFrequency = frequency;
        
        // If duration is specified, set up auto-stop
        if (duration > 0) {
            // Note: For simplicity, we don't implement auto-stop here
            // In a more complex implementation, you could use a timer
        }
    } else {
        stopTone();
    }
}

void BuzzerController::stopTone() {
    ledcWrite(pwmChannel, 0); // 0% duty cycle = silence
}

void BuzzerController::updatePattern() {
    unsigned long currentTime = millis();
    
    switch (currentPattern) {
        case PATTERN_BEEP_FAST:
            if (currentTime - lastToggleTime >= 250) { // 250ms on/off
                toneOn = !toneOn;
                if (toneOn) {
                    playTone(currentFrequency);
                } else {
                    stopTone();
                }
                lastToggleTime = currentTime;
            }
            break;
            
        case PATTERN_BEEP_SLOW:
            if (currentTime - lastToggleTime >= 1000) { // 1000ms on/off
                toneOn = !toneOn;
                if (toneOn) {
                    playTone(currentFrequency);
                } else {
                    stopTone();
                }
                lastToggleTime = currentTime;
            }
            break;
            
        case PATTERN_PULSE:
            // Implement a pulsing pattern with varying intensity
            if (currentTime - lastToggleTime >= 100) {
                static int intensity = 0;
                static int direction = 1;
                
                intensity += direction * 32;
                if (intensity >= 255) {
                    intensity = 255;
                    direction = -1;
                } else if (intensity <= 0) {
                    intensity = 0;
                    direction = 1;
                }
                
                ledcWrite(pwmChannel, intensity);
                lastToggleTime = currentTime;
            }
            break;
            
        case PATTERN_ALARM:
            executePatternStep(alarmPattern, sizeof(alarmPattern) / sizeof(BuzzerTone));
            break;
            
        case PATTERN_SUCCESS:
            executePatternStep(successPattern, sizeof(successPattern) / sizeof(BuzzerTone));
            break;
            
        case PATTERN_ERROR:
            executePatternStep(errorPattern, sizeof(errorPattern) / sizeof(BuzzerTone));
            break;
            
        case PATTERN_NOTIFICATION:
            executePatternStep(notificationPattern, sizeof(notificationPattern) / sizeof(BuzzerTone));
            break;
            
        default:
            break;
    }
}

void BuzzerController::executePatternStep(const BuzzerTone* pattern, int patternLength) {
    unsigned long currentTime = millis();
    
    // Check if current step is complete
    if (!toneOn) {
        // Start new tone
        if (patternStep < patternLength - 1 && pattern[patternStep].frequency != 0) {
            playTone(pattern[patternStep].frequency);
            toneOn = true;
            lastToggleTime = currentTime;
        } else {
            // End of pattern, restart or stop
            if (currentPattern == PATTERN_ALARM) {
                // Restart alarm pattern
                patternStep = 0;
            } else {
                // Stop pattern
                stopPattern();
                return;
            }
        }
    } else {
        // Check if tone duration is complete
        if (currentTime - lastToggleTime >= pattern[patternStep].duration) {
            stopTone();
            toneOn = false;
            lastToggleTime = currentTime;
            
            // Move to pause phase or next step
            if (pattern[patternStep].pause > 0) {
                // Wait for pause duration before next step
                if (currentTime - lastToggleTime >= pattern[patternStep].pause) {
                    patternStep++;
                }
            } else {
                patternStep++;
            }
        }
    }
}

void BuzzerController::playTone(int frequency) {
    playTone(frequency, 0); // Continuous tone
}

void BuzzerController::playBeep(int frequency, int duration) {
    playTone(frequency);
    delay(duration); // Simple blocking beep
    stopTone();
}

void BuzzerController::playDoubleBeep() {
    playBeep(currentFrequency, 100);
    delay(100);
    playBeep(currentFrequency, 100);
}

void BuzzerController::playTripleBeep() {
    for (int i = 0; i < 3; i++) {
        playBeep(currentFrequency, 100);
        if (i < 2) delay(100);
    }
}

void BuzzerController::setDefaultFrequency(int frequency) {
    currentFrequency = frequency;
    if (logger) {
        logger->logDebug(EVENT_SYSTEM_START, "Buzzer frequency changed", String(frequency) + "Hz");
    }
}

void BuzzerController::performBuzzerTest() {
    if (logger) {
        logger->logInfo(EVENT_SYSTEM_START, "Starting buzzer test");
    }
    
    // Test different frequencies
    int testFrequencies[] = {500, 1000, 1500, 2000, 2500};
    int numFreqs = sizeof(testFrequencies) / sizeof(int);
    
    for (int i = 0; i < numFreqs; i++) {
        if (logger) {
            logger->logDebug(EVENT_SYSTEM_START, "Testing frequency", String(testFrequencies[i]) + "Hz");
        }
        playBeep(testFrequencies[i], 300);
        delay(200);
    }
    
    // Test patterns
    playPattern(PATTERN_SUCCESS);
    delay(2000);
    stopPattern();
    
    delay(500);
    
    playPattern(PATTERN_ERROR);
    delay(2000);
    stopPattern();
    
    if (logger) {
        logger->logInfo(EVENT_SYSTEM_START, "Buzzer test completed");
    }
}

void BuzzerController::playStartupTone() {
    // Simple ascending tone sequence to indicate system ready
    playBeep(1000, 150);
    delay(50);
    playBeep(1500, 150);
    delay(50);
    playBeep(2000, 200);
}
