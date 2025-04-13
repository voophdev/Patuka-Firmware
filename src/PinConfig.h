#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

// Pin and sensor configuration as compile-time constants
constexpr int TRIG_PIN_1 = 5;
constexpr int ECHO_PIN_1 = 18;
constexpr int WATER_MAX_DISTANCE = 45;
constexpr int WATER_MIN_DISTANCE = 10;

constexpr int TRIG_PIN_2 = 4;
constexpr int ECHO_PIN_2 = 19;
constexpr int FEED_MAX_DISTANCE = 44;
constexpr int FEED_MIN_DISTANCE = 10;

constexpr int DHT_PIN = 25;  // GPIO pin connected to DHT22 data pin
constexpr int DS18B20_PIN = 26;
constexpr int PH_SENSOR_PIN = 32;   //GPIO pin for pH Sensor

constexpr int PUMP_BUTTON_PIN = 33;   // Button for pump control
constexpr int SERVO_BUTTON_PIN = 14;  // Button for servo control
constexpr int RELAY_PIN = 23;         // Relay control pin for pump
constexpr int SERVO_PIN = 15;         // Servo motor control pin

constexpr int LOADCELL_DOUT_PIN = 13;
constexpr int LOADCELL_SCK_PIN = 12;

#endif  // PIN_CONFIG_H