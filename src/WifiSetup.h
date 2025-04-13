#ifndef WIFISETUP_H
#define WIFISETUP_H

#include <WiFi.h>
#include <Preferences.h>
#include <WebServer.h>

// Externally declared variables (accessible from other files)
extern WebServer server;
extern Preferences preferences;
extern bool isConnected;

// Function declarations
void startAccessPoint();
void saveCredentials(const String& ssid, const String& password);
bool loadCredentials(String& ssid, String& password);
void connectToWiFi();
void setupWebServer();
void handleRoot();
void handleConfigure();
void checkWiFiStatus();
void reconnectWiFi();
void restartWebServer();

#endif // WIFISETUP_H
