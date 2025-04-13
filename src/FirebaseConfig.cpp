#include "FirebaseConfig.h"
#include "WiFiSetup.h"
#include "FirebaseStream.h" 
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// Firebase objects
// FirebaseData firebaseDataStream;  // For streaming
FirebaseData firebaseDataUpdate;  // For other operations
FirebaseAuth auth;
FirebaseConfig config;

unsigned int firebaseReconnectAttempts = 0;

void setupFirebase() {
    String uid;
    config.api_key = PATUKA_FIREBASE_API_KEY;
    auth.user.email = PATUKA_FIREBASE_USER_EMAIL;
    auth.user.password = PATUKA_FIREBASE_USER_PASSWORD;
    config.database_url = PATUKA_FIREBASE_DATABASE_URL;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    Firebase.setReadTimeout(firebaseDataUpdate, 60000);
    Firebase.setwriteSizeLimit(firebaseDataUpdate, "tiny");

    // Assign the callback function for the long running token generation task
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

    // Assign the maximum retry of token generation
    config.max_token_generation_retry = 5;

    Serial.println("Getting User UID");
    while ((auth.token.uid) == "") {
        Serial.print('.');
        delay(1000);
    }
    // Print user UID
    uid = auth.token.uid.c_str();
    Serial.print("User UID: ");
    Serial.print(uid);
    
    initializeFirebaseStream();
    Serial.println("Firebase setup complete.");
}

bool isUpdatingFirebase = false;  // Mutex to prevent overlapping updates

void tokenCheck() {
    if (Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
    Serial.println("Refresh token");
  } else(Serial.println("Token still valid"));
}

void updateFirebaseLevel(FirebaseJson& json) {
    static int attempt = 0;  // Track the current attempt
    static unsigned long lastRetryMillis = 0;  // Time of the last attempt

    if (isUpdatingFirebase) {
        if (millis() - lastRetryMillis < 500) {
            return;  // Wait 500ms before retrying
        }
        attempt++;  // Increment attempt counter

        if (attempt > 3) {  // Max retries reached
            Serial.printf("Failed to update batch after %d attempts.\n", 3);
            isUpdatingFirebase = false;  // Unlock mutex
            attempt = 0;  // Reset attempt counter
            return;
        }
    } else {
        isUpdatingFirebase = true;  // Start the update process
        attempt = 1;  // First attempt
    }

    // Update Firebase with the batch data
    if (Firebase.updateNode(firebaseDataUpdate, "/levelMonitor", json)) {
        Serial.println("Batch update for /levelMonitor successful.");
        isUpdatingFirebase = false;  // Unlock mutex
        attempt = 0;  // Reset attempt counter
    } else {
        Serial.printf("Batch update failed: %s (Attempt %d of 3)\n", 
            firebaseDataUpdate.errorReason().c_str(), 
            attempt);
        lastRetryMillis = millis();  // Record the time of this attempt
    }
}

void updateFirebaseLiveSensorMonitoring(FirebaseJson& json) {
    static int attempt = 0;  // Track the current attempt
    static unsigned long lastRetryMillis = 0;  // Time of the last attempt

    if (isUpdatingFirebase) {
        if (millis() - lastRetryMillis < 500) {
            return;  // Wait 500ms before retrying
        }
        attempt++;  // Increment attempt counter

        if (attempt > 3) {  // Max retries reached
            Serial.printf("Failed to update after %d attempts.\n", 3);
            isUpdatingFirebase = false;  // Unlock mutex
            attempt = 0;  // Reset attempt counter
            return;
        }
    } else {
        isUpdatingFirebase = true;  // Start the update process
        attempt = 1;  // First attempt
    }

    // Update Firebase in batch with the FirebaseJson object
    if (Firebase.updateNode(firebaseDataUpdate, "/liveSensorMonitoring", json)) {
        Serial.println("Batch update for /liveSensorMonitoring successful.");
        isUpdatingFirebase = false;  // Unlock mutex
        attempt = 0;  // Reset attempt counter
    } else {
        Serial.printf("Batch update failed: %s (Attempt %d of 3)\n", 
            firebaseDataUpdate.errorReason().c_str(), 
            attempt);
        lastRetryMillis = millis();  // Record the time of this attempt
    }
}