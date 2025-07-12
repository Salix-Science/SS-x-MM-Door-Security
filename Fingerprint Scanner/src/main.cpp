// Configure pins
#define RX                      2 //Recieve Pin
#define TX                      3 //Transmit Pin
#define TouchSensorPin          4 //Pin that recieves touch signal from sensor to enable power
#define LowPowerPin             5 //Pin to put sensor in low power mode
#define ServoControlPin         6 //Servo control pin
#define LinActControlPin        7 //Linear actuator control pin
#define RedLED                  8 //Pin to Red LED
#define GreenLED                9 //Pin to Green LED
#define LockButton              10 //Manual lock button

#include <Arduino.h>
#include <Servo.h>
#include <SparkFun_AS108M_Arduino_Library.h>
#include <SparkFun_AS108M_Constants.h>
#include <Willowlib.h>

unsigned long starttime;
bool initialize;
int touchread;
bool check;
int currentstate;
int laststate;

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
  Serial.println("=== Willow & Maddy's Fingerprint Security System  ===");
}

void loop() {
  touchread = digitalRead(TouchSensorPin);
  if(touchread == HIGH){ //detect whnen finger activates sensor
    initialize = true;
  }
  if(initialize == true){ //Restart arduino clock for counting elapsed time
    noInterrupts ();
    timer0_millis = 0;
    interrupts ();
    starttime = millis();
    digitalWrite(LowPowerPin, HIGH);
  }
  while(initialize == true){
    check = FingerSearch() 
    if (check == 0){
      digitalWrite(LowPowerPin, LOW); // sensor returned to low power mode
      Serial.println("Fingerprint Accepted");
      UnlockDoor();
      delay(10000); //Wait 10 seconds before locking the door
      LockDoor();
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
  currentstate = digitalRead(LockButton);
  if(currentstate == HIGH && laststate == LOW){
    LockDoor();
  }
  laststate = currentstate;
}
