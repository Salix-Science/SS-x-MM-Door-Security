// Meow. I tried making things as clear as possible (because im just locked in).
//
// This should be the standard file the arduino will run on day to day to access
// the room. 
//
// Salix Science & Maddy's Mischief © 2025

// Configure pins
#define RX                      2 //Recieve Pin
#define TX                      3 //Transmit Pin
#define TouchSensorPin          9 //Pin that recieves touch signal from sensor to enable power
#define LowPowerPin             66 //Pin to put sensor in low power mode
#define ServoControlPin         5 //Servo control pin
#define LinActControlPin        66 //Linear actuator control pin
#define RedLED                  6 //Pin to Red LED
#define GreenLED                7 //Pin to Green LED
#define LockButton              4 //Manual lock button

unsigned long starttime;
unsigned long endtime;
bool initialize;
bool check;
int currentstate;
bool lockstate;

#include <Arduino.h>
#include <Servo.h>
#include <SparkFun_AS108M_Arduino_Library.h>
#include <SparkFun_AS108M_Constants.h>
#include <Willowlib.h>




void setup() {
  Serial.begin(57600);
  fingerSerial.begin(57600); // Starts serial pins at standard sensor baudrate
  handleMotor.attach(ServoControlPin); //Connects motor object and associates with pin
  lockMotor.attach(LinActControlPin);
  pinMode(TouchSensorPin, INPUT);
  pinMode(LowPowerPin, OUTPUT);
  pinMode(ServoControlPin, OUTPUT);
  pinMode(LinActControlPin, OUTPUT);
  pinMode(RedLED, OUTPUT);
  pinMode(GreenLED, OUTPUT);
  LockDoor();
  lockstate = true;
  Serial.println("=== Willow & Maddy's Fingerprint Security System  ===");
  digitalWrite(RedLED, HIGH);
  digitalWrite(GreenLED, LOW);
}

void loop() {

  // detect whnen finger activates sensor
  if(!digitalRead(TouchSensorPin)){ 
    initialize = true;
  }

// Restart arduino clock for counting elapsed time
  if(initialize == true){ 
    noInterrupts ();
    timer0_millis = 0;
    interrupts ();
    starttime = millis();
    digitalWrite(LowPowerPin, HIGH);
  }

  // While fingerprint sensor is on, come to one of three conclusions
  while(initialize == true){
    check = FingerSearch(); 
    if (check == 0){
      digitalWrite(LowPowerPin, LOW); // Sensor returned to low power mode
      Serial.println("Fingerprint Accepted");
      UnlockDoor();
      lockstate = false;
      delay(10000); // Wait 10 seconds before locking the door for person to walk in
      initialize = false;
    }else if(check == 1){
      Serial.println("Fingerprint Denied");
      digitalWrite(LowPowerPin, LOW);
      initialize = false;
    }else if(endtime - starttime >= 7000){
      digitalWrite(LowPowerPin, LOW);
      Serial.println("Timeout");
      initialize = false;
    }

  }
  // Check to see if button is pressed
  currentstate = digitalRead(LockButton);

  // Make sure door isn't locked before attempting to lock door
  if(currentstate == HIGH && lockstate == false){
    LockDoor();
    lockstate = true;
  }

  // Ensure LEDs updated
  if(lockstate == true){
    digitalWrite(RedLED, HIGH);
    digitalWrite(GreenLED, LOW);
  }else{
    digitalWrite(RedLED, LOW);
    digitalWrite(GreenLED, HIGH);
  }
}
