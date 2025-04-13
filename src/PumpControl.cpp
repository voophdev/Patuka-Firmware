#include <Arduino.h>
#include "PumpControl.h"
#include "PinConfig.h"

// FreeRTOS task handle for pump control
TaskHandle_t pumpControlTaskHandle = NULL;

// Debounce variables for hardware button
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;  // 50ms debounce delay

// Flag to indicate a water pump request from Firebase
volatile bool isWaterPumpRequested = false;

bool isPumpRunningByButton = false;  // To track pump state started by button
bool isPumpRunningByFirebase = false; // To track pump state started by Firebase

// Setup function to initialize pump control
void setupPumpControl() {
    pinMode(RELAY_PIN, OUTPUT);  // Set relay as output
    pinMode(PUMP_BUTTON_PIN, INPUT_PULLUP);  // Use internal pull-up resistor for the button

    // Make sure relay is initially off
    digitalWrite(RELAY_PIN, HIGH); // Set HIGH to keep relay off initially

    // Create the FreeRTOS task for pump control
    xTaskCreate(
        pumpControlTask,         // Task function
        "PumpControlTask",       // Task name
        2048,                    // Stack size (increased for stability)
        NULL,                    // Task parameters
        1,                       // Task priority
        &pumpControlTaskHandle   // Task handle
    );

    Serial.begin(115200);
    Serial.println("Pump control initialized.");
}

// FreeRTOS task to manage pump control with debounce and Firebase integration
void pumpControlTask(void *parameter) {
    while (true) {
        unsigned long currentMillis = millis();

        // Handle Firebase-triggered water pump control
        if (isWaterPumpRequested && !isPumpRunningByFirebase) {
            // Start the pump when Firebase request is true
            Serial.println("Firebase triggered water pump ON.");
            digitalWrite(RELAY_PIN, LOW);  // Set LOW to turn the pump on
            isPumpRunningByFirebase = true;
        } else if (!isWaterPumpRequested && isPumpRunningByFirebase) {
            // Stop the pump when Firebase request changes to false
            Serial.println("Firebase triggered water pump OFF.");
            digitalWrite(RELAY_PIN, HIGH);  // Set HIGH to turn the pump off
            isPumpRunningByFirebase = false;
        }

        // Handle button-pressed water pump control independently
        int buttonState = digitalRead(PUMP_BUTTON_PIN);

        // Debounce logic for hardware button press
        if (buttonState == LOW && (currentMillis - lastDebounceTime) > debounceDelay) {
            // Button pressed and enough time passed since the last debounce
            if (!isPumpRunningByButton && !isWaterPumpRequested) {
                // Start the pump only if Firebase is not requesting it
                digitalWrite(RELAY_PIN, LOW);  // Set LOW to turn the pump on
                Serial.println("Pump is running (button pressed)...");
                isPumpRunningByButton = true;
            }
            lastDebounceTime = currentMillis;  // Update the debounce timer
        } else if (buttonState == HIGH && isPumpRunningByButton && (currentMillis - lastDebounceTime) > debounceDelay) {
            // Button released and enough time passed since the last debounce
            // Stop the pump only if it was manually started by the button
            Serial.println("Pump stopped (button released).");
            digitalWrite(RELAY_PIN, HIGH);  // Set HIGH to turn the pump off
            isPumpRunningByButton = false;
            lastDebounceTime = currentMillis;  // Update the debounce timer
        }

        // Delay to prevent the task from using 100% CPU
        vTaskDelay(50 / portTICK_PERIOD_MS);  // 50ms delay
    }
}

// Function to automatically trigger the water pump for 10 seconds
void automaticTrigger() {
    Serial.println("Automatic trigger: Pump is running for 10 seconds...");
    digitalWrite(RELAY_PIN, LOW);  // Set LOW to turn the pump on
    delay(10000);  // Run the pump for 10 seconds
    digitalWrite(RELAY_PIN, HIGH);  // Set HIGH to turn the pump off
    Serial.println("Automatic trigger: Pump stopped.");
}

// Function to set the pump request flag from Firebase
void setWaterPumpRequested(bool request) {
    isWaterPumpRequested = request;

    // Stop pump immediately if the request is false and it is currently running by Firebase
    if (!request && isPumpRunningByFirebase) {
        Serial.println("Immediate stop triggered by Firebase request change to OFF.");
        digitalWrite(RELAY_PIN, HIGH);  // Set HIGH to turn the pump off
        isPumpRunningByFirebase = false;
    }
}
