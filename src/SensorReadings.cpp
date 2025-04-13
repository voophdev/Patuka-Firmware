#include <Arduino.h>
#include <Adafruit_Sensor.h>

#include "SensorReadings.h"
#include "FirebaseConfig.h"
#include "UltrasonicSensor.h"
#include "LCDDisplay.h"
#include "PinConfig.h"
#include "RTCConfig.h"

// Define the DHT object here (actual definition)
DHT dht(DHT_PIN, DHT22);  // Initialize DHT object
OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);

float voltageAt4pH = 3.30;  // Voltage at pH 4.01
float voltageAt9pH = 2.20;  // Voltage at pH 9.18
float calibrationSlope = (4.01 - 9.18) / (voltageAt4pH - voltageAt9pH);
float calibrationOffset = 4.01 - (calibrationSlope * voltageAt4pH);

// Define variables to store previous levels
int previousWaterLevel = -1;
int previousFeedLevel = -1;

int waterLevel = 0;
int feedLevel = 0;

void updateSensorReadings() {
    // Create a FirebaseJson object to hold the batch data
    FirebaseJson json;

    // Measure water level
    float distance1 = measureDistance(TRIG_PIN_1, ECHO_PIN_1);
    waterLevel = (distance1 != -1) 
        ? calculateLevel(distance1, WATER_MIN_DISTANCE, WATER_MAX_DISTANCE) 
        : 0;

    // Measure feed level
    float distance2 = measureDistance(TRIG_PIN_2, ECHO_PIN_2);
    feedLevel = (distance2 != -1) 
        ? calculateLevel(distance2, FEED_MIN_DISTANCE, FEED_MAX_DISTANCE) 
        : 0;

    // Check if either level has changed
    bool hasChanged = (waterLevel != previousWaterLevel) || (feedLevel != previousFeedLevel);

    if (hasChanged) {
        // Add readings to the FirebaseJson object
        json.set("/waterLevel", waterLevel);
        json.set("/feedLevel", feedLevel);

        // Update Firebase in batch with the original node names
        updateFirebaseLevel(json);

        // Update previous values to current readings
        previousWaterLevel = waterLevel;
        previousFeedLevel = feedLevel;
    }
}

// Define variables to store previous readings
float previousPHValue = -1;
float previousTemperature = -1;
float previousHumidity = -1;
float previousTemperatureDS18B20 = -1;

void liveSensorMonitoring() {

    // Create a FirebaseJson object to hold the batch data
    FirebaseJson json;

    // DHT22 Temperature and Humidity Monitoring
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    bool hasChanged = false; // Flag to track if any value has changed

    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("Failed to read from DHT22 sensor!");
    } else {
        // Check if temperature or humidity has changed
        if (temperature != previousTemperature || humidity != previousHumidity) {
            // Set temperature and humidity directly in the json object
            json.set("/feedTempLive", round(temperature * 10) / 10.0); // Round to one decimal
            json.set("/moistureLive", round(humidity * 10) / 10.0); // Round to one decimal
            Serial.printf("Temperature: %.1f°C | Humidity: %.1f%%\n", temperature, humidity);
            
            // Update previous values
            previousTemperature = temperature;
            previousHumidity = humidity;

            hasChanged = true;
        }
    }

    // DS18B20 Temperature Monitoring
    sensors.requestTemperatures();  // Request temperature readings
    float temperatureDS18B20 = sensors.getTempCByIndex(0);  // Get temperature from DS18B20

    if (temperatureDS18B20 == DEVICE_DISCONNECTED_C) {
        Serial.println("Failed to read from DS18B20 sensor!");
    } else if (temperatureDS18B20 != previousTemperatureDS18B20) {
        json.set("/waterTempLive", round(temperatureDS18B20 * 10) / 10.0); // Round to one decimal
        Serial.printf("DS18B20 Temperature: %.1f°C\n", temperatureDS18B20);
        
        // Update previous value
        previousTemperatureDS18B20 = temperatureDS18B20;

        hasChanged = true;
    }

    // Read raw pH value
    int rawPHValue = analogRead(PH_SENSOR_PIN);  // Read raw value from pH sensor
    float voltage = (rawPHValue / 4095.0) * 3.3;  // Convert to voltage
    float pHValue = (voltage * calibrationSlope) + calibrationOffset;

    if (pHValue != previousPHValue) {
        json.set("/pHLive", round(pHValue * 10) / 10.0); // Round to one decimal
        Serial.printf("pH Value: %.1f\n", pHValue);
        
        // Update previous value
        previousPHValue = pHValue;

        hasChanged = true;
    }

    // Update Firebase only if any of the values have changed
    if (hasChanged) {
        updateFirebaseLiveSensorMonitoring(json);
    }
}

void sensorDataMonitoring() {
    // Get the current date and time
    const DateTime now = rtc.now();

    // Buffers to hold the formatted date and time
    char date[11];  // Format: YYYY-MM-DD (10 characters + null terminator)
    char time[9];   // Format: HH:MM:SS (8 characters + null terminator)

    // Format the date and time with leading zeros
    snprintf(date, sizeof(date), "%04d-%02d-%02d", now.year(), now.month(), now.day());
    snprintf(time, sizeof(time), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());

    // Create a FirebaseJson object to hold the sensor data
    FirebaseJson json;
    json.set("feedTemp", round(dht.readTemperature() * 10) / 10.0); // Round to one decimal
    json.set("moistureContent", round(dht.readHumidity() * 10) / 10.0); // Round to one decimal

    // DS18B20 Temperature Monitoring
    sensors.requestTemperatures();  // Request temperature readings
    float temperatureDS18B20 = sensors.getTempCByIndex(0);  // Get temperature from DS18B20
    json.set("waterTemp", round(temperatureDS18B20 * 10) / 10.0); // Round to one decimal

    // Read raw pH value
    int rawPHValue = analogRead(PH_SENSOR_PIN);  // Read raw value from pH sensor
    float voltage = (rawPHValue / 4095.0) * 3.3;  // Convert to voltage
    float pHValue = (voltage * calibrationSlope) + calibrationOffset;
    json.set("pHLevel", round(pHValue * 10) / 10.0); // Round to one decimal

    // Update Firebase structure under the sensorReading node
    String path = "/sensorReading/" + String(date);  // Use formatted date

    // Update Firebase with the individual reading
    if (Firebase.updateNode(firebaseDataUpdate, path + "/" + String(time), json)) {
        Serial.println("Sensor data updated successfully.");
    } else {
        Serial.printf("Failed to update sensor data: %s\n", firebaseDataUpdate.errorReason().c_str());
    }
}
void displayReadings() {



}