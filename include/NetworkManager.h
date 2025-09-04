/**
 * @file NetworkManager.h
 * @brief WiFi and BLE connectivity for ESP32 Smart Alarm
 * @author Nighty Byte Team
 * @date 2025-08-25
 */

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoOTA.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP32Time.h>
#include <Preferences.h>
#include "config.h"
#include "Logger.h"

enum NetworkState {
    NETWORK_IDLE,
    NETWORK_CONNECTING,
    NETWORK_CONNECTED,
    NETWORK_AP_MODE,
    NETWORK_ERROR
};

class NetworkManager {
private:
    Logger* logger;
    ESP32Time* rtc;
    
    // Network state
    NetworkState currentState;
    String ssid;
    String password;
    bool apModeEnabled;
    unsigned long lastConnectionAttempt;
    int connectionRetries;
    
    // WiFi components
    WebServer* webServer;
    WiFiUDP ntpUDP;
    NTPClient* timeClient;
    
    // Preferences for storing credentials
    Preferences preferences;
    
    // Callbacks
    std::function<void(bool)> connectionCallback;
    std::function<void(String, String)> commandCallback;
    
    // Web server handlers
    void handleRoot();
    void handleSetAlarm();
    void handleGetStatus();
    void handleSetWiFi();
    void handleOTA();
    void handleNotFound();
    
    // Helper methods
    void startWebServer();
    void stopWebServer();
    void startAccessPoint();
    void connectToWiFi();
    void updateTimeFromNTP();
    bool loadWiFiCredentials();
    void saveWiFiCredentials(const String& ssid, const String& password);

public:
    NetworkManager(Logger* log, ESP32Time* rtcInstance);
    ~NetworkManager();
    
    bool begin();
    void update(); // Call frequently in main loop
    
    // WiFi management
    bool connectWiFi(const String& ssid, const String& password);
    void disconnectWiFi();
    void startAPMode(const String& apName = "SmartAlarm-Setup");
    void stopAPMode();
    
    // Status
    NetworkState getState() const { return currentState; }
    bool isConnected() const { return currentState == NETWORK_CONNECTED; }
    String getLocalIP() const;
    int getSignalStrength() const;
    String getNetworkInfo();
    
    // Time synchronization
    void syncTime();
    bool isTimeValid() const;
    
    // OTA updates
    void initializeOTA();
    void handleOTA();
    
    // Web interface
    void enableWebInterface(bool enable);
    bool isWebInterfaceEnabled() const { return webServer != nullptr; }
    
    // Callbacks
    void setConnectionCallback(std::function<void(bool)> callback) { connectionCallback = callback; }
    void setCommandCallback(std::function<void(String, String)> callback) { commandCallback = callback; }
    
    // BLE functionality (stub for future implementation)
    void initializeBLE();
    void updateBLE();
    bool isBLEEnabled() const;
    
    // Utility
    void performNetworkTest();
    void resetNetworkSettings();
};

#endif // NETWORK_MANAGER_H
