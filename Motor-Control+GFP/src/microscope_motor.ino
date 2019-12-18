/*
   This code is for running the stepper motors and GFP blue_light board for the Braingeneers PiCroscope Project


 */

#define DEBUG
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"
#include "EEPROM.h"

#define SWITCH_2_PIN 7
#define BLUE_LED_PIN 2
#define WHITE_LED_PIN 3
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
        if(lim_switch==2)
        {
                return digitalRead(SWITCH_2_PIN);
        }
        else if(lim_switch==1)
        {
                //quick hack
                return true;
                //return digitalRead(12);
        }
}

void return_to_start(){
        while(digitalRead(SWITCH_2_PIN) && curMotorPosition > -5000) { //safety in case of limit switch failure
                myMotor1->onestep(BACKWARD, INTERLEAVE);
                myMotor2->onestep(BACKWARD, INTERLEAVE);
                curMotorPosition--;
        }
        //correct for mechanical hysteresis in limit switch
        //read_switch(1) ensures no collision with tissue culture
        while(digitalRead(SWITCH_2_PIN)==0) {
                myMotor1->onestep(FORWARD, INTERLEAVE);
                myMotor2->onestep(FORWARD, INTERLEAVE);
                curMotorPosition++;
        }
}

void return_to_start_step();




void setup() {
        Serial.begin(115200);     // set up Serial library at 9600 bps
        //Serial.println("starting: ");
        AFMS.begin(); // create with the default frequency 1.6KHz
        LightController.begin();

        myMotor1->setSpeed(200); // 10 rpm
        myMotor2->setSpeed(200);

        pinMode(BLUE_LED_PIN, OUTPUT);
        pinMode(WHITE_LED_PIN, OUTPUT);
        pinMode(4, OUTPUT);
        digitalWrite(BLUE_LED_PIN, LOW);
        digitalWrite(WHITE_LED_PIN, LOW);
        //Set limit switch pin as input
        //input INPUT_PULLUP not for use with Pat's board
        pinMode(SWITCH_2_PIN, INPUT);
        //for blue_lights
        // blue_light->setSpeed(100);
        // blue_light->run(FORWARD);
        // // turn on motor
        // blue_light->run(RELEASE);
}
int val = -1;
char a = 'n';
char b = 'n';
bool return_flag = false;

void loop() {
        //Serial.println("running: ");
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
                return_flag = true;
                //return_to_start();
                //curMotorPosition = 0;
                //newMotorPosition = 0;
                //EEPROM.update(address, 0);
                a = 'n';
        }
        if ( a == 'm') {
                //    if ((val > -1000) && (val < 1000)) safety
                newMotorPosition = val;
                //save the new motor position into EEPROM
                EEPROM.update(address, val);
        }
        if(return_flag) {
                return_to_start_step();
        }
        else{
                stepsToTake = newMotorPosition - curMotorPosition;
                if ( stepsToTake > 0) {
                        if(read_switch(1)==1) {//stop collision with cell plate
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
        }
        if (a == 'l') {
                if (val == 0) {
                        //analogWrite(ledPin, val);
                        digitalWrite(BLUE_LED_PIN, LOW);
                        digitalWrite(WHITE_LED_PIN, LOW);
                        //blue_light->setSpeed(val);
                        //blue_light->run(FORWARD);
                }
                if ( val == 1)
                        digitalWrite(BLUE_LED_PIN, HIGH);
                if( val == 2)
                        digitalWrite(WHITE_LED_PIN, HIGH);
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


void return_to_start_step(){
        //this is a mess to make the motor control non-blocking, must make into a state machine at a later date
        enum state {DOWN, UP, EXIT, ERROR};
        static int state = DOWN;
        switch (state)
        {
        case DOWN:
                if(digitalRead(SWITCH_2_PIN) == 1) { //safety in case of limit switch failure
                        myMotor1->onestep(BACKWARD, INTERLEAVE);
                        myMotor2->onestep(BACKWARD, INTERLEAVE);
                        curMotorPosition--;
                        if(curMotorPosition < -5000) {
                                state = ERROR;//error condition
                        }
                }
                else{
                        state = UP;
                }
                break;
        case UP:
                if(digitalRead(SWITCH_2_PIN)==0) {
                        myMotor1->onestep(FORWARD, INTERLEAVE);
                        myMotor2->onestep(FORWARD, INTERLEAVE);
                        curMotorPosition++;
                }
                else{
                        state = EXIT;
                }

                break;
        case EXIT:
                return_flag = false;
                curMotorPosition = 0;
                newMotorPosition = 0;
                EEPROM.update(address, 0);
                state = DOWN;
                break;
        case ERROR:
                return_flag = false;
                curMotorPosition = 0;
                newMotorPosition = 0;
                EEPROM.update(address, 0);
                state = DOWN;
                break;

        }
        //correct for mechanical hysteresis in limit switch
        //read_switch(1) ensures no collision with tissue culture
        // while(digitalRead(SWITCH_2_PIN)==0) {
        //         myMotor1->onestep(FORWARD, INTERLEAVE);
        //         myMotor2->onestep(FORWARD, INTERLEAVE);
        //         curMotorPosition++;
        // }
}
