/**
 * @file BuzzerController.h
 * @brief PWM-based buzzer controller for 3.3V buzzers on ESP32
 * @author Nighty Byte Team
 * @date 2025-08-25
 */

#ifndef BUZZER_CONTROLLER_H
#define BUZZER_CONTROLLER_H

#include <Arduino.h>
#include "config.h"
#include "Logger.h"

enum BuzzerPattern {
    PATTERN_OFF,
    PATTERN_CONTINUOUS,
    PATTERN_BEEP_FAST,
    PATTERN_BEEP_SLOW,
    PATTERN_PULSE,
    PATTERN_ALARM,
    PATTERN_SUCCESS,
    PATTERN_ERROR,
    PATTERN_NOTIFICATION
};

struct BuzzerTone {
    int frequency;
    int duration;
    int pause;
};

class BuzzerController {
private:
    Logger* logger;
    
    // PWM settings
    int buzzerPin;
    int pwmChannel;
    int currentFrequency;
    
    // Pattern control
    BuzzerPattern currentPattern;
    bool isActive;
    unsigned long patternStartTime;
    unsigned long lastToggleTime;
    int patternStep;
    bool toneOn;
    
    // Pattern definitions
    static const BuzzerTone alarmPattern[];
    static const BuzzerTone successPattern[];
    static const BuzzerTone errorPattern[];
    static const BuzzerTone notificationPattern[];
    
    void playTone(int frequency, int duration = 0);
    void stopTone();
    void updatePattern();
    void executePatternStep(const BuzzerTone* pattern, int patternLength);

public:
    BuzzerController(Logger* log);
    ~BuzzerController();
    
    bool begin(int pin = BUZZER_PIN, int channel = BUZZER_CHANNEL);
    void update(); // Call frequently in main loop
    
    // Basic control
    void setBuzzer(bool enabled);
    void playPattern(BuzzerPattern pattern);
    void stopPattern();
    
    // Custom tones
    void playTone(int frequency);
    void playBeep(int frequency = BUZZER_FREQUENCY, int duration = 200);
    void playDoubleBeep();
    void playTripleBeep();
    
    // Status
    bool isPlaying() const { return isActive; }
    BuzzerPattern getCurrentPattern() const { return currentPattern; }
    
    // Configuration
    void setDefaultFrequency(int frequency);
    int getDefaultFrequency() const { return BUZZER_FREQUENCY; }
    
    // Test functions
    void performBuzzerTest();
    void playStartupTone();
};

#endif // BUZZER_CONTROLLER_H
