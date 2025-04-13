#include "WiFiSetup.h"
#include <WiFi.h>
#include <Preferences.h>
#include <WebServer.h>
#include "FirebaseConfig.h"

// Web server on port 80
WebServer server(80);
Preferences preferences;
bool isConnected = false;

void startAccessPoint() {
    const char* ssid = PATUKA_AP_SSID;
    const char* password = PATUKA_AP_PASSWORD;

    WiFi.softAP(ssid, password);  // Start AP mode
    Serial.println();
    Serial.printf("Access Point \"%s\" started with IP: ", ssid);
    Serial.println(WiFi.softAPIP());
}

void disableAccessPoint() {
    WiFi.softAPdisconnect(true);  // Disable AP mode
    Serial.println("Access Point mode disabled.");
}

void saveCredentials(const String& ssid, const String& password) {
    preferences.begin("wifi", false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.end();
}

bool loadCredentials(String& ssid, String& password) {
    preferences.begin("wifi", true);
    ssid = preferences.getString("ssid", "");
    password = preferences.getString("password", "");
    preferences.end();
    return !ssid.isEmpty() && !password.isEmpty();
}

void connectToWiFi() {
    const char* ssid = PATUKA_WIFI_SSID;
    const char* password = PATUKA_WIFI_PASSWORD;

    WiFi.mode(WIFI_AP_STA);  // Enable both AP and STA modes

    Serial.printf("Connecting to Wi-Fi: %s\n", ssid);
    WiFi.begin(ssid, password);

    unsigned long startTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {
        delay(50);  // Keep delay short to avoid blocking
        server.handleClient();  // Allow the server to handle requests
    }

    if (WiFi.status() == WL_CONNECTED) {
        isConnected = true;
        Serial.println("\nConnected to Wi-Fi!");
        disableAccessPoint();  // Turn off AP mode if connected
        setupFirebase();
    } else {
        Serial.println("\nFailed to connect to Wi-Fi. Keeping AP mode active.");
        startAccessPoint();  // Fallback to AP mode
    }
}

void setupWebServer() {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/configure", HTTP_POST, handleConfigure);
    server.begin();
    Serial.println("HTTP server started");
}

void handleRoot() {
    String html = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <meta name='viewport' content='width=device-width, initial-scale=1.0'>
            <title>Configure Wi-Fi</title>
            <style>
                body { font-family: Arial, sans-serif; background-color: #f4f4f4; margin: 0; padding: 20px;
                       display: flex; justify-content: center; align-items: center; height: 100vh; }
                .container { width: 100%; max-width: 400px; background: #fff; padding: 20px; border-radius: 8px;
                             box-shadow: 0px 0px 10px rgba(0,0,0,0.1); box-sizing: border-box; }
                h1 { color: #333; text-align: center; font-size: 24px; margin-bottom: 20px; }
                input[type='text'], input[type='password'], input[type='submit'] {
                    width: 100%; padding: 10px; margin-bottom: 20px; border: 1px solid #ccc; border-radius: 4px;
                    font-size: 16px; box-sizing: border-box; }
                input[type='submit'] { background-color: #4CAF50; color: white; border: none; cursor: pointer; }
                input[type='submit']:hover { background-color: #45a049; }
                .note { font-size: 12px; color: #777; text-align: center; margin-top: 20px; }
            </style>
        </head>
        <body>
            <div class='container'>
                <h1>Configure Wi-Fi</h1>
                <form action='/configure' method='post'>
                    <input type='text' name='ssid' placeholder='Enter Wi-Fi SSID' required><br>
                    <input type='password' name='password' placeholder='Enter Wi-Fi Password' required><br>
                    <input type='submit' value='Configure'>
                </form>
                <div class='note'>Make sure the network is available and has internet access.</div>
            </div>
        </body>
        </html>
    )";
    server.send(200, "text/html", html);
}

void handleConfigure() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    if (ssid.isEmpty() || password.isEmpty()) {
        server.send(400, "text/html", "SSID or Password cannot be empty.");
        return;
    }

    saveCredentials(ssid, password);
    Serial.println("Saved credentials. Reconnecting to Wi-Fi...");

    WiFi.begin(ssid.c_str(), password.c_str());
    unsigned long startTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {
        delay(50);  // Small delay to prevent blocking
        server.handleClient();  // Ensure the server stays responsive
    }

    if (WiFi.status() == WL_CONNECTED) {
        isConnected = true;
        setupFirebase();
        server.send(200, "text/html", "Connected to Wi-Fi successfully. Please go back to Patuka App");
        delay(2000);
        disableAccessPoint();
    } else {
        server.send(200, "text/html", "Failed to connect. Try again.");
        startAccessPoint();
    }

    restartWebServer();  // Restart server to ensure it's active
}

void checkWiFiStatus() {
    if (WiFi.status() == WL_CONNECTED && !isConnected) {
        isConnected = true;
        Serial.println("Wi-Fi connection re-established.");
        setupFirebase();
        tokenCheck();
        disableAccessPoint();  // Disable AP if connected to Wi-Fi

    } else if (WiFi.status() != WL_CONNECTED && isConnected) {
        isConnected = false;
        Serial.println("Lost Wi-Fi connection. Re-enabling AP mode.");
        startAccessPoint();  // Restart AP mode to allow reconfiguration
        server.begin();  // Ensure server is active in AP mode
    }
}

void restartWebServer() {
    server.begin();  // Restart the web server
    Serial.println("Web server restarted.");
}
