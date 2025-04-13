#include "GSMModule.h"

// Define GSM Serial and TinyGSM object
HardwareSerial gsmSerial(2);  // UART2 for GSM
TinyGsm modem(gsmSerial);      // Initialize TinyGSM with UART2

const char* PHONE_NUMBER = "+639055125873";  // Replace with your number
bool initializedGSM = false;

// Send SMS using TinyGSM
void sendSMS(const String& message) {
    gsmSerial.begin(112500, SERIAL_8N1, 16, 17);

    // Use TinyGSM's built-in SMS sending function
    if (!initializedGSM) {
        if (modem.restart()) {
        Serial.println("GSM module initialized successfully.");    
        initializedGSM = true;  
        } else {
            Serial.println("Failed to initialize GSM module.");
        }
    }

    Serial.println("Sending SMS...");
    delay(4000);

    if (modem.sendSMS(PHONE_NUMBER, message)) {
        Serial.println("SMS sent successfully.");
    } else {
        Serial.println("SMS sending failed.");
    }
  gsmSerial.end();
  gsmSerial.begin(9600, SERIAL_8N1, 16, 17);
}
