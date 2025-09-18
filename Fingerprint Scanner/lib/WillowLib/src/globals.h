#pragma once
#include <stdint.h>

// Configure pins
constexpr uint16_t TX = 3;                  // Transmit Pin
constexpr uint16_t RX = 2;                  // Recieve Pin
constexpr uint16_t TouchSensorPin = 9;      // Pin that recieves touch signal from sensor to enable power
constexpr uint16_t LowPowerPin = 66;        // Pin to put sensor in low power mode
constexpr uint16_t ServoControlPin = 7;     // Servo control pin
constexpr uint16_t LinActControlPin = 66;   // Linear actuator control pin
constexpr uint16_t RedLED = 6;              // Pin to Red LED
constexpr uint16_t GreenLED = 7;            // Pin to Green LED
constexpr uint16_t LockButton = 4;          // Manual lock button


/*class Sensor {
    uint64_t matchStartTime;
    uint64_t matchEndTime;
    bool check;
    uint32_t currentState;
    bool lockState;
};*/


extern unsigned long starttime;
extern unsigned long endtime;