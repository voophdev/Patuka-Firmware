#ifndef SENSOR_READINGS_H
#define SENSOR_READINGS_H

#include <DHT.h> 
#include <OneWire.h>
#include <DallasTemperature.h>

extern DHT dht;
extern OneWire oneWire;
extern DallasTemperature sensors;
extern int waterLevel;
extern int feedLevel;

void updateSensorReadings();  // Function declaration
void liveSensorMonitoring(); // Live sensor monitoring with firebase updates
void sensorDataMonitoring(); // Sensor data monitoring without firebase updates


#endif  // SENSOR_READINGS_H
