#ifndef GSMModule_h
#define GSMModule_h

// Define the modem type for TinyGSM
#define TINY_GSM_MODEM_SIM900  // Ensure SIM900A is correctly identified

#include <Arduino.h>
#include <TinyGsmClient.h>

// Declare GSM module on UART2 (GPIO 16 = RX, 17 = TX)
extern HardwareSerial gsmSerial;
extern TinyGsm modem;  // Declare TinyGsm object

// Function prototypes for GSM initialization and sending SMS
void sendSMS(const String& message);

#endif  // GSMModule_h
