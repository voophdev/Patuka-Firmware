#ifndef FEEDING_H
#define FEEDING_H

#include <Arduino.h>

// Global flag to track manual feed button press
extern volatile bool isFeedButtonPressed;

// Function prototypes
void initializeManualFeed(int servoPin, int buttonPin);
void activateServo();
void IRAM_ATTR handleFeedButtonPress();  // Interrupt handler for button
void manualFeedTask(void *parameter);  // FreeRTOS task for manual feed
void createManualFeedTask();


#endif  // FEEDING_H
