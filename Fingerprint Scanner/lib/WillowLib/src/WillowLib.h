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

extern Servo handleMotor;
extern Servo lockMotor;
extern SoftwareSerial fingerSerial; // RX, TX

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
};

void ShowIndexTable();
void UnlockDoor();
void LockDoor();
DetectionResult WaitFingerPlaced();
bool FingerStatus();
uint8_t PSGetImage();
bool ReceivePacket(uint8_t length);
void FingerEnroll(uint16_t uID);
MatchResult FingerSearch();
uint8_t PSSearch(uint16_t *matchedID, uint16_t *score);
uint8_t PSGenChar(uint8_t bufID);
uint8_t PSRegModule();
uint8_t PSStoreChar(uint16_t uID);
extern volatile unsigned long timer0_millis;
