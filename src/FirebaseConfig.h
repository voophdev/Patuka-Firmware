#ifndef FIREBASE_CONFIG_H
#define FIREBASE_CONFIG_H

#include <FirebaseESP32.h>
// #include "GSMModule.h"
// #include "PinConfig.h"

extern FirebaseData firebaseDataUpdate;  // For other operations
extern FirebaseConfig config;

void setupFirebase();
void tokenCheck();
void updateFirebaseLevel(FirebaseJson& json);
void updateFirebaseLiveSensorMonitoring(FirebaseJson& json);


#endif // FIREBASE_CONF IG_H
