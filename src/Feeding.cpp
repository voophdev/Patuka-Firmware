#include "Feeding.h"
#include "PinConfig.h"
#include <Arduino.h>
#include <ESP32Servo.h>
#include "GSMModule.h"
#include "FirebaseStream.h"
#include "WeighingScale.h"
#include "SMSTask.h"

// Create the servo object
Servo feedServo;

// Track the state of the manual feed button and Firebase
volatile bool isFeedRequested = false;  // Global flag for feeding request

// Debounce variables for hardware button
volatile unsigned long lastFeedDebounceTime = 0;
const unsigned long feedDebounceDelay = 200;  // 200ms debounce delay

// FreeRTOS task handle for manual feed
TaskHandle_t manualFeedTaskHandle = NULL;

// Mutex for weighing scale
SemaphoreHandle_t xScaleMutex;

// Global variable to hold the current weight
float currentWeight = 0.0;

// ISR for manual feed button press
void IRAM_ATTR handleFeedButtonPress() {
    unsigned long currentTime = millis();
    if (currentTime - lastFeedDebounceTime > feedDebounceDelay) {
        lastFeedDebounceTime = currentTime;  // Update debounce time

        // Disable the interrupt to avoid bouncing
        detachInterrupt(digitalPinToInterrupt(SERVO_BUTTON_PIN));

        // Set the feed request flag
        isFeedRequested = true;

        // Create the manual feed task
        createManualFeedTask();
    }
}

// Initialize the manual feed system
void initializeManualFeed(int servoPin, int buttonPin) {
    // Initialize the weighing scale
    setupWeighingScale();
    feedServo.attach(servoPin);  // Attach the servo
    feedServo.write(90);  // Neutral position

    // Configure the button as input with pull-up and attach the interrupt
    pinMode(buttonPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(buttonPin), handleFeedButtonPress, FALLING);

    // Create the mutex for the weighing scale
    xScaleMutex = xSemaphoreCreateMutex();
    if (xScaleMutex == NULL) {
        Serial.println("Failed to create mutex for weighing scale.");
    }

    // Create the weight monitoring task
    BaseType_t xReturned = xTaskCreate(
        weightMonitoringTask,     // Task function
        "WeightMonitoringTask",  // Task name
        4096,                     // Stack size (in bytes)
        NULL,                     // Task parameters (none)
        1,                        // Task priority
        NULL                      // Task handle (not needed)
    );

    if (xReturned != pdPASS) {
        Serial.println("Failed to create WeightMonitoringTask.");
    }

    Serial.println("Manual feed system initialized.");
}

// Create the manual feed task
void createManualFeedTask() {
    // Create the manual feed task if not already created
    if (manualFeedTaskHandle == NULL) {
        BaseType_t xReturned = xTaskCreate(
            manualFeedTask,          // Task function
            "ManualFeedTask",        // Task name
            8192,                    // Stack size (in bytes)
            NULL,                    // Task parameters (none)
            1,                       // Task priority
            &manualFeedTaskHandle    // Task handle
        );

        if (xReturned != pdPASS) {
            Serial.println("Failed to create ManualFeedTask.");
            manualFeedTaskHandle = NULL;
        }
    }
}

void manualFeedTask(void *parameter) {
    Serial.println("Manual feed task running...");

    float weightBefore = 0.0;
    // Get the weight before feeding (protect with a mutex)
    if (xSemaphoreTake(xScaleMutex, portMAX_DELAY) == pdTRUE) {
        weightBefore = currentWeight;
        Serial.print("Weight before feeding: ");
        Serial.print(weightBefore, 2);
        Serial.println(" kg");
        xSemaphoreGive(xScaleMutex);
    }

    // Activate the servo
    activateServo();

    // Increase the delay to stabilize scale readings
    vTaskDelay(8000 / portTICK_PERIOD_MS);  // Wait for 8 seconds before reading weight again

    float weightAfter = 0.0;
    // Get the weight after feeding (protect with a mutex)
    if (xSemaphoreTake(xScaleMutex, portMAX_DELAY) == pdTRUE) {
        weightAfter = currentWeight;
        Serial.print("Weight after feeding: ");
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

    // Create a separate task for sending SMS notification with the total feed dispensed
    String *message = new String("Manual Feed Successful!\nTotal Feed Dispensed: ");
    *message += String(totalFeedDispensed, 2);
    *message += " kg";
    xTaskCreate(sendSMSTask, "SendSMSTask", 4096, (void *)message, 1, NULL);

    // Delay before proceeding to avoid WDT triggering during SMS sending
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    // Create a task to update Firebase with feed dispensed data
    float *feedDispensed = new float(totalFeedDispensed);
    xTaskCreate(firebaseUpdateTask, "FirebaseUpdateTask", 8192, (void *)feedDispensed, 1, NULL);

    // Re-enable the interrupt for future button presses
    attachInterrupt(digitalPinToInterrupt(SERVO_BUTTON_PIN), handleFeedButtonPress, FALLING);

    // Reset the button pressed flag
    isFeedRequested = false;

    // Delete the task after completion and reset the task handle
    manualFeedTaskHandle = NULL;
    vTaskDelete(NULL);
}

// Activate the servo motor for manual feed
void activateServo() {
    Serial.println("Activating servo...");

    // Rotate to 180 degrees (clockwise)
    feedServo.write(180);
    vTaskDelay(300 / portTICK_PERIOD_MS);  // 1-second delay

    // Reset to neutral position (90 degrees)
    feedServo.write(90);
    vTaskDelay(2000 / portTICK_PERIOD_MS);  // 1-second delay

    // Rotate to 0 degrees (counter-clockwise)
    feedServo.write(0);
    vTaskDelay(320 / portTICK_PERIOD_MS);  // 1-second delay

    // Reset to neutral position (90 degrees)
    feedServo.write(90);
    vTaskDelay(1000 / portTICK_PERIOD_MS);  // 1-second delay

    Serial.println("Servo reset to initial position.");
}
