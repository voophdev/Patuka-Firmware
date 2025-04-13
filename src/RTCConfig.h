#ifndef RTCCONFIG_H
#define RTCCONFIG_H

#include <RTClib.h>
#include <Wire.h>

extern RTC_DS3231 rtc;  // Declare the RTC object

void initializeRTC();
String getFormattedTime12Hour();
String getFormattedDate();

#endif  // RTCCONFIG_H
