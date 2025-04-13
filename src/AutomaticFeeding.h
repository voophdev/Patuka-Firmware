#ifndef AUTOMATIC_FEEDING_H
#define AUTOMATIC_FEEDING_H

#include <RTClib.h>  // For DateTime object

// Declare the automation state as extern
extern volatile bool isAutomationEnabled;

// Function to check if the current time matches any feeding time
void checkFeedingTimes(const DateTime& currentTime);

// Function to trigger automatic feeding asynchronously
void triggerAutomaticFeeding();

// Automatic feeding task function
void automaticFeedingTask(void *parameter);

// Function to set the automation state
void setAutomationEnabled(bool enabled);

#endif  // AUTOMATIC_FEEDING_H