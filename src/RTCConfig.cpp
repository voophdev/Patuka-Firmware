#include "RTCConfig.h"

// Initialize RTC object
RTC_DS3231 rtc;

void initializeRTC() {
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC. Check your wiring!");
        while (1);  // Halt if RTC is not detected
    }

    if (rtc.lostPower()) {
        Serial.println("RTC lost power, setting to compile time.");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // Set RTC to compile time
    }

    Serial.println("RTC initialized.");
}

// Get the formatted time in 12-hour format
String getFormattedTime12Hour() {
    DateTime now = rtc.now();  // Retrieve the current time
    int hour = now.hour();
    int minute = now.minute();
    int second = now.second();
    String period = (hour < 12) ? "AM" : "PM";

    // Convert 24-hour to 12-hour format
    if (hour == 0) {
        hour = 12;  // Midnight case
    } else if (hour > 12) {
        hour -= 12;
    }

    // Format the time string with leading zeros
    char timeBuffer[12];  // Buffer for formatted time
    snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d:%02d %s", hour, minute, second, period.c_str());

    return String(timeBuffer);
}

// Get the formatted date
String getFormattedDate() {
    DateTime now = rtc.now();
    char dateBuffer[11];  // Buffer for formatted date
    snprintf(dateBuffer, sizeof(dateBuffer), "%02d/%02d/%04d", now.month(), now.day(), now.year());

    return String(dateBuffer);
}
