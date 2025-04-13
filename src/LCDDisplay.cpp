#include <Wire.h>
#include "LCDDisplay.h"
#include "WiFiSetup.h"


// Initialize the LCD with I2C address 0x27 for 16x4 display
LiquidCrystal_I2C lcd(0x27, 20, 4);

byte degreeSymbol[8] = {
  B00111,
  B00101,
  B00111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

void printCentered(String text, int row) {
  int lcdWidth = 20; // Width of the LCD (columns)
  int startingColumn = (lcdWidth - text.length()) / 2; // Calculate starting column
  
  // Ensure the starting column is non-negative (in case text is too long)
  startingColumn = max(0, startingColumn);
  
  lcd.setCursor(startingColumn, row); // Set cursor position
  lcd.print(text); // Print text
}

void setupLCD() {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.createChar(0, degreeSymbol); 
}

// void displayLevels(int waterLevel, int feedLevel) {
//     lcd.setCursor(0, 2);  // Third line for water level
//     lcd.print("Water: ");
//     lcd.print(waterLevel);
//     lcd.print("%    ");  // Padding to clear previous values

//     lcd.setCursor(0, 3);  // Fourth line for feed level
//     lcd.print("Feed: ");
//     lcd.print(feedLevel);
//     lcd.print("%    ");  // Padding to clear previous values
// }

void displayTime(const String &time, const String &date) {
    lcd.setCursor(6, 0);
    lcd.print("              ");
    // lcd.setCursor(0, 1);
    // lcd.print("                    ");
    // lcd.setCursor(0, 2);
    // lcd.print("                    ");

    lcd.setCursor(0, 0);  // First line for time
    lcd.print("Time: ");
    lcd.print(time);
    lcd.print("  ");  // Padding to clear previous values

    lcd.setCursor(0, 1);  // Second line for date
    lcd.print("Date: ");
    lcd.print(date);
    lcd.print("  ");  // Padding to clear previous values
    lcd.setCursor(0, 2);

    if (WiFi.status() == WL_CONNECTED) {
        lcd.print("WiFi: Connected");
        lcd.print("     ");  // Padding to clear previous values
    } else {
        lcd.print("WiFi: Disconnected");
        lcd.print("  ");  // Padding to clear previous values
    }
}
