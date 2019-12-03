/*
   This code is for running the stepper motors and GFP blue_light board for the Braingeneers PiCroscope Project


 */

#define DEBUG
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"
#include "EEPROM.h"

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_MotorShield LightController = Adafruit_MotorShield(0x61);

//L -> R
//Yellow, Orange, NC, Brown, Redaaaaa
Adafruit_StepperMotor * myMotor1 = AFMS.getStepper(200,  1);
Adafruit_StepperMotor *myMotor2 = AFMS.getStepper(200,  2);
Adafruit_DCMotor *blue_light = LightController.getMotor(1);
Adafruit_DCMotor *white_light = LightController.getMotor(2);


byte index = 0;
int address = 0;
int ledPin = 3;

int step_count[2] = {0, 0};
int curMotorPosition = EEPROM.read(0);
int newMotorPosition = EEPROM.read(0);
//int curMotorPosition = 0;
//int newMotorPosition = 0;
int stepsToTake = 0;

//Read limit switch
boolean read_switch(int lim_switch) {
    if(lim_switch==0)
    {
      return digitalRead(11);
    }
    else if(lim_switch==1)
    {
      //quick hack
      return true;
      //return digitalRead(12);
    }
}

void return_to_start(){
  while(read_switch(0)==1 && curMotorPosition > -3000){ //safety in case of limit switch failure
    myMotor1->onestep(BACKWARD, INTERLEAVE);
    myMotor2->onestep(BACKWARD, INTERLEAVE);
    curMotorPosition--;
  }
  //correct for mechanical hysteresis in limit switch
  //read_switch(1) ensures no collision with tissue culture
  while(read_switch(0)==0 && read_switch(1)==1){
    myMotor1->onestep(FORWARD, INTERLEAVE);
    myMotor2->onestep(FORWARD, INTERLEAVE);
    curMotorPosition++;
  }
}


void setup() {
        Serial.begin(115200);     // set up Serial library at 9600 bps

        AFMS.begin(); // create with the default frequency 1.6KHz
        LightController.begin();

        myMotor1->setSpeed(200); // 10 rpm
        myMotor2->setSpeed(200);

        pinMode(2, OUTPUT);
        digitalWrite(2, HIGH);

        //Set limit switch pin as input
        //input INPUT_PULLUP not for use with Pat's board
        pinMode(11, INPUT_PULLUP);
        pinMode(12, INPUT);
        //for blue_lights
         // blue_light->setSpeed(100);
         // blue_light->run(FORWARD);
         // // turn on motor
         // blue_light->run(RELEASE);
}
int val = -1;
char a = 'n';
char b = 'n';

void loop() {
        if (Serial.available() >= 2) {
                a = Serial.read();
                //for fast GFP blue_light response act here to avoid processing delays

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
        // Serial.print("switch zero: ");Serial.print(read_switch(0));
        // Serial.print(" switch one: "); Serial.print(read_switch(1)); Serial.println();
        // delay(500);
        if ( a == 'c') {
                //Calibration case
                curMotorPosition = 0;
                newMotorPosition = 0;
                EEPROM.update(address, 0);
        }
        if ( a == 'r') {
                //return to origin based on limit switch
                return_to_start();
                curMotorPosition = 0;
                newMotorPosition = 0;
                EEPROM.update(address, 0);
                a = 'n';
        }
        if ( a == 'm') {
                //    if ((val > -1000) && (val < 1000)) safety
                newMotorPosition = val;
                //save the new motor position into EEPROM
                EEPROM.update(address, val);
        }
        stepsToTake = newMotorPosition - curMotorPosition;
        if ( stepsToTake > 0) {
                if(read_switch(1)==1){//stop collision with cell plate
                  myMotor1->onestep(FORWARD, INTERLEAVE );
                  myMotor2->onestep(FORWARD, INTERLEAVE);
                  curMotorPosition++;
                }else{
                  newMotorPosition = curMotorPosition;
                  stepsToTake = 0;
                }
        }
        else if ( stepsToTake < 0) {
                myMotor1->onestep(BACKWARD, INTERLEAVE);
                myMotor2->onestep(BACKWARD, INTERLEAVE);
                curMotorPosition--;
        }
        else {
                myMotor1->release();
                myMotor2->release();
        }

        if (a == 'l') {
                if (val >= 0 && val <= 255) {
                        //analogWrite(ledPin, val);
                        blue_light->setSpeed(val);
                        blue_light->run(FORWARD);
                }
                else{
                        blue_light->setSpeed(0);
                        blue_light->run(RELEASE);
                }
        }
        if (a == 'w') {
                if (val >= 0 && val <= 255) {
                        //analogWrite(ledPin, val);
                        white_light->setSpeed(val);
                        white_light->run(FORWARD);
                }
                else{
                        white_light->setSpeed(0);
                        white_light->run(RELEASE);
                }
        }


}
