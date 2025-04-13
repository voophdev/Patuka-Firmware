#ifndef PUMP_CONTROL_H
#define PUMP_CONTROL_H

#include <Arduino.h>

// Setup function to initialize pump control
void setupPumpControl();        

// FreeRTOS task function for controlling the pump, including button debounce and Firebase integration
void pumpControlTask(void *parameter);

// Function to automatically trigger the pump for a specific time (10 seconds)
void automaticTrigger();

// Function to set the pump request flag from Firebase
void setWaterPumpRequested(bool request);

#endif  // PUMP_CONTROL_H
