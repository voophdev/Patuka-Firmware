#include <Arduino.h>
#include <ESP32Servo.h>

#include "WiFiSetup.h"
#include "FirebaseConfig.h"
#include "UltrasonicSensor.h"
#include "LCDDisplay.h"
#include "RTCConfig.h"
#include "SensorReadings.h"
#include "TimeDisplay.h"
#include "PinConfig.h"
#include "Feeding.h"  // Include the manual feed module
#include "AutomaticFeeding.h"  // Automatic feeding module
#include "PumpControl.h"  // Pump control module
#include "GSMModule.h"
#include "WeighingScale.h"

// Timing variables
unsigned long previousMillisSensor = 0;
unsigned long previousMillisLiveSensor = 0;
unsigned long previousMillisSensorData = 0;
unsigned long previousMillisTime = 0;
unsigned long previousMillisTokenCheck = 0; // Store the last token check time
unsigned long previousMillisUltrasonic = 0;

const unsigned long sensorInterval = 3000;  // 5 seconds for sensors
const unsigned long liveSensorInterval = 3000;  // 10 seconds for live sensor monitoring
const unsigned long sensorDataInterval = 60000; // 1 min interval
const unsigned long timeInterval = 1000;    // 1 second for time updates
const unsigned long tokenRefreshInterval = 600000; // 1 second

// bool initializedGSM = false;

void setup() {
    Serial.begin(115200);

    Wire.begin(21, 22);  // SDA = GPIO 21, SCL = GPIO 22
    setupLCD();

    printCentered("Patuka System", 1);
    printCentered("Starting", 2);

    // Initialize components
    initializeRTC();

    setupUltrasonicSensor(TRIG_PIN_1, ECHO_PIN_1);
    setupUltrasonicSensor(TRIG_PIN_2, ECHO_PIN_2);

    // Initialize the manual feed system (servo + button)
    initializeManualFeed(SERVO_PIN, SERVO_BUTTON_PIN);
    setupPumpControl();
    sensors.begin(); 
    dht.begin();
    delay(3000);  
    lcd.clear();

    printCentered("System Initialized", 1);
    printCentered("Starting Feeding", 2);
    printCentered("System", 3);
    delay(2000);
    lcd.clear();
    
    createTimeDisplayTask();

    connectToWiFi();
    setupWebServer();
    delay(2000);  // Short delay for stability
}

void loop() {
    server.handleClient();
    checkWiFiStatus();
    handleCalibrationCommand();

     vTaskDelay(100 / portTICK_PERIOD_MS);

    unsigned long currentMillis = millis();

    if (currentMillis - previousMillisTokenCheck >= tokenRefreshInterval) {
        previousMillisTokenCheck = currentMillis;
        tokenCheck();
    }

    if (currentMillis - previousMillisTime >= timeInterval) {
        previousMillisTime = currentMillis;
        const DateTime now = rtc.now();
        if (isAutomationEnabled) {
            checkFeedingTimes(now);
        }
    }

    // Sensor update logic
    if (currentMillis - previousMillisSensor >= sensorInterval) {
        previousMillisSensor = currentMillis;
        updateSensorReadings();
    }

    // Live sensor monitoring (every 2 seconds)
    if (currentMillis - previousMillisLiveSensor >= liveSensorInterval) {
        previousMillisLiveSensor = currentMillis;
        liveSensorMonitoring();  // Monitor temperature, pH, etc.
    }

        // Sensor data monitoring (every 60 seconds)
    if (currentMillis - previousMillisSensorData >= sensorDataInterval) {
        previousMillisSensorData = currentMillis;
        sensorDataMonitoring();  // Monitor temperature, pH, etc.
    }
}