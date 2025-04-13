#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

extern unsigned long lastLevelMeasurementTime;
extern const unsigned long levelMeasurementInterval;

void setupUltrasonicSensor(int trigPin, int echoPin);
float measureDistance(int trigPin, int echoPin);
int calculateLevel(float distance, int minDistance, int maxDistance);

#endif // ULTRASONIC_SENSOR_H
