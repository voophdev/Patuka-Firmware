#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include "LCDDisplay.h"
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>

extern LiquidCrystal_I2C lcd;

void setupLCD();
void displayTime(const String &time, const String &date);
void printCentered(String text, int row);

// Bitmap for the degree symbol
extern byte degreeSymbol[8];

#endif
