#include "UltrasonicSensor.h"
#include <Arduino.h>

unsigned long lastLevelMeasurementTime = 0;
const unsigned long levelMeasurementInterval = 1000; // Measure every 10 seconds

void setupUltrasonicSensor(int trigPin, int echoPin) {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

float measureDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 20000);
  if (duration == 0) {
    Serial.println("No echo received within timeout.");
    return -1;
  }

  return duration * 0.034 / 2;
}

int calculateLevel(float distance, int minDistance, int maxDistance) {
  if (distance == -1) return 0;
  if (distance <= minDistance) return 100;
  if (distance >= maxDistance) return 0;
  return map(distance, minDistance, maxDistance, 100, 0);
}
