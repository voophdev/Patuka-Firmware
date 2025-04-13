#include "TimeDisplay.h"
#include "LCDDisplay.h"
#include "RTCConfig.h"

void updateTimeDisplayTask(void *parameter) {
    while (true) {
        // Get the current time and date
        String currentTime = getFormattedTime12Hour();  // Ensure this returns a String
        String currentDate = getFormattedDate();        // Ensure this returns a String

        // Display the time and date
        displayTime(currentTime, currentDate);

        // Delay for 1 second before the next update
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void createTimeDisplayTask() {
    xTaskCreate(
        updateTimeDisplayTask,   // Task function
        "TimeDisplayTask",       // Task name
        2048,                    // Stack size in bytes
        NULL,                    // Parameter to pass to the task
        1,                       // Task priority
        NULL                     // Task handle (optional)
    );
}
