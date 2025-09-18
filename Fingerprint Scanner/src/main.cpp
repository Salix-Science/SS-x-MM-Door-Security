
// Meow. I tried making things as clear as possible (because im just locked in).
//
// This should be the standard file the arduino will run on day to day to access
// the room.
//
// Salix Science & Maddy's Mischief © 2025

#include <Arduino.h>
#include <Servo.h>
#include <SparkFun_AS108M_Arduino_Library.h>
#include <SparkFun_AS108M_Constants.h>
#include <Willowlib.h>

Servo handleMotor;
Servo lockMotor;

void UnlockDoor() {
  handleMotor.write(75);
  delay(3000);
  handleMotor.write(30);
  delay(3000);
  Serial.println("Door Unlocked");
}

void LockDoor() {
  lockMotor.write(180);
  delay(2000);
  lockMotor.write(0);
  Serial.println("Locked");
}

void setup() {
  Serial.begin(57600);
  fingerSerial.begin(57600); // Starts serial pins at standard sensor baudrate
  handleMotor.attach(ServoControlPin); // Connects motor object and associates with pin
  lockMotor.attach(LinActControlPin);
  pinMode(TouchSensorPin, INPUT);
  pinMode(LowPowerPin, OUTPUT);
  pinMode(ServoControlPin, OUTPUT);
  pinMode(LinActControlPin, OUTPUT);
  pinMode(RedLED, OUTPUT);
  pinMode(GreenLED, OUTPUT);
  Serial.println("=== Willow & Maddy's Fingerprint Security System  ===");
  digitalWrite(RedLED, HIGH);
  digitalWrite(GreenLED, LOW);
  handleMotor.write(30);
}

void loop() {

  // Restart arduino clock for counting elapsed time

  noInterrupts();
  timer0_millis = 0;
  interrupts();
  starttime = millis();
  digitalWrite(LowPowerPin, HIGH);
  
  // While fingerprint sensor is on, come to one of three conclusions
  MatchResult check = FingerSearch();

  if (check == MatchResult::Accepted) {
    digitalWrite(LowPowerPin, LOW); // Sensor returned to low power mode
    Serial.println("Fingerprint Accepted");
    UnlockDoor();
    delay(10000); // Wait 10 seconds before locking the door for person to walk in
  } else if (check == MatchResult::Denied) {
    Serial.println("Fingerprint Denied");
    digitalWrite(LowPowerPin, LOW);
  } else if (check == MatchResult::Error) {
    Serial.println("Error when trying to match fingerprint");
  } else if (check == MatchResult::Timeout) {
    Serial.println("Fingerprint search timed out");
  } else if (endtime - starttime >= 7000) {
    digitalWrite(LowPowerPin, LOW);
    Serial.println("Timeout");
  }
  
  /*
  // Check to see if button is pressed
  currentstate = digitalRead(LockButton);

  // Make sure door isn't locked before attempting to lock door
  if (currentstate == HIGH && lockstate == false) {
    LockDoor();
    lockstate = true;
  }

  // Ensure LEDs updated
  if (lockstate == true) {
    digitalWrite(RedLED, HIGH);
    digitalWrite(GreenLED, LOW);
  } else {
    digitalWrite(RedLED, LOW);
    digitalWrite(GreenLED, HIGH);
  }*/
  
  
}
