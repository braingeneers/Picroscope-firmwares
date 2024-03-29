/*
  This code is for running the stepper motors and GFP light board for the Braingeneers PiCroscope Project


*/

#define DEBUG
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"
#include "EEPROM.h"

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
//Adafruit_MotorShield LightController = Adafruit_MotorShield(0x61);

//L -> R
//Yellow, Orange, NC, Brown, Redaaaaa
Adafruit_StepperMotor * myMotor1 = AFMS.getStepper(200,  1);
Adafruit_StepperMotor *myMotor2 = AFMS.getStepper(200,  2);
//Adafruit_DCMotor *light = LightController.getMotor(1);


byte index = 0;
int address = 0;
int step_count[2] = {0, 0};
int curMotorPosition = EEPROM.read(0);
int newMotorPosition = EEPROM.read(0);
//int curMotorPosition = 0;
//int newMotorPosition = 0;
int stepsToTake = 0;


void setup() {
  Serial.begin(115200);           // set up Serial library at 9600 bps

  AFMS.begin();  // create with the default frequency 1.6KHz
  //  LightController.begin();

  myMotor1->setSpeed(200);  // 10 rpm
  myMotor2->setSpeed(200);

  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  //for lights
  //  light->setSpeed(100);
  //  light->run(FORWARD);
  //  // turn on motor
  //  light->run(RELEASE);
}
int val = -1;
char a = 'n';
char b = 'n';

void loop() {
  if (Serial.available() >= 2) {
    a = Serial.read();
    //for fast GFP light response act here to avoid processing delays
    if(a == 'l') digitalWrite(2,LOW);
    else if(a == 'o') digitalWrite(2,HIGH);
    b = Serial.read();
    val = Serial.parseInt();
    //    //flush
    //    while(Serial.available) Serial.read();
#ifdef DEBUG
    Serial.println(a);
    Serial.println(b);
    Serial.println(val, DEC);
#endif
  }

  if ( a == 'c') {
    //Calibration case
    curMotorPosition = 0;
    newMotorPosition = 0;
    EEPROM.update(address, 0);
  }
  if ( a == 'm') {
    //    if ((val > -1000) && (val < 1000)) safety
    newMotorPosition = val;
    //save the new motor position into EEPROM
    EEPROM.update(address, val);
  }
  stepsToTake = newMotorPosition - curMotorPosition;
  if ( stepsToTake > 0) {
    myMotor1->step(1, FORWARD, SINGLE);
    myMotor2->step(1, FORWARD, SINGLE);
    curMotorPosition++;
  }
  else if ( stepsToTake < 0) {
    myMotor1->step(1, BACKWARD, SINGLE);
    myMotor2->step(1, BACKWARD, SINGLE);
    curMotorPosition--;
  }
  else {
    myMotor1->release();
    myMotor2->release();
  }

  //light control board
//  if (a == 'l') {
//    a = 'n';
//#ifdef DEBUG
//    Serial.println("boof");
//#endif
//    if (val > 0 && val <= 255) {
//      digitalWrite(2, LOW);
//    }
//    else {
//      digitalWrite(2, HIGH);
//    }
//  }

}
