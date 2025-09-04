# ESP32 Night Routine Smart Alarm System

A comprehensive smart alarm system built for ESP32 DevKit V1 that combines alarm scheduling, environmental sensing, and remote connectivity features.

## ✨ Features

### Core Functionality
- ✅ **Alarm Management**: Multiple alarms with flexible scheduling (daily, weekdays, weekends, custom days)
- ✅ **Smart Pill Box Integration**: Contact switch detection to dismiss alarms when pill box is opened
- ✅ **3.3V Buzzer Control**: PWM-controlled buzzer with multiple patterns (alarm, notification, success, error)
- ✅ **Light Sensor**: Automatic bedtime reminders based on ambient light levels
- ✅ **USB Charging Detection**: Monitor phone charging state
- ✅ **Comprehensive Logging**: Event logging to flash memory with multiple log levels

### Connectivity & Remote Features
- ✅ **WiFi Connectivity**: Auto-connect to saved networks or AP mode for setup
- ✅ **Web Interface**: Browser-based configuration and control
- ✅ **NTP Time Sync**: Automatic time synchronization
- ✅ **OTA Updates**: Over-the-air firmware updates
- 🔄 **BLE Support**: Planned for future implementation

### Advanced Features
- ✅ **Non-blocking Architecture**: All operations use non-blocking code
- ✅ **Modular Design**: Separate classes for each major component
- ✅ **Persistent Storage**: Settings and alarms saved to flash
- ✅ **Serial Commands**: Debug interface for testing and diagnostics

## 🔧 Hardware Setup

### Required Components
- **ESP32 DevKit V1** (main microcontroller)
- **3.3V Buzzer** (for alarm sounds)
- **Contact Switch** (for pill box detection)
- **LDR (Light Dependent Resistor)** with 10kΩ resistor for voltage divider
- **Voltage divider circuit** for USB detection (2x 10kΩ resistors)
- **Breadboard and jumper wires**

### Pin Connections

```
ESP32 DevKit V1 Pin Assignments:
┌─────────────────────────────────────────┐
│ Component          │ GPIO Pin │ Type    │
├─────────────────────────────────────────┤
│ Buzzer             │ GPIO2    │ PWM Out │
│ Pill Box Switch    │ GPIO4    │ Digital │
│ Light Sensor (LDR) │ GPIO36   │ ADC     │
│ USB Detection      │ GPIO39   │ ADC     │
│ Status LED         │ GPIO2    │ Shared  │
└─────────────────────────────────────────┘
```

### Wiring Diagram

```
Light Sensor (LDR):
3.3V ──┬── LDR ──┬── GPIO36 (ADC1_CH0)
       │         │
       │         └── 10kΩ ── GND
       
USB Detection:
5V (USB) ──┬── 10kΩ ──┬── GPIO39 (ADC1_CH3)
           │          │
           │          └── 10kΩ ── GND

Pill Box Contact Switch:
GPIO4 ──── Switch ──── GND
(Internal pullup enabled)

Buzzer:
GPIO2 ──── Buzzer(+) ──── Buzzer(-) ──── GND
```

## 🚀 Quick Start

### 1. Platform Setup
```bash
# Install PlatformIO
pip install platformio

# Or use VS Code with PlatformIO extension
```

### 2. Clone and Build
```bash
# Navigate to project directory
cd nighty_byte

# Build the project
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor
```

### 3. Initial Configuration
1. **Power on the ESP32** - it will create a WiFi hotspot named `SmartAlarm-XXXXXX`
2. **Connect to the hotspot** using password `smartalarm2025`
3. **Open browser** and navigate to `192.168.4.1`
4. **Configure WiFi** by entering your network credentials
5. **Set up alarms** through the web interface

### 4. Web Interface
Once connected to WiFi, access the web interface at the device's IP address:
- **Set alarms** with custom times and schedules
- **Monitor system status** in real-time
- **Configure WiFi settings**
- **Access OTA update interface**

## 💻 Serial Commands

Connect via serial monitor (115200 baud) for debugging:

```
STATUS        - Show complete system status
TEST_BUZZER   - Test buzzer with different patterns
TEST_SENSORS  - Test all sensor readings
TEST_NETWORK  - Test network connectivity
ADD_ALARM     - Add a test alarm (1 minute from now)
CLEAR_ALARMS  - Remove all configured alarms
BEEP          - Play a test beep sound
LOGS          - Show logging summary
HELP          - Display available commands
```

## 📁 Project Structure

```
nighty_byte/
├── platformio.ini          # PlatformIO configuration
├── include/                 # Header files
│   ├── config.h            # Hardware pins and system configuration
│   ├── Logger.h            # Event logging system
│   ├── AlarmManager.h      # Alarm scheduling and management
│   ├── SensorManager.h     # Sensor reading and processing
│   ├── BuzzerController.h  # PWM buzzer control
│   └── NetworkManager.h    # WiFi, web server, and OTA
├── src/                    # Implementation files
│   ├── main.cpp            # Main application logic
│   ├── Logger.cpp          # Logging implementation
│   ├── AlarmManager.cpp    # Alarm management logic
│   ├── SensorManager.cpp   # Sensor processing
│   ├── BuzzerController.cpp # Buzzer control patterns
│   └── NetworkManager.cpp  # Network and web functionality
├── lib/                    # Custom libraries (empty)
└── README.md              # This file
```

## ⚙️ Configuration

### Key Configuration Options (config.h)
```cpp
// Hardware pins
#define BUZZER_PIN 2
#define PILL_BOX_SWITCH_PIN 4
#define LIGHT_SENSOR_PIN 36
#define USB_DETECT_PIN 39

// Timing
#define ALARM_BUZZER_DURATION_MS 300000  // 5 minutes max
#define ALARM_SNOOZE_DURATION_MS 540000  // 9 minutes snooze
#define BEDTIME_REMINDER_HOUR 22         // 10 PM bedtime reminder

// Thresholds
#define BEDTIME_LIGHT_THRESHOLD 500      // ADC value for "dark"
#define USB_VOLTAGE_THRESHOLD 2048       // ADC value for USB detection
```

## 🔍 System Architecture

### Component Overview
```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│  AlarmManager   │    │  SensorManager  │    │ BuzzerController│
│                 │    │                 │    │                 │
│ • Scheduling    │◄──►│ • Light Sensor  │◄──►│ • PWM Control   │
│ • Pill Box Int. │    │ • USB Detection │    │ • Patterns      │
│ • Persistence   │    │ • Contact Switch│    │ • Volume        │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         ▲                       ▲                       ▲
         │                       │                       │
         ▼                       ▼                       ▼
┌─────────────────────────────────────────────────────────────────┐
│                        Main Application                         │
│                                                                │
│ • System coordination    • Event handling    • Serial commands │
└─────────────────────────────────────────────────────────────────┘
         ▲                       ▲                       ▲
         │                       │                       │
         ▼                       ▼                       ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ NetworkManager  │    │     Logger      │    │   ESP32Time     │
│                 │    │                 │    │                 │
│ • WiFi/AP Mode  │    │ • Event Logging │    │ • RTC Functions │
│ • Web Server    │    │ • Flash Storage │    │ • NTP Sync      │
│ • OTA Updates   │    │ • Serial Output │    │ • Time Keeping  │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

### Data Flow
1. **Sensors** continuously monitor environment and pill box
2. **AlarmManager** checks time and triggers alarms
3. **BuzzerController** plays appropriate patterns
4. **NetworkManager** handles remote commands and time sync
5. **Logger** records all significant events
6. **Main loop** coordinates all components non-blocking

## 🎯 Usage Examples

### Setting Up Alarms

**Via Web Interface:**
1. Open browser to device IP
2. Fill alarm form with time and days
3. Click "Add Alarm"

**Via Serial Commands:**
```
ADD_ALARM     # Adds test alarm 1 minute from now
```

### Monitoring System
```
STATUS        # Shows complete system status including:
              # - Memory usage and uptime
              # - Network connection details
              # - Sensor readings
              # - Active alarms
              # - Current time
```

### Testing Components
```
TEST_BUZZER   # Plays tones at different frequencies
TEST_SENSORS  # Shows raw sensor readings
TEST_NETWORK  # Tests WiFi and NTP connectivity
```

## 🐛 Troubleshooting

### Common Issues

**1. WiFi Connection Failed**
- Check SSID and password
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)
- Device will automatically start AP mode after 3 failed attempts

**2. Alarm Not Triggering**
- Verify time is synchronized (check STATUS command)
- Confirm alarm is enabled and scheduled correctly
- Check that system time matches expected timezone

**3. Sensors Not Reading**
- Verify wiring connections
- Use TEST_SENSORS command to check raw values
- Check voltage dividers are correct

**4. Buzzer Not Working**
- Verify 3.3V buzzer (not 5V)
- Check GPIO2 connection
- Use TEST_BUZZER command

**5. Web Interface Not Accessible**
- Check device IP with STATUS command
- Ensure device and computer on same network
- Try AP mode if WiFi connection failed

### Debug Information
- **Serial Monitor**: Set to 115200 baud for debug output
- **Log Levels**: DEBUG, INFO, WARNING, ERROR
- **Memory Monitoring**: Free heap displayed in status
- **Component Status**: Each component reports initialization

## 🔮 Future Enhancements

### Planned Features
- [ ] **BLE Connectivity**: Bluetooth Low Energy for mobile app
- [ ] **Presence Detection**: BLE/WiFi device proximity sensing  
- [ ] **Sleep Quality Tracking**: Motion sensors during night
- [ ] **Weather Integration**: API calls for weather-based alarms
- [ ] **Voice Commands**: Basic speech recognition
- [ ] **Mobile App**: Dedicated smartphone application
- [ ] **Multiple Pill Boxes**: Support for different medications
- [ ] **Geofencing**: Location-based alarm modifications

### Hardware Expansions
- [ ] **E-paper Display**: Status and alarm information
- [ ] **PIR Motion Sensor**: Room occupancy detection
- [ ] **Temperature/Humidity**: Environmental monitoring
- [ ] **SD Card**: Extended log storage
- [ ] **Battery Backup**: UPS functionality
- [ ] **External RTC**: More accurate timekeeping

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🤝 Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

---

**Built with ❤️ for better night routines and medication adherence**
