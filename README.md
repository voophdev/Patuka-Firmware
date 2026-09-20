# Patuka

Patuka is an automatic chicken feeder for small poultry farms. The ESP32
firmware dispenses feed on a schedule or on demand, monitors feed and water
levels, measures environmental conditions and feed weight, controls a water
pump, sends SMS notifications, and synchronizes farm data with Firebase.

## Companion Mobile App

The mobile application for this project is available in the
[voophdev/capstone](https://github.com/voophdev/capstone) repository.

## Development

This project uses PlatformIO with the Arduino framework and targets the
`esp32dev` board.

Install PlatformIO, configure the required environment variables, then build or
upload the firmware:

```powershell
pio run
pio run --target upload
```

## Architecture

Patuka is organized as an ESP32 Arduino firmware project. The main application
loop handles networking, Firebase authentication, scheduled feeding, sensor
uploads, and the web configuration server. Independent FreeRTOS tasks handle
the time display, weight monitoring, Firebase streams, feeding, pump control,
and SMS notifications.

```text
ESP32
|-- Wi-Fi + Firebase Realtime Database
|-- Web server + fallback access point for Wi-Fi setup
|-- Sensors: levels, temperature, humidity, pH, and feed weight
|-- Actuators: feed servo and water pump relay
|-- LCD + RTC for local status and scheduling
`-- GSM modem for SMS notifications
```

Main source modules:

- `main.cpp`: startup sequence and periodic work.
- `WifiSetup.*` and `FirebaseConfig.*`: Wi-Fi, access point, web setup, and
	Firebase initialization.
- `FirebaseStream.*`: remote manual feeding, pump requests, and automation
	state.
- `Feeding.*`, `AutomaticFeeding.*`, and `PumpControl.*`: manual/automatic
	feeding and pump control.
- `SensorReadings.*`, `UltrasonicSensor.*`, and `WeighingScale.*`: sensor
	acquisition and Firebase readings.
- `LCDDisplay.*` and `TimeDisplay.*`: local display tasks.
- `RTCConfig.*`: DS3231 clock and schedule time.
- `GSMModule.*` and `SMSTask.*`: SIM900-compatible modem and SMS work.

## Hardware Requirements

The following bill of materials matches the interfaces in `src/` and is sized
for a small poultry-farm prototype:

| Device | Quantity | Recommended specification |
| --- | ---: | --- |
| ESP32 development board | 1 | ESP32 DevKit V1 / `esp32dev`, 240 MHz dual-core, 4 MB flash, Wi-Fi, 3.3 V GPIO. |
| GSM modem | 1 | SIM900A development board with antenna, SIM socket, and 5 V-compatible supply; UART2 uses GPIO 16/17. |
| Water level sensor | 1 | JSN-SR04T waterproof ultrasonic sensor, 5 V, 20-450 cm range. Use a voltage divider on the echo line if the module outputs 5 V. |
| Feed level sensor | 1 | HC-SR04P ultrasonic sensor, 3.3-5 V compatible, minimum 2 cm range. |
| Temperature/humidity sensor | 1 | DHT22 / AM2302, 3.3 V logic, with a 4.7-10 kOhm pull-up resistor. |
| Water temperature sensor | 1 | Waterproof DS18B20 probe, 3.0-5.5 V, with a 4.7 kOhm data pull-up resistor. |
| pH sensor module | 1 | PH-4502C analog pH interface with a BNC pH probe; calibrate its output for the ESP32 ADC range. |
| Feed scale interface | 1 | HX711 24-bit load-cell amplifier module. |
| Load cell | 1 | 5 kg single-point load cell with a rigid platform and mounting hardware. |
| Status display | 1 | 20x4 LCD with PCF8574 I2C backpack at address `0x27`. |
| Real-time clock | 1 | DS3231 RTC module with backup coin cell. |
| Feed servo | 1 | MG996R metal-gear servo, 4.8-6.6 V, at least 9 kg-cm torque. |
| Pump relay | 1 | 1-channel 5 V opto-isolated relay module, active-low input. |
| Water pump | 1 | 12 V DC diaphragm or food-safe peristaltic pump, approximately 3-5 L/min. |
| Push buttons | 2 | Normally-open momentary panel buttons for pump and manual feed control. |

Power and installation parts:

- 12 V DC, 5 A regulated power supply for the pump and GSM supply path.
- 5 V, 3 A buck converter for the ESP32, servo, relay, LCD, and sensors.
- Common ground between the ESP32 and low-voltage modules; keep the pump and
	servo current paths separate from the ESP32 supply wiring.
- 5 V-to-3.3 V resistor dividers for any ultrasonic or modem signal that can
	exceed the ESP32 GPIO limit.
- Waterproof enclosure, feed hopper, dispensing gate, water container,
	food-safe tubing, load-cell platform, terminal blocks, and shielded cables.

Do not power the servo, pump, or SIM900A directly from an ESP32 3.3 V pin.
The SIM900A and servo need a stable supply capable of handling current peaks;
the pump must be switched through the relay and powered from its own supply.

### GPIO Map

The authoritative pin definitions are in [`src/PinConfig.h`](src/PinConfig.h).

| Function | GPIO |
| --- | ---: |
| Water ultrasonic trigger / echo | 5 / 18 |
| Feed ultrasonic trigger / echo | 4 / 19 |
| DHT22 data | 25 |
| DS18B20 data | 26 |
| Analog pH input | 32 |
| Pump button / feed button | 33 / 14 |
| Pump relay / feed servo | 23 / 15 |
| HX711 DOUT / SCK | 13 / 12 |
| I2C SDA / SCL | 21 / 22 |
| GSM UART2 RX / TX | 16 / 17 |

## How-To Guides

### Configure Credentials

1. Copy `.env.example` to `.env` as a private reference.
2. Set the eight `PATUKA_*` variables in the terminal or CI environment used
	 for the build. The pre-build script reads host environment variables; it
	 does not automatically load `.env`.
3. Build with `pio run`.

### First Boot

1. Connect the hardware and open the serial monitor at `115200` baud.
2. Upload the firmware and watch for initialization errors. The firmware checks
	 the RTC, scale, Wi-Fi, and Firebase connection during startup.
3. If Wi-Fi does not connect, join the configured fallback access point and use
	 the web configuration page to save Wi-Fi credentials.
4. Confirm that the LCD displays the time, date, and connection state.

### Calibrate the Load Cell

The current calibration factor and tare value are in
[`src/WeighingScale.cpp`](src/WeighingScale.cpp).

1. Upload the firmware and ensure the load cell is unloaded.
2. Use the serial calibration workflow implemented by
	 `handleCalibrationCommand()`.
3. Place a known mass on the load cell and adjust the calibration factor until
	 the reported weight is correct.
4. Update the resulting calibration values in the source before deployment.

### Verify Feeding and Pump Control

- Press the manual feed button to exercise the servo and record a feed session.
- Test the pump button while monitoring the active-low relay behavior.
- Enable automation in Firebase and verify the configured schedule in
	`AutomaticFeeding.cpp`.
- Test SMS delivery with a valid SIM900A-compatible modem, SIM card, antenna,
	power supply, and configured destination number.

## Configuration

Credentials are injected at build time from host environment variables. They are
not stored in the source tree.

Use [.env.example](.env.example) as the variable reference, then set these
variables in the shell or CI environment before building:

```powershell
$env:PATUKA_FIREBASE_DATABASE_URL = "https://..."
$env:PATUKA_FIREBASE_API_KEY = "..."
$env:PATUKA_FIREBASE_USER_EMAIL = "..."
$env:PATUKA_FIREBASE_USER_PASSWORD = "..."
$env:PATUKA_WIFI_SSID = "..."
$env:PATUKA_WIFI_PASSWORD = "..."
$env:PATUKA_AP_SSID = "Patuka-Setup"
$env:PATUKA_AP_PASSWORD = "..."
```

Then run the PlatformIO build or upload command in the same terminal. CI can
provide the same values as protected environment secrets.
