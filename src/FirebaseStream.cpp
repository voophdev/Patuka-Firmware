#include "FirebaseStream.h"
#include "Feeding.h"
#include "PumpControl.h"
#include "AutomaticFeeding.h"
#include "LCDDisplay.h"

// Firebase Data objects for streaming
FirebaseData firebaseDataStreamManualFeed;
FirebaseData firebaseDataStreamAutomation;

// FreeRTOS task handle for Firebase streaming
TaskHandle_t firebaseTaskHandleManualFeed = NULL;
TaskHandle_t firebaseTaskHandleAutomation = NULL;

void initializeFirebaseStream() {
    // Create the FreeRTOS task for Firebase streaming for manual feed
    xTaskCreate(
        firebaseStreamTaskManualFeed,   // Task function
        "FirebaseStreamManualFeedTask", // Task name
        8192,                           // Stack size in bytes
        NULL,                           // Task parameters (none)
        1,                              // Task priority
        &firebaseTaskHandleManualFeed   // Task handle
    );

    // Create the FreeRTOS task for Firebase streaming for automation
    xTaskCreate(
        firebaseStreamTaskAutomation,   // Task function
        "FirebaseStreamAutomationTask", // Task name
        8192,                           // Stack size in bytes
        NULL,                           // Task parameters (none)
        1,                              // Task priority
        &firebaseTaskHandleAutomation   // Task handle
    );
}

void firebaseStreamTaskManualFeed(void *parameter) {
    if (Firebase.beginStream(firebaseDataStreamManualFeed, "/manualFeed")) {
        Firebase.setStreamCallback(firebaseDataStreamManualFeed, streamCallbackManualFeed, streamTimeoutCallbackManualFeed);
        Serial.println("Firebase stream (manual feed) started successfully.");

        // Keep the task alive for Firebase to continue streaming
        while (true) {
            vTaskDelay(10 / portTICK_PERIOD_MS); // 5 seconds delay between yields
        }
    } else {
        Serial.println("Failed to start Firebase stream (manual feed).");
        Serial.println("Reason: " + firebaseDataStreamManualFeed.errorReason());

        // Delete this task if streaming failed
        vTaskDelete(NULL);
    }
}

void firebaseStreamTaskAutomation(void *parameter) {
    if (Firebase.beginStream(firebaseDataStreamAutomation, "/automation/isEnabled")) {
        Firebase.setStreamCallback(firebaseDataStreamAutomation, streamCallbackAutomation, streamTimeoutCallbackAutomation);
        Serial.println("Firebase stream (automation/isEnabled) started successfully.");

        // Keep the task alive for Firebase to continue streaming
        while (true) {
            vTaskDelay(10 / portTICK_PERIOD_MS); // 5 seconds delay between yields
        }
    } else {
        Serial.println("Failed to start Firebase stream (automation/isEnabled).");
        Serial.println("Reason: " + firebaseDataStreamAutomation.errorReason());

        // Delete this task if streaming failed
        vTaskDelete(NULL);
    }
}

void streamCallbackManualFeed(StreamData data) {
    // Check if the feed value was updated
    if (data.dataPath() == "/feed") {
        if (data.boolData()) {
            Serial.println("Manual feed triggered from Firebase!");
            createManualFeedTask();
        }
    }

    // Check if the water pump value was updated
    if (data.dataPath() == "/water") {
        bool waterPumpState = data.boolData();
        Serial.printf("Water pump state updated from Firebase: %s\n", waterPumpState ? "ON" : "OFF");

        // Update the water pump status
        setWaterPumpRequested(waterPumpState);
    }
}

void streamCallbackAutomation(StreamData data) {
    // Check if the isEnabled value was updated
    if (data.dataType() == "boolean") {
        bool automationEnabledState = data.boolData();
        isAutomationEnabled = automationEnabledState;

        // Update the LCD to display the automation state
        lcd.setCursor(0, 3);
        lcd.print("                "); // Clear the line first
        lcd.setCursor(0, 3);
        lcd.print("Automation: ");
        lcd.print(isAutomationEnabled ? "Enabled " : "Disabled");
    }
}

void streamTimeoutCallbackManualFeed(bool timeout) {
    if (timeout) {
        Serial.println("Stream timed out for manual feed, resuming connection...");
        Firebase.beginStream(firebaseDataStreamManualFeed, "/manualFeed"); // Restart the stream
    }
}

void streamTimeoutCallbackAutomation(bool timeout) {
    if (timeout) {
        Serial.println("Stream timed out for automation/isEnabled, resuming connection...");
        Firebase.beginStream(firebaseDataStreamAutomation, "/automation/isEnabled"); // Restart the stream
    }
}

