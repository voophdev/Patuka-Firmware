#include "AutomaticFeeding.h"
#include <Arduino.h>
#include "Feeding.h"  // Include manual feeding module
#include "PumpControl.h"
#include "GSMModule.h"
#include "WeighingScale.h"  // Include weighing scale module
#include "FirebaseStream.h"
#include "RTCConfig.h"
#include "SMSTask.h"

// Feeding schedule: hour, minute, second (24-hour format)
const int NUM_FEEDING_TIMES = 3;
int feedingTimes[NUM_FEEDING_TIMES][3] = {
    {7, 0, 0},  // Example: 1:48:00 AM
    {15, 0, 0},  // Example: 1:50:00 AM
    {18, 0, 0}   // Example: 1:52:00 AM
};

// Automation state flag
volatile bool isAutomationEnabled = true;

// Function to set the automation state
void setAutomationEnabled(bool enabled) {
    isAutomationEnabled = enabled;
    if (enabled) {
        Serial.println("Automation enabled.");
    } else {
        Serial.println("Automation disabled.");
    }
}

// Static variables to track the last triggered feeding time
static int lastTriggeredHour = -1;
static int lastTriggeredMinute = -1;
static int lastTriggeredSecond = -1;

void checkFeedingTimes(const DateTime& currentTime) {
    if (!isAutomationEnabled) {
        return;  // Do nothing if automation is not enabled
    }

    const int tolerance = 2;  // Tolerance window of ±2 seconds

    for (int i = 0; i < NUM_FEEDING_TIMES; ++i) {
        int feedHour = feedingTimes[i][0];
        int feedMinute = feedingTimes[i][1];
        int feedSecond = feedingTimes[i][2];

        // Check if the current time matches the feeding time with tolerance
        if (currentTime.hour() == feedHour && 
            currentTime.minute() == feedMinute &&
            abs(currentTime.second() - feedSecond) <= tolerance) {
            
            // Check if the feeding has already been triggered for this feeding time
            if (feedHour != lastTriggeredHour || feedMinute != lastTriggeredMinute || feedSecond != lastTriggeredSecond) {
                triggerAutomaticFeeding();  // Trigger automatic feeding task
                
                // Update the last triggered time
                lastTriggeredHour = feedHour;
                lastTriggeredMinute = feedMinute;
                lastTriggeredSecond = feedSecond;
            }
            break;  // Exit loop to avoid unnecessary checks
        }
    }
}


// Function to trigger automatic feeding as a FreeRTOS task
void triggerAutomaticFeeding() {
    xTaskCreate(automaticFeedingTask, "AutomaticFeedingTask", 8192, NULL, 1, NULL);
}

// Automatic feeding task implementation
void automaticFeedingTask(void *parameter) {
    Serial.println("Automatic Feeding Task Started.");

    // Acquire scale mutex to get weight before feeding
    float weightBefore = 0.0;
    if (xSemaphoreTake(xScaleMutex, portMAX_DELAY) == pdTRUE) {
        // Get the weight before feeding
        weightBefore = currentWeight;
        Serial.print("Weight before automatic feeding: ");
        Serial.print(weightBefore, 2);
        Serial.println(" kg");
        xSemaphoreGive(xScaleMutex);
    }

    // Activate the servo to dispense feed
    activateServo();

    // Delay to stabilize scale readings
    vTaskDelay(8000 / portTICK_PERIOD_MS);  // Wait for 8 seconds before reading weight again


    // Acquire scale mutex to get weight after feeding
    float weightAfter = 0.0;
    if (xSemaphoreTake(xScaleMutex, portMAX_DELAY) == pdTRUE) {
        // Get the weight after feeding
        weightAfter = currentWeight;
        Serial.print("Weight after automatic feeding: ");
        Serial.print(weightAfter, 2);
        Serial.println(" kg");
        xSemaphoreGive(xScaleMutex);
    }

    // Calculate the total feed dispensed
    float totalFeedDispensed = calculateFeedDispensed(weightBefore, weightAfter);

    if (totalFeedDispensed < 0) {
    totalFeedDispensed = 0.00;  // Reset to 0.00 if less than 0
    }
    
    Serial.print("Total Feed Dispensed: ");
    Serial.print(totalFeedDispensed, 2);
    Serial.println(" kg");

    // Yield to prevent WDT issues
    vTaskDelay(50 / portTICK_PERIOD_MS);  // Increased delay to 50ms


    // Trigger water pump
    automaticTrigger(); // Activates the water pump for 10 seconds

    // Yield to prevent WDT issues
    vTaskDelay(50 / portTICK_PERIOD_MS);  // Increased delay to 50ms


    // Create a separate task for sending SMS notification with the total feed dispensed
    String *message = new String("Automatic Feed Successful!\nTotal Feed Dispensed: ");
    *message += String(totalFeedDispensed, 2);
    *message += " kg";
    BaseType_t xReturned = xTaskCreate(sendSMSTask, "SendSMSTask", 4096, (void *)message, 1, NULL);
    if (xReturned != pdPASS) {
        Serial.println("Failed to create SendSMSTask.");
        delete message;  // Free allocated memory if task creation failed
    }

    // Delay before proceeding to avoid WDT triggering during SMS sending
    vTaskDelay(2000 / portTICK_PERIOD_MS);


    // Create a task to update Firebase with feed dispensed data
    float *feedDispensed = new float(totalFeedDispensed);
    xReturned = xTaskCreate(firebaseUpdateTask, "FirebaseUpdateTask", 8192, (void *)feedDispensed, 1, NULL);
    if (xReturned != pdPASS) {
        Serial.println("Failed to create FirebaseUpdateTask.");
        delete feedDispensed;  // Free allocated memory if task creation failed
    }

    // Delete this task after completion
    vTaskDelete(NULL);
}