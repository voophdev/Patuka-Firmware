#ifndef WEIGHING_SCALE_H
#define WEIGHING_SCALE_H

#include <HX711.h>
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// Global Variables
extern HX711 scale;          // HX711 scale object
extern float currentWeight;  // Global variable to hold the current weight
extern SemaphoreHandle_t xScaleMutex;  // Mutex for weighing scale
// Global variables
extern int sessionCount; // Local session counter
extern String previousDate;  // Store the previous date to detect changes

// Function prototypes
void setupWeighingScale();
float getWeight();
float calculateFeedDispensed(float weightBefore, float weightAfter);
void weightMonitoringTask(void *parameter);
void calibrateWeighingScale();
void handleCalibrationCommand();
void firebaseUpdateTask(void *parameter);

#endif  // WEIGHING_SCALE_H
