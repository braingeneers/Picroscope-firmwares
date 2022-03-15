/*
   This code is for running the stepper motors and GFP blue_light board for the Braingeneers PiCroscope Project


 */

#define DEBUG
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"
#include "EEPROM.h"
#include "DHT.h"
#include <Arduino.h>
#include <SPI.h>
#include <pins_arduino.h>
#include <avr/interrupt.h>

#define SWITCH_2_PIN 6 //changed from pin 7
#define SWITCH_X_PIN A0
#define SWITCH_Y_PIN A1

#define BLUE_LED_PIN 5
#define WHITE_LED_PIN 3
#define SAFE_SWITCH_PIN 2
#define MOTOR_SAFETY_PIN 11
#define DHTPIN 8
//#define ACTIVE_LOW

#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);
// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x60);
Adafruit_MotorShield XY_Stage = Adafruit_MotorShield(0x61);

//L -> R
//Yellow, Orange, NC, Brown, Redaaaaa
Adafruit_StepperMotor *myMotor1 = AFMS.getStepper(200,  1);
Adafruit_StepperMotor *myMotor2 = AFMS.getStepper(200,  2);

Adafruit_StepperMotor *xTranslation = XY_Stage.getStepper(200,  1);
Adafruit_StepperMotor *yTranslation = XY_Stage.getStepper(200,  2);
//Adafruit_DCMotor *blue_light = LightController.getMotor(1);
//Adafruit_DCMotor *white_light = LightController.getMotor(2);


byte index = 0;
int address = 0;
int ledPin = 3;

int step_count[2] = {0, 0};
int curMotorPosition = EEPROM.read(0);
int newMotorPosition = EEPROM.read(0);
//int curMotorPosition = 0;
//int newMotorPosition = 0;
int stepsToTake = 0;
int previousStepsToTake = 0;
double encoderStepsToTake = 0;
int xStepsToTake = 0;
int yStepsToTake = 0;


//CopyPated encoder code starts hereby


const byte encoderPinA = 9;
const byte encoderPinB = 10;//outoutB digital pin3
volatile int count = 0;
int safeMotorEncoderPositionA = 0;
int previousCount = 0;

const byte encoderPinC = 11;
const byte encoderPinD = 12;
volatile int count2 = 0;
int safeMotorEncoderPositionB = 0;
int previousCount2 = 0;


volatile uint8_t stateA = 0;
volatile uint8_t stateB = 0;
volatile uint8_t stateC = 0;
volatile uint8_t stateD = 0;

// void isrC();
// void isrD();

//#define readA bitRead(PINC, PC0)
#define readA bitRead(PINB, PB1) //D9
#define readB bitRead(PINB, PB2) //D10

uint8_t bus = B00000000;
volatile byte pinBStatus;

//#define readA PINB | PB1
//#define readB PINB | PB0

// #define readC digitalRead(2)
// #define readD digitalRead(3)
#define readC bitRead(PINB, PB4) //D12
#define readD bitRead(PINB, PB5) //D13
//end of encoder code blocked

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

void return_to_start(){//never use this blocking BS, must be a leftover
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
void encoder_setup(){
  PCICR |= B00000001;
  //PCMSK0 |= B00000011;


  //trying to put the other interrupt on
  PCMSK0 |= B00110110; //this corresponds to pins D8 and D11
  //PCMSK1 |= B00000001;

  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  pinMode(encoderPinC, INPUT_PULLUP);
  pinMode(encoderPinD, INPUT_PULLUP);

  stateA = readA;
  stateB = readB;

  stateC = readC;
  stateD = readD;
}


void return_to_start_step();




void setup() {
        Serial.begin(115200);     // set up Serial library at 9600 bps
        //Serial.println("starting: ");
        AFMS.begin(); // create with the default frequency 1.6KHz
        XY_Stage.begin();

        encoder_setup();

        myMotor1->setSpeed(200); // 10 rpm
        myMotor2->setSpeed(200);

        pinMode(BLUE_LED_PIN, OUTPUT);
        pinMode(WHITE_LED_PIN, OUTPUT);
        //pinMode(4, OUTPUT);
        pinMode(SAFE_SWITCH_PIN, OUTPUT);
        pinMode(MOTOR_SAFETY_PIN, OUTPUT);

        #ifdef ACTIVE_LOW
        digitalWrite(BLUE_LED_PIN, HIGH);
        digitalWrite(WHITE_LED_PIN, HIGH);
        digitalWrite(SAFE_SWITCH_PIN, LOW);
        #else
        digitalWrite(BLUE_LED_PIN, LOW);
        digitalWrite(WHITE_LED_PIN, LOW);
        digitalWrite(SAFE_SWITCH_PIN, HIGH);
        digitalWrite(MOTOR_SAFETY_PIN, HIGH); // we dont have this one on the active low board
        #endif

        //Set limit switch pin as input
        //input INPUT_PULLUP not for use with Pat's board
        pinMode(SWITCH_2_PIN, INPUT);
        //for blue_lights
        // blue_light->setSpeed(100);
        // blue_light->run(FORWARD);
        // // turn on motor
        // blue_light->run(RELEASE);
        dht.begin();
}
int val = -1;
char a = 'n';
char b = 'n';
bool return_flag = false;
float t = dht.readTemperature();
int temp_timer = millis();
int motor_timer = millis();
float highest_temp = 0;
float trigger_temp = 50;
bool catastrophe = false;

void loop() {

        noInterrupts();
        safeMotorEncoderPositionA = count;
        safeMotorEncoderPositionB = count2;
        interrupts();

        if (abs(millis() - temp_timer) > 2000 && return_flag == false && stepsToTake == 0) {
                //we're not reading temp while motors are trying to move add timers for motors
                t = dht.readTemperature();
                if (t > highest_temp) {
                        highest_temp = t;
                }
                if (t > trigger_temp && !isnan(t)) {
                        catastrophe = true;
              #ifdef ACTIVE_LOW
                        digitalWrite(SAFE_SWITCH_PIN, HIGH);
              #else
                        digitalWrite(SAFE_SWITCH_PIN, LOW);
                        digitalWrite(MOTOR_SAFETY_PIN, LOW);
                        digitalWrite(BLUE_LED_PIN, LOW);
                        digitalWrite(WHITE_LED_PIN, LOW);
              #endif
                }
        }
        //motor running too long
        else if (abs(millis() - motor_timer) > 10000 && (return_flag == true || stepsToTake == 0)) {
                catastrophe = true;
          #ifdef ACTIVE_LOW
                digitalWrite(SAFE_SWITCH_PIN, HIGH);
          #else
                digitalWrite(SAFE_SWITCH_PIN, LOW);
                digitalWrite(MOTOR_SAFETY_PIN, LOW);
                digitalWrite(BLUE_LED_PIN, LOW);
                digitalWrite(WHITE_LED_PIN, LOW);
          #endif
        }
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
        switch(a){
          // Serial.print("switch zero: ");Serial.print(read_switch(0));
          // Serial.print(" switch one: "); Serial.print(read_switch(1)); Serial.println();
          // delay(500);
          case 'e':
          //report encoder data
                  Serial.print("Motor A: ");
                  Serial.println(safeMotorEncoderPositionA);
                  Serial.print("Motor B: ");
                  Serial.println(safeMotorEncoderPositionB);          
                  break;
          case 'c' :
          //Calibration case
                  curMotorPosition = 0;
                  newMotorPosition = 0;
                  EEPROM.update(address, 0);
                  break;

          case 'r' :
          //return to origin based on limit switch
                  motor_timer = millis();
                  return_flag = true;
                  //return_to_start();
                  curMotorPosition = 0; //uncommented this to make ERROR condition work properly in return_to_start_step()
                  //newMotorPosition = 0;
                  //EEPROM.update(address, 0);
                  a = 'n';
                  break;
          case 'm' :
          //set new motor position
                  //    if ((val > -1000) && (val < 1000)) safety
                  newMotorPosition = val;
                  //save the new motor position into EEPROM
                  EEPROM.update(address, val);
                  break;
          case 'x' :
          //move x axis motors on XY stage
                  //    if ((val > -1000) && (val < 1000)) safety
                  xStepsToTake = val;
                  break;

          case 'y' :
          //move y axis motors on XY plate
                  //    if ((val > -1000) && (val < 1000)) safety
                  yStepsToTake = val;
                  break;

          case 'l' :
          //control lights
                  if (val == 0) {
                          //analogWrite(ledPin, val);
                          #ifdef ACTIVE_LOW
                          digitalWrite(BLUE_LED_PIN, HIGH);
                          digitalWrite(WHITE_LED_PIN, HIGH);
                          #else
                          digitalWrite(BLUE_LED_PIN, LOW);
                          digitalWrite(WHITE_LED_PIN, LOW);
                          #endif
                          //blue_light->setSpeed(val);
                          //blue_light->run(FORWARD);
                  }
                  #ifdef ACTIVE_LOW
                  if ( val == 1)
                          digitalWrite(BLUE_LED_PIN, LOW);
                  if( val == 2)
                          digitalWrite(WHITE_LED_PIN, LOW);
                  #else
                  if ( val == 1)
                          digitalWrite(BLUE_LED_PIN, HIGH);
                  if( val == 2)
                          digitalWrite(WHITE_LED_PIN, HIGH);
                  #endif
                  break;
          case 't' :
          //report temperature data
                  Serial.print(F("%  Temperature: "));
                  Serial.print(t);
                  Serial.print(F("°C "));
                  Serial.print(F("%  Peak Temperature: "));
                  Serial.print(highest_temp);
                  Serial.print(F("°C "));
                  Serial.print(F("%  Trigger Temperature: "));
                  Serial.print(trigger_temp);
                  Serial.print(F("°C "));
                  Serial.print(F("%  catastrophe: "));
                  Serial.println(catastrophe);
                  a = 'n';
                  break;

          case 's' : //Catastrophe reset
                  catastrophe = false;
            #ifdef ACTIVE_LOW
                  digitalWrite(SAFE_SWITCH_PIN, LOW);
            #else
                  digitalWrite(SAFE_SWITCH_PIN, HIGH);
                  digitalWrite(MOTOR_SAFETY_PIN, HIGH);
            #endif
                  trigger_temp = val;
                  a = 'n';
                  break;
          case 'p' : //pin toggle
                  digitalWrite(val, !digitalRead(val));
                  a = 'n';
                  break;
          //clear command
        }
        a='n';

        if(return_flag) {
                return_to_start_step();
        }
        else{
                //Elevator Motors
                stepsToTake = newMotorPosition - curMotorPosition;
                encoderStepsToTake = stepsToTake * 1.5;

                //previousStepsToTake seems to not be set up yet
                if (stepsToTake != 0 && previousStepsToTake == 0){
                    motor_timer = millis();
                }

                if ( stepsToTake > 0) {
                        if(read_switch(1)==1) {//stop collision with cell plate
                                myMotor1->onestep(FORWARD, INTERLEAVE);
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
                //X Stage Translation
                if ( xStepsToTake > 0) {
                        xTranslation->onestep(FORWARD, SINGLE);
                        xStepsToTake--;
                }
                else if ( xStepsToTake < 0) {
                        xTranslation->onestep(BACKWARD, SINGLE);
                        xStepsToTake++;
                }
                else {
                        xTranslation->release();
                }
                //Y Stage Translation
                if ( yStepsToTake > 0) {
                        yTranslation->onestep(FORWARD, SINGLE);
                        yStepsToTake--;
                }
                else if ( yStepsToTake < 0) {
                        yTranslation->onestep(BACKWARD, SINGLE);
                        yStepsToTake++;
                }
                else {
                        yTranslation->release();
                }


        }

}

void move_motor_to_position_with_feedback(){
  //these 2 variables should not be recalculated every time, i need to test
  stepsToTake = newMotorPosition - curMotorPosition;
  encoderStepsToTake = stepsToTake * 1.5;
  //make sure to reset count before using
  if ( encoderStepsToTake > 0){
      if ( safeMotorEncoderPositionA < encoderStepsToTake) {
          if(read_switch(1)==1) {//stop collision with cell plate
              myMotor1->onestep(FORWARD, INTERLEAVE);
              // myMotor2->onestep(FORWARD, INTERLEAVE);
              //curMotorPosition++;
          }
      }
      else{
          myMotor2->release();
      }
      if ( safeMotorEncoderPositionB < encoderStepsToTake) {
          if(read_switch(1)==1) {//stop collision with cell plate
              //myMotor1->onestep(FORWARD, INTERLEAVE);
              myMotor2->onestep(FORWARD, INTERLEAVE);
              //curMotorPosition++;
          }
      }
      else{
          myMotor2->release();
      }
  }
  if ( encoderStepsToTake < 0){
      if ( safeMotorEncoderPositionA > encoderStepsToTake) {
          if(read_switch(2)==1) {//stop collision with lower paddle
                  myMotor1->onestep(BACKWARD, INTERLEAVE);
                  // myMotor2->onestep(FORWARD, INTERLEAVE);
                  //curMotorPosition++;
          }
      }
      else{
          myMotor1->release();
      }
      if ( safeMotorEncoderPositionB > encoderStepsToTake) {
          if(read_switch(2)==1) {//stop collision with lower paddle
                  //myMotor1->onestep(FORWARD, INTERLEAVE);
                  myMotor2->onestep(FORWARD, INTERLEAVE);
                  //curMotorPosition++;
          }
      }
      else{
          myMotor2->release();
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

ISR(PCINT0_vect) {

  //Serial.println(PINB);

  uint8_t b = readB;

  if(stateB != b){

    if (readA == b) {
      count ++;
      } else {
      count --;
    }

    stateB = b;

  }

  uint8_t c = readC;

  if(stateC != c){

    if (readD == c) {
      count2 ++;
    }
    else {
      count2 --;
    }

    stateC = c;

  }
}