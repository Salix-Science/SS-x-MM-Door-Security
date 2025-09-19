#pragma once

#include <Arduino.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include <stdint.h>
#include "globals.h"

// Fingerprint sensor constants
constexpr uint32_t PS_OK = 0x00;
constexpr uint32_t PS_NO_FINGER = 0x02;
constexpr uint32_t PS_COMM_ERR = 0x01;

enum class DetectionResult {
    Detected,
    Timeout
};

enum class MatchResult {
    Accepted,
    Denied,
    Error,
    Timeout
};

class FingerprintSensor {
    SoftwareSerial fingerSerial; 
    uint8_t packetBuffer[20];
    uint64_t startTime;
    uint64_t endTime;
public:
    FingerprintSensor(uint16_t transmitPin, uint16_t recievePin);

    void begin(uint64_t baud);
    void showIndexTable();
    DetectionResult waitFingerPlaced();
    bool fingerStatus();
    uint8_t psGetImage();
    bool receivePacket(uint8_t length);
    void fingerEnroll(uint16_t uID);
    MatchResult fingerSearch();
    uint8_t psSearch(uint16_t *matchedID, uint16_t *score);
    uint8_t psGenChar(uint8_t bufID);
    uint8_t psRegModule();
    uint8_t psStoreChar(uint16_t uID);
};