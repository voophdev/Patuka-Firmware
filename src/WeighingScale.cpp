#include "WeighingScale.h"
#include <Arduino.h>
#include "PinConfig.h"
#include "FirebaseStream.h"
#include "RTCConfig.h"

HX711 scale;

FirebaseData firebaseData;
FirebaseJson json;

// RECALIBRATE AFTER SYSTEM SETUP
float calibrationFactor = 21.83; // Calibration factor to be set after calibration
float tareValue = 73019.00; // Tare value to be determined during calibration

// float calibrationFactor = 0.00; // Calibration factor to be set after calibration
// float tareValue = 0.00; // Tare value to be determined during calibration

// Setup function for the weighing scale
void setupWeighingScale() {
    Serial.println("Initializing weighing scale...");
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    delay(2000);
    
    if (scale.wait_ready_retry(10, 500)) {
        Serial.println("Scale is ready.");
    } else {
        Serial.println("Scale not found. Please check your wiring.");
        while (1);  // Stop execution here
    }
    
    // Set calibration factor and tare value
    scale.set_scale(calibrationFactor);
    scale.set_offset(tareValue);
    Serial.println("Weighing scale setup complete.");
}

// Get the current weight reading from the weighing scale
float getWeight() {
    long reading = scale.get_units(15); // Increase the number of samples for a more stable reading
    return reading / 1000.0; // Convert from grams to kilograms
}

// Calculate the feed dispensed based on before and after weight readings
float calculateFeedDispensed(float weightBefore, float weightAfter) {
    return weightBefore - weightAfter;
}

// Task to monitor the weight periodically
void weightMonitoringTask(void *parameter) {
    while (true) {
        if (xSemaphoreTake(xScaleMutex, portMAX_DELAY) == pdTRUE) {
            if (scale.wait_ready_timeout(1000)) {
                currentWeight = getWeight();
            } else {
                Serial.println("Scale not ready for weight reading.");
            }
            xSemaphoreGive(xScaleMutex);
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS); // Delay between each reading
    }
}

// Global variables
int sessionCount = 0;  // Local session counter
String previousDate = "";  // Store the previous date to detect changes

// Task to send feed dispensed data to Firebase with retry logic
// Task to send feed dispensed data to Firebase with retry logic
void firebaseUpdateTask(void *parameter) {
    // Cast the parameter to float pointer and dereference to get the total feed dispensed value
    float totalFeedDispensed = *((float *)parameter);

    // Get the current date and time from the RTC
    DateTime now = rtc.now();
    String dateStr = String(now.year()) + "-" + (now.month() < 10 ? "0" : "") + String(now.month()) + "-" + (now.day() < 10 ? "0" : "") + String(now.day());
    String timeStr = (now.hour() < 10 ? "0" : "") + String(now.hour()) + ":" + (now.minute() < 10 ? "0" : "") + String(now.minute());

    // Check if the date has changed
    if (previousDate != dateStr) {
        previousDate = dateStr;
        sessionCount = 0;  // Reset session count for the new day
    }

    // Prepare the path to be sent to Firebase
    String sessionPath = "/monthlyFeedData/" + dateStr + "/feedSessions/" + String(sessionCount);

    // Prepare the JSON data to be sent to Firebase
    FirebaseJson json;
    json.set("time", timeStr);
    json.set("feedDispensed", totalFeedDispensed);

    // Retry logic for Firebase update
    int retryCount = 0;
    const int maxRetries = 3;
    bool success = false;

    while (retryCount < maxRetries && !success) {
        // Yield to prevent WDT reset
        taskYIELD();

        // Send data to Firebase
        if (Firebase.setJSON(firebaseData, sessionPath, json)) {
            Serial.println("Firebase update successful.");
            sessionCount++;  // Increment session count after successful update
            success = true;
        } else {
            Serial.print("Firebase update failed. Attempt ");
            Serial.print(retryCount + 1);
            Serial.print(" of ");
            Serial.println(maxRetries);
            retryCount++;

            // Yield and delay to prevent WDT reset and allow retries
            taskYIELD();
            vTaskDelay(200 / portTICK_PERIOD_MS); // Delay between retries
        }

        // Yield to avoid WDT reset
        taskYIELD();
    }

    if (!success) {
        Serial.println("Firebase update failed after maximum retries.");
    }

    // Ensure to delete the task properly
    vTaskDelete(NULL);
}

// Function to calibrate the weighing scale
void calibrateWeighingScale() {
    Serial.println("Tare calibration in progress...");
    Serial.println("Ensure the scale is empty, then press 't' to tare.");
    
    // Wait for user to input the tare command via serial
    while (!Serial.available() || Serial.read() != 't') {
        delay(500); // Wait for user input
    }
    
    // Perform tare to set the zero reference
    scale.tare();
    tareValue = scale.get_offset();
    Serial.print("Tare value: ");
    Serial.println(tareValue);
    Serial.println("Tare complete.");
    
    Serial.println("Place a known weight on the scale and enter the weight in grams.");
    
    // Wait for user to input the known weight via serial
    while (!Serial.available()) {
        delay(500); // Wait for user input
    }
    
    float knownWeight = Serial.parseFloat();
    Serial.print("Known weight: ");
    Serial.println(knownWeight);

    // Get the raw average reading
    long rawReading = scale.get_units(15); // Use more samples for stability

    // Calculate the calibration factor
    calibrationFactor = rawReading / knownWeight;

    // Update the calibration factor on the scale
    scale.set_scale(calibrationFactor);

    Serial.print("Calibration complete. New calibration factor is: ");
    Serial.println(calibrationFactor);
}

// Function to handle serial input for calibration
void handleCalibrationCommand() {
    if (Serial.available()) {
        char command = Serial.read();
        if (command == 'c') {
            calibrateWeighingScale();
        } else if (command == 'u') {
            calibrationFactor += 0.1; // Increase calibration factor
            scale.set_scale(calibrationFactor);
            Serial.print("Calibration factor increased to: ");
            Serial.println(calibrationFactor);
        } else if (command == 'd') {
            calibrationFactor -= 0.1; // Decrease calibration factor
            scale.set_scale(calibrationFactor);
            Serial.print("Calibration factor decreased to: ");
            Serial.println(calibrationFactor);
        }
    }
}
