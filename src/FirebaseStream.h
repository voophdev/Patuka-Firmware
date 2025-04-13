#ifndef FIREBASESTREAM_H
#define FIREBASESTREAM_H

#include <FirebaseESP32.h>
#include <Arduino.h>

// Initialize Firebase streaming and create tasks
void initializeFirebaseStream();

// Callback function for Firebase streaming data updates for manual feed and automation
void streamCallbackManualFeed(StreamData data);
void streamCallbackAutomation(StreamData data);

// Callback function for Firebase streaming timeouts for manual feed and automation
void streamTimeoutCallbackManualFeed(bool timeout);
void streamTimeoutCallbackAutomation(bool timeout);

// FreeRTOS task functions for Firebase streaming
void firebaseStreamTaskManualFeed(void *parameter);
void firebaseStreamTaskAutomation(void *parameter);

#endif // FIREBASESTREAM_H
