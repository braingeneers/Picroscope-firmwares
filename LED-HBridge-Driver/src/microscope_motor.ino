/*
   This code is for running the stepper motors and GFP light board for the Braingeneers PiCroscope Project


 */

#define DEBUG
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"
#include "EEPROM.h"

// Create the motor shield object
Adafruit_MotorShield LightController = Adafruit_MotorShield(0x61);

Adafruit_DCMotor *light = LightController.getMotor(1);


byte index = 0;
int address = 0;
int ledPin = 3;


void setup() {
        Serial.begin(115200);     // set up Serial library at 9600 bps

        LightController.begin(1600000);
        pinMode(ledPin, OUTPUT);

        //for lights
        light->setSpeed(100);
        light->run(FORWARD);
        //light->run(RELEASE);
}
int val = -1;
char a = 'n';
char b = 'n';

void loop() {
        if (Serial.available() >= 2) {
                a = Serial.read();
                //for fast GFP light response act here to avoid processing delays
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

//light control board
        if (a == 'l') {
                if (val >= 0 && val <= 255) {
                        analogWrite(ledPin, val);
                        // light->setSpeed(val);
                        // light->run(FORWARD);
                }
                else{
                        // light->setSpeed(0);
                        // light->run(RELEASE);
                }
        }
//    if (val > 0 && val <= 255) {
//      digitalWrite(2, LOW);
//    }
//    else {
//      digitalWrite(2, HIGH);
//    }
//  }

}
