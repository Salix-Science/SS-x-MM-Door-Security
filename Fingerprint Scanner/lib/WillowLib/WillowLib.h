#pragma once
#include <stdint.h>
#include <Servo.h>
#include <SparkFun_AS108M_Arduino_Library.h>
#include <SparkFun_AS108M_Constants.h>
#include <SoftwareSerial.h>

// Fingerprint sensor constants 
constexpr uint32_t PS_OK = 0x00;
constexpr uint32_t PS_NO_FINGER = 0x02;
constexpr uint32_t PS_COMM_ERR = 0x01;

Servo handleMotor;
Servo lockMotor;
SoftwareSerial fingerSerial(RX, TX); // RX, TX

void ShowIndexTable();
void UnlockDoor();
void LockDoor();
int WaitFingerPlaced();
bool FingerStatus();
uint8_t PSGetImage();
bool ReceivePacket(uint8_t length);
void FingerEnroll(uint16_t uID);
int FingerSearch();
uint8_t PSSearch(uint16_t *matchedID, uint16_t *score);
uint8_t PSGenChar(uint8_t bufID);
uint8_t PSRegModule();
uint8_t PSStoreChar(uint16_t uID);
extern volatile unsigned long timer0_millis;

// Buffers
uint8_t packetBuffer[20];

// Enrollment configuration
const uint8_t g_max_samples = 2; // Usually 2 samples needed


void ShowIndexTable() {
  Serial.println("Requesting index table...");
  // Not implemented here
}

// Wait for finger placement
int WaitFingerPlaced() {
  Serial.println("Place your finger on the sensor...");
  while (true) {
    if (FingerStatus()) {
      Serial.println("Finger detected.");
      delay(500); // Wait for finger stabilization
      return 0;
    }
    delay(100);
    endtime = millis();
    if(endtime - starttime >= 7000){
      return 1;
    }
  }
}

void UnlockDoor() {
    handleMotor.write(45);
    delay(2000);
    handleMotor.write(90);
  Serial.println("Door Unlocked");
}

void LockDoor(){
  lockMotor.write(180);
  delay(2000);
  lockMotor.write(0);
  Serial.println("Locked");
}

// Check finger presence
bool FingerStatus() {
  return PSGetImage() != PS_NO_FINGER;
}

// Send GetImage command
uint8_t PSGetImage() {
  uint8_t cmd[] = {
    0xEF,0x01, 0xFF,0xFF,0xFF,0xFF, 0x01,
    0x00,0x03,
    0x01,
    0x00,0x05
  };
  fingerSerial.write(cmd, sizeof(cmd));

  if (!ReceivePacket(12))
    return PS_COMM_ERR;

  return packetBuffer[9];
}

// Receive bytes into packetBuffer
bool ReceivePacket(uint8_t length) {
  unsigned long start = millis();
  uint8_t idx = 0;
  while (idx < length) {
    if (fingerSerial.available()) {
      packetBuffer[idx++] = fingerSerial.read();
    } else if (millis() - start > 2000) {
      Serial.println("Timeout waiting for packet");
      return false;
    }
  }
  return true;
}

// Enroll fingerprint for given ID
void FingerEnroll(uint16_t uID) {
  for (uint8_t i = 0; i < g_max_samples; i++) {
    Serial.print("Sample "); Serial.println(i + 1);
    WaitFingerPlaced();

    uint8_t res = PSGenChar(i + 1);
    if (res != PS_OK) {
      Serial.print("GenChar failed, error code: 0x");
      Serial.println(res, HEX);
      return;
    }

    Serial.println("Remove finger...");
    while (FingerStatus()) delay(100);
  }

  if (PSRegModule() != PS_OK) {
    Serial.println("RegModule failed.");
    return;
  }

  if (PSStoreChar(uID) != PS_OK) {
    Serial.println("StoreChar failed.");
    return;
  }

  Serial.println("Enrollment complete!");
}


int FingerSearch() {
  Serial.println("Place finger on the scanner.");
  int x = WaitFingerPlaced();
  if(x == 1){
    return 2;
  }
  uint8_t res = PSGenChar(1);
  if (res != PS_OK) {
    Serial.print("GenChar failed during search, error code: 0x");
    Serial.println(res, HEX);
    return 1;
  }

  uint16_t matchedID, score;
  res = PSSearch(&matchedID, &score);

  if (res == PS_OK) {
    Serial.print("Match found! ID=");
    Serial.print(matchedID);
    Serial.print(", Score=");
    Serial.println(score);
    return 0;
  } else if (res == 0x09) {
    Serial.println("No match found.");
    return 1;
  } else {
    Serial.print("Search error: 0x");
    Serial.println(res, HEX);
    return 1;
  }
}

// Search command
uint8_t PSSearch(uint16_t *matchedID, uint16_t *score) {
  uint16_t startID = 0;
  uint16_t endID = 200; // Adjust according to your database size
  uint8_t cmd[] = {
    0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,
    0x00,0x08,
    0x04,0x01,
    (uint8_t)(startID >> 8), (uint8_t)(startID & 0xFF),
    (uint8_t)(endID >> 8), (uint8_t)(endID & 0xFF),
    0x00,0x00
  };
  uint16_t sum = 0;
  for (uint8_t i = 6; i < sizeof(cmd) - 2; i++) sum += cmd[i];
  cmd[sizeof(cmd) - 2] = (sum >> 8) & 0xFF;
  cmd[sizeof(cmd) - 1] = sum & 0xFF;

  fingerSerial.write(cmd, sizeof(cmd));

  if (!ReceivePacket(16)) return PS_COMM_ERR;

  if (packetBuffer[9] != PS_OK) return packetBuffer[9];

  *matchedID = (packetBuffer[10] << 8) | packetBuffer[11];
  *score     = (packetBuffer[12] << 8) | packetBuffer[13];

  return PS_OK;
}

// Generate character file from image (GenChar)
uint8_t PSGenChar(uint8_t bufID) {
  uint8_t cmd[] = {
    0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,
    0x00,0x04,
    0x02, bufID,
    0x00,0x00
  };
  uint16_t sum = 0;
  for (uint8_t i = 6; i < 11; i++) sum += cmd[i];
  cmd[11] = (sum >> 8) & 0xFF;
  cmd[12] = sum & 0xFF;

  fingerSerial.write(cmd, sizeof(cmd));
  delay(150); // Wait for sensor to process

  if (!ReceivePacket(12)) return PS_COMM_ERR;

  return packetBuffer[9];
}

// Create template from char files
uint8_t PSRegModule() {
  uint8_t cmd[] = {
    0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,
    0x00,0x03,
    0x05,
    0x00,0x09
  };
  fingerSerial.write(cmd, sizeof(cmd));

  if (!ReceivePacket(12)) return PS_COMM_ERR;

  return packetBuffer[9];
}

// Store template into flash library
uint8_t PSStoreChar(uint16_t uID) {
  uint8_t cmd[] = {
    0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,
    0x00,0x06,
    0x06,0x01,
    (uint8_t)(uID >> 8), (uint8_t)(uID & 0xFF),
    0x00,0x00
  };
  uint16_t sum = 0;
  for (uint8_t i = 6; i < 13; i++) sum += cmd[i];
  cmd[13] = (sum >> 8) & 0xFF;
  cmd[14] = sum & 0xFF;

  fingerSerial.write(cmd, sizeof(cmd));

  if (!ReceivePacket(12)) return PS_COMM_ERR;

  return packetBuffer[9];
}