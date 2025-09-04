/**
 * @file NetworkManager.cpp
 * @brief WiFi and BLE connectivity implementation for ESP32 Smart Alarm
 * @author Nighty Byte Team
 * @date 2025-08-25
 */

#include "NetworkManager.h"
#include <ArduinoJson.h>

NetworkManager::NetworkManager(Logger* log, ESP32Time* rtcInstance) {
    logger = log;
    rtc = rtcInstance;
    
    currentState = NETWORK_IDLE;
    apModeEnabled = false;
    lastConnectionAttempt = 0;
    connectionRetries = 0;
    
    webServer = nullptr;
    timeClient = nullptr;
}

NetworkManager::~NetworkManager() {
    if (webServer) {
        delete webServer;
    }
    if (timeClient) {
        delete timeClient;
    }
    preferences.end();
}

bool NetworkManager::begin() {
    // Initialize preferences
    if (!preferences.begin("network", false)) {
        if (logger) logger->logError(EVENT_SYSTEM_START, "Failed to initialize network preferences");
        return false;
    }
    
    // Load saved WiFi credentials
    if (loadWiFiCredentials()) {
        if (logger) {
            logger->logInfo(EVENT_SYSTEM_START, "Loaded WiFi credentials", "SSID: " + ssid);
        }
        
        // Try to connect to saved network
        connectToWiFi();
    } else {
        if (logger) {
            logger->logInfo(EVENT_SYSTEM_START, "No saved WiFi credentials, starting AP mode");
        }
        
        // Start in AP mode for initial setup
        startAccessPoint();
    }
    
    // Initialize NTP client
    timeClient = new NTPClient(ntpUDP, NTP_SERVER, TIMEZONE_OFFSET_HOURS * 3600, NTP_UPDATE_INTERVAL_MS);
    
    // Initialize OTA
    initializeOTA();
    
    if (logger) {
        logger->logInfo(EVENT_SYSTEM_START, "NetworkManager initialized");
    }
    
    return true;
}

void NetworkManager::update() {
    unsigned long currentTime = millis();
    
    switch (currentState) {
        case NETWORK_CONNECTING:
            if (WiFi.status() == WL_CONNECTED) {
                currentState = NETWORK_CONNECTED;
                if (logger) {
                    logger->logInfo(EVENT_WIFI_CONNECTED, "WiFi connected", 
                                   "IP: " + WiFi.localIP().toString() + 
                                   ", RSSI: " + String(WiFi.RSSI()) + "dBm");
                }
                
                // Start web server
                startWebServer();
                
                // Sync time
                syncTime();
                
                // Trigger callback
                if (connectionCallback) {
                    connectionCallback(true);
                }
            } else if (currentTime - lastConnectionAttempt > WIFI_CONNECT_TIMEOUT_MS) {
                // Connection timeout
                connectionRetries++;
                if (connectionRetries < 3) {
                    if (logger) {
                        logger->logWarning(EVENT_WIFI_DISCONNECTED, "WiFi connection timeout, retrying",
                                         "Attempt: " + String(connectionRetries));
                    }
                    connectToWiFi();
                } else {
                    // Give up and start AP mode
                    if (logger) {
                        logger->logError(EVENT_WIFI_DISCONNECTED, "WiFi connection failed, starting AP mode");
                    }
                    startAccessPoint();
                }
            }
            break;
            
        case NETWORK_CONNECTED:
            if (WiFi.status() != WL_CONNECTED) {
                currentState = NETWORK_ERROR;
                if (logger) {
                    logger->logWarning(EVENT_WIFI_DISCONNECTED, "WiFi disconnected unexpectedly");
                }
                
                // Stop web server
                stopWebServer();
                
                // Try to reconnect
                connectToWiFi();
                
                // Trigger callback
                if (connectionCallback) {
                    connectionCallback(false);
                }
            } else {
                // Handle web server
                if (webServer) {
                    webServer->handleClient();
                }
                
                // Handle OTA
                ArduinoOTA.handle();
                
                // Update NTP periodically
                if (timeClient && currentTime % 60000 == 0) { // Every minute
                    timeClient->update();
                    if (rtc && timeClient->isTimeSet()) {
                        rtc->setTime(timeClient->getEpochTime());
                    }
                }
            }
            break;
            
        case NETWORK_AP_MODE:
            // Handle web server in AP mode
            if (webServer) {
                webServer->handleClient();
            }
            break;
            
        case NETWORK_ERROR:
            // Try to recover after some time
            if (currentTime - lastConnectionAttempt > 30000) { // Wait 30 seconds
                if (logger) {
                    logger->logInfo(EVENT_WIFI_CONNECTED, "Attempting to recover from network error");
                }
                connectToWiFi();
            }
            break;
            
        default:
            break;
    }
}

bool NetworkManager::connectWiFi(const String& newSsid, const String& newPassword) {
    ssid = newSsid;
    password = newPassword;
    
    // Save credentials
    saveWiFiCredentials(ssid, password);
    
    connectToWiFi();
    return true;
}

void NetworkManager::connectToWiFi() {
    if (ssid.isEmpty()) {
        return;
    }
    
    // Stop AP mode if running
    if (apModeEnabled) {
        WiFi.softAPdisconnect();
        apModeEnabled = false;
        stopWebServer();
    }
    
    currentState = NETWORK_CONNECTING;
    lastConnectionAttempt = millis();
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    
    if (logger) {
        logger->logInfo(EVENT_WIFI_CONNECTED, "Connecting to WiFi", "SSID: " + ssid);
    }
}

void NetworkManager::disconnectWiFi() {
    if (currentState == NETWORK_CONNECTED || currentState == NETWORK_CONNECTING) {
        WiFi.disconnect();
        currentState = NETWORK_IDLE;
        stopWebServer();
        
        if (logger) {
            logger->logInfo(EVENT_WIFI_DISCONNECTED, "WiFi disconnected by user");
        }
    }
}

void NetworkManager::startAccessPoint() {
    String apName = "SmartAlarm-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apName.c_str(), "smartalarm2025");
    
    currentState = NETWORK_AP_MODE;
    apModeEnabled = true;
    
    // Start web server for configuration
    startWebServer();
    
    if (logger) {
        logger->logInfo(EVENT_SYSTEM_START, "Access Point started", 
                       "SSID: " + apName + ", IP: " + WiFi.softAPIP().toString());
    }
}

void NetworkManager::stopAPMode() {
    if (apModeEnabled) {
        WiFi.softAPdisconnect();
        apModeEnabled = false;
        stopWebServer();
        currentState = NETWORK_IDLE;
        
        if (logger) {
            logger->logInfo(EVENT_SYSTEM_START, "Access Point stopped");
        }
    }
}

void NetworkManager::startWebServer() {
    if (webServer) {
        delete webServer;
    }
    
    webServer = new WebServer(HTTP_PORT);
    
    // Set up routes
    webServer->on("/", [this]() { handleRoot(); });
    webServer->on("/setalarm", HTTP_POST, [this]() { handleSetAlarm(); });
    webServer->on("/status", HTTP_GET, [this]() { handleGetStatus(); });
    webServer->on("/setwifi", HTTP_POST, [this]() { handleSetWiFi(); });
    webServer->on("/ota", HTTP_GET, [this]() { handleOTA(); });
    webServer->onNotFound([this]() { handleNotFound(); });
    
    webServer->begin();
    
    if (logger) {
        logger->logInfo(EVENT_SYSTEM_START, "Web server started", "Port: " + String(HTTP_PORT));
    }
}

void NetworkManager::stopWebServer() {
    if (webServer) {
        webServer->stop();
        delete webServer;
        webServer = nullptr;
        
        if (logger) {
            logger->logInfo(EVENT_SYSTEM_START, "Web server stopped");
        }
    }
}

void NetworkManager::syncTime() {
    if (!timeClient) {
        return;
    }
    
    timeClient->begin();
    if (timeClient->update()) {
        if (rtc) {
            rtc->setTime(timeClient->getEpochTime());
        }
        
        if (logger) {
            logger->logInfo(EVENT_SYSTEM_START, "Time synchronized", 
                           "Epoch: " + String(timeClient->getEpochTime()));
        }
    } else {
        if (logger) {
            logger->logWarning(EVENT_SYSTEM_START, "Failed to sync time from NTP");
        }
    }
}

void NetworkManager::initializeOTA() {
    ArduinoOTA.setPort(OTA_PORT);
    ArduinoOTA.setPassword(OTA_PASSWORD);
    ArduinoOTA.setHostname("smartalarm");
    
    ArduinoOTA.onStart([this]() {
        if (logger) {
            logger->logInfo(EVENT_OTA_START, "OTA update started");
        }
    });
    
    ArduinoOTA.onEnd([this]() {
        if (logger) {
            logger->logInfo(EVENT_OTA_SUCCESS, "OTA update completed");
        }
    });
    
    ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) {
        static unsigned int lastPercent = 0;
        unsigned int percent = (progress / (total / 100));
        if (percent != lastPercent && percent % 10 == 0) {
            if (logger) {
                logger->logDebug(EVENT_OTA_START, "OTA progress", String(percent) + "%");
            }
            lastPercent = percent;
        }
    });
    
    ArduinoOTA.onError([this](ota_error_t error) {
        String errorMsg = "Unknown error";
        switch (error) {
            case OTA_AUTH_ERROR: errorMsg = "Auth Failed"; break;
            case OTA_BEGIN_ERROR: errorMsg = "Begin Failed"; break;
            case OTA_CONNECT_ERROR: errorMsg = "Connect Failed"; break;
            case OTA_RECEIVE_ERROR: errorMsg = "Receive Failed"; break;
            case OTA_END_ERROR: errorMsg = "End Failed"; break;
        }
        if (logger) {
            logger->logError(EVENT_OTA_FAILED, "OTA update failed", errorMsg);
        }
    });
    
    ArduinoOTA.begin();
}

// Web server handlers
void NetworkManager::handleRoot() {
    String html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Smart Alarm Configuration</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .container { max-width: 600px; margin: 0 auto; }
        .form-group { margin-bottom: 15px; }
        label { display: block; margin-bottom: 5px; }
        input, select { width: 100%; padding: 8px; margin-bottom: 10px; }
        button { background: #007cba; color: white; padding: 10px 20px; border: none; cursor: pointer; }
        button:hover { background: #005a87; }
        .status { background: #f0f0f0; padding: 15px; margin: 10px 0; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Smart Alarm System</h1>
        
        <div class="status">
            <h3>System Status</h3>
            <p>WiFi: <span id="wifi-status">Loading...</span></p>
            <p>Time: <span id="current-time">Loading...</span></p>
            <p>Alarms: <span id="alarm-count">Loading...</span></p>
        </div>
        
        <h2>WiFi Configuration</h2>
        <form action="/setwifi" method="post">
            <div class="form-group">
                <label>SSID:</label>
                <input type="text" name="ssid" required>
            </div>
            <div class="form-group">
                <label>Password:</label>
                <input type="password" name="password">
            </div>
            <button type="submit">Connect WiFi</button>
        </form>
        
        <h2>Add Alarm</h2>
        <form action="/setalarm" method="post">
            <div class="form-group">
                <label>Time:</label>
                <input type="time" name="time" required>
            </div>
            <div class="form-group">
                <label>Days:</label>
                <select name="days">
                    <option value="daily">Daily</option>
                    <option value="weekdays">Weekdays</option>
                    <option value="weekends">Weekends</option>
                </select>
            </div>
            <div class="form-group">
                <label>Label:</label>
                <input type="text" name="label" placeholder="Optional alarm label">
            </div>
            <button type="submit">Add Alarm</button>
        </form>
    </div>
    
    <script>
        function updateStatus() {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('wifi-status').textContent = data.wifi;
                    document.getElementById('current-time').textContent = data.time;
                    document.getElementById('alarm-count').textContent = data.alarms;
                })
                .catch(error => console.error('Error:', error));
        }
        
        // Update status every 5 seconds
        updateStatus();
        setInterval(updateStatus, 5000);
    </script>
</body>
</html>
    )";
    
    webServer->send(200, "text/html", html);
}

void NetworkManager::handleSetAlarm() {
    String timeStr = webServer->arg("time");
    String days = webServer->arg("days");
    String label = webServer->arg("label");
    
    // Parse time (HH:MM format)
    int colonIndex = timeStr.indexOf(':');
    if (colonIndex > 0) {
        int hour = timeStr.substring(0, colonIndex).toInt();
        int minute = timeStr.substring(colonIndex + 1).toInt();
        
        if (commandCallback) {
            String command = "SETALARM:" + String(hour) + ":" + String(minute) + ":" + days + ":" + label;
            commandCallback("SETALARM", command);
        }
        
        webServer->send(200, "text/plain", "Alarm set successfully");
    } else {
        webServer->send(400, "text/plain", "Invalid time format");
    }
}

void NetworkManager::handleGetStatus() {
    DynamicJsonDocument doc(512);
    
    doc["wifi"] = (currentState == NETWORK_CONNECTED) ? ("Connected (" + WiFi.localIP().toString() + ")") : 
                  (currentState == NETWORK_AP_MODE) ? "AP Mode" : "Disconnected";
    doc["time"] = rtc ? rtc->getTime("%Y-%m-%d %H:%M:%S") : "Not set";
    doc["alarms"] = "0"; // TODO: Get actual alarm count
    doc["uptime"] = millis() / 1000;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    webServer->send(200, "application/json", jsonString);
}

void NetworkManager::handleSetWiFi() {
    String newSsid = webServer->arg("ssid");
    String newPassword = webServer->arg("password");
    
    if (!newSsid.isEmpty()) {
        connectWiFi(newSsid, newPassword);
        webServer->send(200, "text/plain", "WiFi credentials updated. Connecting...");
    } else {
        webServer->send(400, "text/plain", "SSID cannot be empty");
    }
}

void NetworkManager::handleOTA() {
    String html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>OTA Update</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
    <h1>OTA Update Ready</h1>
    <p>Use Arduino IDE or PlatformIO to upload firmware via OTA.</p>
    <p>Device IP: )" + getLocalIP() + R"(</p>
    <p>OTA Port: )" + String(OTA_PORT) + R"(</p>
    <p>Password: [Protected]</p>
</body>
</html>
    )";
    
    webServer->send(200, "text/html", html);
}

void NetworkManager::handleNotFound() {
    webServer->send(404, "text/plain", "Not Found");
}

// Helper methods
bool NetworkManager::loadWiFiCredentials() {
    ssid = preferences.getString("ssid", "");
    password = preferences.getString("password", "");
    
    return !ssid.isEmpty();
}

void NetworkManager::saveWiFiCredentials(const String& newSsid, const String& newPassword) {
    preferences.putString("ssid", newSsid);
    preferences.putString("password", newPassword);
}

String NetworkManager::getLocalIP() const {
    if (currentState == NETWORK_CONNECTED) {
        return WiFi.localIP().toString();
    } else if (currentState == NETWORK_AP_MODE) {
        return WiFi.softAPIP().toString();
    }
    return "Not connected";
}

int NetworkManager::getSignalStrength() const {
    if (currentState == NETWORK_CONNECTED) {
        return WiFi.RSSI();
    }
    return 0;
}

String NetworkManager::getNetworkInfo() {
    String info = "Network Status:\n";
    info += "State: ";
    
    switch (currentState) {
        case NETWORK_IDLE: info += "Idle"; break;
        case NETWORK_CONNECTING: info += "Connecting"; break;
        case NETWORK_CONNECTED: info += "Connected"; break;
        case NETWORK_AP_MODE: info += "AP Mode"; break;
        case NETWORK_ERROR: info += "Error"; break;
    }
    
    info += "\n";
    
    if (currentState == NETWORK_CONNECTED) {
        info += "SSID: " + WiFi.SSID() + "\n";
        info += "IP: " + WiFi.localIP().toString() + "\n";
        info += "RSSI: " + String(WiFi.RSSI()) + " dBm\n";
    } else if (currentState == NETWORK_AP_MODE) {
        info += "AP IP: " + WiFi.softAPIP().toString() + "\n";
        info += "Stations: " + String(WiFi.softAPgetStationNum()) + "\n";
    }
    
    return info;
}

bool NetworkManager::isTimeValid() const {
    return timeClient && timeClient->isTimeSet();
}

void NetworkManager::enableWebInterface(bool enable) {
    if (enable && !webServer) {
        startWebServer();
    } else if (!enable && webServer) {
        stopWebServer();
    }
}

// BLE stubs (for future implementation)
void NetworkManager::initializeBLE() {
    // TODO: Implement BLE initialization
    if (logger) {
        logger->logInfo(EVENT_SYSTEM_START, "BLE initialization - TODO: Not implemented yet");
    }
}

void NetworkManager::updateBLE() {
    // TODO: Implement BLE update logic
}

bool NetworkManager::isBLEEnabled() const {
    // TODO: Return actual BLE status
    return false;
}

void NetworkManager::performNetworkTest() {
    if (logger) {
        logger->logInfo(EVENT_SYSTEM_START, "Starting network test");
    }
    
    // Test WiFi connectivity
    if (currentState == NETWORK_CONNECTED) {
        // Try to ping a reliable server
        // TODO: Implement ping test
        if (logger) {
            logger->logInfo(EVENT_SYSTEM_START, "WiFi connectivity test", "Connected to " + WiFi.SSID());
        }
    }
    
    // Test time sync
    if (timeClient) {
        if (timeClient->update()) {
            if (logger) {
                logger->logInfo(EVENT_SYSTEM_START, "NTP time sync test", "Success");
            }
        } else {
            if (logger) {
                logger->logWarning(EVENT_SYSTEM_START, "NTP time sync test", "Failed");
            }
        }
    }
    
    if (logger) {
        logger->logInfo(EVENT_SYSTEM_START, "Network test completed");
    }
}

void NetworkManager::resetNetworkSettings() {
    preferences.clear();
    ssid = "";
    password = "";
    
    if (logger) {
        logger->logInfo(EVENT_SYSTEM_START, "Network settings reset");
    }
}
