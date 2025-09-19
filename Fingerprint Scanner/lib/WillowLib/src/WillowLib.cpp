#include "WillowLib.h"

SoftwareSerial fingerSerial(RX, TX); // RX, TX

uint8_t packetBuffer[20];

// Enrollment configuration
constexpr uint8_t g_max_samples = 2; // Usually 2 samples needed

FingerprintSensor::FingerprintSensor(uint16_t transmitPin, uint16_t recievePin) : fingerSerial(TX, RX), startTime(0), endTime(0) {

}

void FingerprintSensor::begin(uint64_t baud) {
  fingerSerial.begin(baud);
}

void FingerprintSensor::resetTimer() {
  startTime = millis();
}

bool FingerprintSensor::hasElapsed(uint64_t duration) {
  return endTime - startTime >= duration;
}

void FingerprintSensor::showIndexTable() {
  Serial.println("Requesting index table...");
  // Not implemented here
}

// Wait for finger placement
DetectionResult FingerprintSensor::waitFingerPlaced() {
  Serial.println("Place your finger on the sensor...");
  while (true) {
    if (fingerStatus()) {
      Serial.println("Finger detected.");
      delay(500); // Wait for finger stabilization
      return DetectionResult::Detected;
    }
    delay(100);
    endTime = millis();
    if (endTime - startTime >= 7000) {
      return DetectionResult::Timeout;
    }
  }
}

// Check finger presence
bool FingerprintSensor::fingerStatus() { return psGetImage() != PS_NO_FINGER; }

// Send GetImage command
uint8_t FingerprintSensor::psGetImage() {
  uint8_t cmd[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x01, 0x00, 0x05};
  fingerSerial.write(cmd, sizeof(cmd));

  if (!receivePacket(12))
    return PS_COMM_ERR;

  return packetBuffer[9];
}

// Receive bytes into packetBuffer
bool FingerprintSensor::receivePacket(uint8_t length) {
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
void FingerprintSensor::fingerEnroll(uint16_t uID) {
  for (uint8_t i = 0; i < g_max_samples; i++) {
    Serial.print("Sample ");
    Serial.println(i + 1);
    waitFingerPlaced();

    uint8_t res = psGenChar(i + 1);
    if (res != PS_OK) {
      Serial.print("GenChar failed, error code: 0x");
      Serial.println(res, HEX);
      return;
    }

    Serial.println("Remove finger...");
    while (fingerStatus())
      delay(100);
  }

  if (psRegModule() != PS_OK) {
    Serial.println("RegModule failed.");
    return;
  }

  if (psStoreChar(uID) != PS_OK) {
    Serial.println("StoreChar failed.");
    return;
  }

  Serial.println("Enrollment complete!");
}

MatchResult FingerprintSensor::fingerSearch() {
  Serial.println("Place finger on the scanner.");
  DetectionResult x = waitFingerPlaced();

  if (x == DetectionResult::Timeout) {
    return MatchResult::Timeout;
  }
  
  uint8_t res = psGenChar(1);

  if (res != PS_OK) {
    Serial.print("GenChar failed during search, error code: 0x");
    Serial.println(res, HEX);
    return MatchResult::Error;
  }

  uint16_t matchedID, score;
  res = psSearch(&matchedID, &score);

  if (res == PS_OK) {
    Serial.print("Match found! ID=");
    Serial.print(matchedID);
    Serial.print(", Score=");
    Serial.println(score);
    return MatchResult::Accepted;
  } else if (res == 0x09) {
    Serial.println("No match found.");
    return MatchResult::Denied;
  } else {
    Serial.print("Search error: 0x");
    Serial.println(res, HEX);
    return MatchResult::Error;
  }
}

// Search command
uint8_t FingerprintSensor::psSearch(uint16_t *matchedID, uint16_t *score) {
  uint16_t startID = 0;
  uint16_t endID = 200; // Adjust according to your database size
  uint8_t cmd[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x08, 0x04, 0x01, (uint8_t)(startID >> 8), (uint8_t)(startID & 0xFF), (uint8_t)(endID >> 8), (uint8_t)(endID & 0xFF), 0x00, 0x00};
  uint16_t sum = 0;
  for (uint8_t i = 6; i < sizeof(cmd) - 2; i++)
    sum += cmd[i];
  cmd[sizeof(cmd) - 2] = (sum >> 8) & 0xFF;
  cmd[sizeof(cmd) - 1] = sum & 0xFF;

  fingerSerial.write(cmd, sizeof(cmd));

  if (!receivePacket(16))
    return PS_COMM_ERR;

  if (packetBuffer[9] != PS_OK)
    return packetBuffer[9];

  *matchedID = (packetBuffer[10] << 8) | packetBuffer[11];
  *score = (packetBuffer[12] << 8) | packetBuffer[13];

  return PS_OK;
}

// Generate character file from image (GenChar)
uint8_t FingerprintSensor::psGenChar(uint8_t bufID) {
  uint8_t cmd[] = {0xEF, 0x01, 0xFF, 0xFF,  0xFF, 0xFF, 0x01, 0x00, 0x04, 0x02, bufID, 0x00, 0x00};
  uint16_t sum = 0;
  for (uint8_t i = 6; i < 11; i++)
    sum += cmd[i];
  cmd[11] = (sum >> 8) & 0xFF;
  cmd[12] = sum & 0xFF;

  fingerSerial.write(cmd, sizeof(cmd));
  delay(150); // Wait for sensor to process

  if (!receivePacket(12))
    return PS_COMM_ERR;

  return packetBuffer[9];
}

// Create template from char files
uint8_t FingerprintSensor::psRegModule() {
  uint8_t cmd[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x05, 0x00, 0x09};
  fingerSerial.write(cmd, sizeof(cmd));

  if (!receivePacket(12))
    return PS_COMM_ERR;

  return packetBuffer[9];
}

// Store template into flash library
uint8_t FingerprintSensor::psStoreChar(uint16_t uID) {
  uint8_t cmd[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x06, 0x06, 0x01, (uint8_t)(uID >> 8), (uint8_t)(uID & 0xFF), 0x00, 0x00};
  uint16_t sum = 0;
  for (uint8_t i = 6; i < 13; i++)
    sum += cmd[i];
  cmd[13] = (sum >> 8) & 0xFF;
  cmd[14] = sum & 0xFF;

  fingerSerial.write(cmd, sizeof(cmd));

  if (!receivePacket(12))
    return PS_COMM_ERR;

  return packetBuffer[9];
}