/**
 * @file config.h
 * @brief Hardware configuration and pin definitions for ESP32 Smart Alarm
 * @author Nighty Byte Team
 * @date 2025-08-25
 */

#ifndef CONFIG_H
#define CONFIG_H

// Hardware Version
#define HARDWARE_VERSION "1.0"
#define FIRMWARE_VERSION "1.0.0"

// GPIO Pin Definitions for ESP32 DevKit V1
// ===========================================

// Buzzer Configuration (3.3V compatible)
#define BUZZER_PIN 2              // GPIO2 - Built-in LED pin, good for buzzer
#define BUZZER_FREQUENCY 2000     // 2kHz frequency for buzzer
#define BUZZER_CHANNEL 0          // PWM channel for buzzer

// Pill Box Contact Switch
#define PILL_BOX_SWITCH_PIN 4     // GPIO4 - Digital input with internal pullup
#define PILL_BOX_DEBOUNCE_MS 50   // Debounce delay in milliseconds

// Light Sensor (LDR with voltage divider)
#define LIGHT_SENSOR_PIN 36       // GPIO36 (ADC1_CH0) - Analog input only
#define LIGHT_SAMPLES 10          // Number of samples to average
#define LIGHT_SAMPLE_DELAY_MS 10  // Delay between samples
#define BEDTIME_LIGHT_THRESHOLD 500 // ADC value below which it's considered dark (0-4095)

// USB Charging Detection
#define USB_DETECT_PIN 39         // GPIO39 (ADC1_CH3) - Analog input only
#define USB_VOLTAGE_THRESHOLD 2048 // ADC threshold for 5V detection (assuming voltage divider)

// Status LED (optional - using built-in LED)
#define STATUS_LED_PIN 2          // GPIO2 - Same as buzzer, will blink when not buzzing

// Serial Communication
#define SERIAL_BAUD_RATE 115200

// WiFi Configuration
#define WIFI_SSID_MAX_LENGTH 32
#define WIFI_PASSWORD_MAX_LENGTH 64
#define WIFI_CONNECT_TIMEOUT_MS 10000

// Time Configuration
#define NTP_SERVER "pool.ntp.org"
#define TIMEZONE_OFFSET_HOURS 0   // UTC offset for your timezone
#define NTP_UPDATE_INTERVAL_MS 3600000 // Update time every hour

// Alarm Configuration
#define MAX_ALARMS 5              // Maximum number of alarms
#define ALARM_BUZZER_DURATION_MS 300000 // 5 minutes maximum buzzer time
#define ALARM_SNOOZE_DURATION_MS 540000 // 9 minutes snooze time

// Logging Configuration
#define LOG_BUFFER_SIZE 1024      // Size of log buffer
#define MAX_LOG_ENTRIES 100       // Maximum log entries to keep in memory
#define LOG_TO_SERIAL true        // Enable serial logging
#define LOG_TO_FLASH true         // Enable flash logging

// Bedtime Reminder Configuration
#define BEDTIME_REMINDER_HOUR 22  // 10 PM default bedtime reminder
#define BEDTIME_REMINDER_MINUTE 0
#define BEDTIME_CHECK_INTERVAL_MS 60000 // Check every minute

// Power Management
#define DEEP_SLEEP_DURATION_US 60000000 // 1 minute deep sleep when idle
#define LOW_BATTERY_THRESHOLD 3.3       // Voltage threshold for low battery warning

// OTA Configuration
#define OTA_PORT 3232
#define OTA_PASSWORD "nightybyte2025"

// Network Settings
#define WEBSOCKET_PORT 81
#define HTTP_PORT 80

// Sensor Reading Intervals
#define LIGHT_SENSOR_INTERVAL_MS 30000    // Read light sensor every 30 seconds
#define USB_DETECT_INTERVAL_MS 5000       // Check USB charging every 5 seconds
#define PILL_BOX_CHECK_INTERVAL_MS 100    // Check pill box very frequently when alarm is active

#endif // CONFIG_H
