/*
   This code is for running the stepper motors and GFP blue_light board for the Braingeneers PiCroscope Project


 */

//#define DEBUG
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

#define DHTTYPE DHT22
// DHT 22  (AM2302), AM2321
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
int newEncoderPosition = 0;
int encoderA_StepsToTake = 0;
int encoderB_StepsToTake = 0;
int xStepsToTake = 0;
int yStepsToTake = 0;
bool dirA_up = true;
bool dirB_up = true;
int encoderZero = 0;


//CopyPated encoder code starts hereby



volatile int count = 0;
int safeMotorEncoderPositionA = 0;
int previousCount = 0;


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
const byte encoderPinB = 9;
const byte encoderPinA = 10;//outoutB digital pin3
//note B definition comes first, this was a quick fix to reverse direction of count
#define readB bitRead(PINB, PB1) //D9
#define readA bitRead(PINB, PB2) //D10

uint8_t bus = B00000000;
volatile byte pinBStatus;

//#define readA PINB | PB1
//#define readB PINB | PB0

// #define readC digitalRead(2)
// #define readD digitalRead(3)
const byte encoderPinC = 12;
const byte encoderPinD = 13;
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

void encoder_setup(){
  PCICR |= B00000001;
  //PCMSK0 |= B00000011;


  //trying to put the other interrupt on
  PCMSK0 |= B00110110; //this corresponds to pins D8 and D11
  //PCMSK1 |= B00000001;

  // pinMode(encoderPinA, INPUT_PULLUP);
  // pinMode(encoderPinB, INPUT_PULLUP);
  //
  // pinMode(encoderPinC, INPUT_PULLUP);
  // pinMode(encoderPinD, INPUT_PULLUP);

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
        //pinMode(SWITCH_2_PIN, INPUT);
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
unsigned long temp_timer = millis();
unsigned long motor_timer = millis();
float highest_temp = 0;
float trigger_temp = 50;
bool catastrophe = false;
unsigned long speed_timer = 0;
bool speed_timer_flag = false;
bool motorA_on = false;
bool motorB_on = false;
bool motors_moving = false;
int nanCount = 0;
int hysteresisA = 0;
int hysteresisB = 0;
#define HYSTERESIS_GAP 5



void move_motor_to_position_with_feedback();
void shut_down_everything();
void lights_off();

void loop() {

        noInterrupts();
        safeMotorEncoderPositionA = count;
        safeMotorEncoderPositionB = count2;
        interrupts();


        motors_moving = (motorA_on || motorB_on || return_flag || stepsToTake != 0 || xStepsToTake != 0 || yStepsToTake !=0);
        if (!motors_moving){
            motor_timer = millis();
        }

        if (abs(millis() - temp_timer) > 2500 && !motors_moving) {
                //we're not reading temp while motors are trying to move add timers for motors
                t = dht.readTemperature();
                if (isnan(t)){
                  nanCount++;
                }
                else{
                  nanCount = 0;
                }
                if (t > highest_temp) {
                        highest_temp = t;
                }
                if (t > trigger_temp || nanCount > 10) {
                    shut_down_everything();
                }
        }
        //motor running too long
        else if (abs(millis() - motor_timer) > 25000 && motors_moving) {
            shut_down_everything();
        }
        //Serial.println("running: ");
        if (Serial.available() >= 2) {
                a = Serial.read();
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

          case 'e':
          //report encoder data or take encoder based feedback steps
                  switch(val){
                      case 1:
                      //report encoder positions
                        Serial.print("Motor A: ");
                        Serial.println(safeMotorEncoderPositionA);
                        Serial.print("Motor B: ");
                        Serial.println(safeMotorEncoderPositionB);
                        Serial.print("Encoder Zero Point: ");
                        Serial.println(encoderZero);
                        Serial.print("Motor Timer: ");
                        Serial.println(abs(millis() - motor_timer));
                        Serial.print("Motor A On:");
                        Serial.println(motorA_on);
                        Serial.print("Motor B On:");
                        Serial.println(motorB_on);


                        break;

                      case 2:
                      //recalibrate set zero
                        count = 0;
                        count2 = 0;
                        safeMotorEncoderPositionA = 0;
                        safeMotorEncoderPositionB = 0;
                        encoderZero = 0;
                        newEncoderPosition = 0;
                        break;

                      default:
                        //set number of steps to take in encoder based feedback control
                        // count = 0;
                        // count2 = 0;
                        // safeMotorEncoderPositionA = 0;
                        // safeMotorEncoderPositionB = 0;

                        //encoder steps to take is acted on outside this state machine
                        //encoderStepsToTake = val;
                        //motor_timer = millis();

                        //copying this code into the case M block for quick compatible drop in

                        hysteresisA = hysteresisB = 0;
                        newEncoderPosition = (val+encoderZero);
                  }
                  break;

          case 'c' :
          //set zero point without limit switch based reset.
          //deprecated, unlikely to ever be used in practice
                  curMotorPosition = 0;
                  newMotorPosition = 0;
                  break;

          case 'r' :
          //return to origin based on limit switch
                  //motor_timer = millis();
                  return_flag = true;
                  curMotorPosition = 0; //uncommented this to make ERROR condition work properly in return_to_start_step()
                  //newMotorPosition = 0;
                  a = 'n';
                  break;
          case 'm' :
          //set new motor position
                  //    if ((val > -1000) && (val < 1000)) safety
                  #ifdef DEBUG
                  //measure motor speed in debug mode
                  speed_timer = millis();
                  speed_timer_flag = true;
                  #endif

                  //copied this code from the case E block for quick compatible drop in
                  // dirA_up = ((val + encoderZero) > safeMotorEncoderPositionA);
                  // dirB_up = ((val + encoderZero) > safeMotorEncoderPositionB);
                  hysteresisA = hysteresisB = 0;
                  newEncoderPosition = (val+encoderZero);
                  // newMotorPosition = val;
                  //motor_timer = millis();
                  break;
          case 'x' :
          //move x axis motors on XY stage
                  //    if ((val > -1000) && (val < 1000)) safety
                  //motor_timer = millis();
                  xStepsToTake = val;
                  break;

          case 'y' :
          //move y axis motors on XY plate
                  //    if ((val > -1000) && (val < 1000)) safety
                  //motor_timer = millis();
                  yStepsToTake = val;
                  break;

          case 'l' :
          //control lights
                  if (val == 0) {
                  //turn all lights off
                          #ifdef ACTIVE_LOW
                          digitalWrite(BLUE_LED_PIN, HIGH);
                          digitalWrite(WHITE_LED_PIN, HIGH);
                          #else
                          digitalWrite(BLUE_LED_PIN, LOW);
                          digitalWrite(WHITE_LED_PIN, LOW);
                          #endif
                  }
                  //otherwise turn one of the two lights on
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
                  return_flag = false; //needed to reset motor catastrophe
                  stepsToTake = 0;
                  xStepsToTake = 0;
                  yStepsToTake = 0;

                  count = 0;
                  count2 = 0;
                  safeMotorEncoderPositionA = 0;
                  safeMotorEncoderPositionB = 0;
                  encoderZero = 0;
                  newEncoderPosition = 0;
            #ifdef ACTIVE_LOW
                  digitalWrite(SAFE_SWITCH_PIN, LOW);
            #else
                  digitalWrite(SAFE_SWITCH_PIN, HIGH);
                  digitalWrite(MOTOR_SAFETY_PIN, HIGH);
            #endif
                  trigger_temp = val;
                  a = 'n';
                  break;

          case 'p' :
          // toggle the state of any pin, useful for debugging hardware
                  digitalWrite(val, !digitalRead(val));
                  a = 'n';
                  break;
        }

        //clear unprocessed command
        a='n';

        //We are now outside of the serial command response logic
        //taking action of the flags set above in a non blocking fashion
        if(return_flag) {
                //if the return flag is high then we will take single steps back
                //towards our limit switch paddle until we reach it
                return_to_start_step();
        }
        // else if (newMotorPosition != curMotorPosition){
        //         //Elevator Motors
        //         //Move motors based on values defined in serial
        //         stepsToTake = newMotorPosition - curMotorPosition;
        //         //encoderStepsToTake = stepsToTake * 1.5;
        //
        //         if ( stepsToTake > 0) {
        //                 if(read_switch(1)==1) {//stop collision with cell plate
        //                         myMotor1->onestep(FORWARD, INTERLEAVE);
        //                         myMotor2->onestep(FORWARD, INTERLEAVE);
        //                         curMotorPosition++;
        //                 }else{
        //                         newMotorPosition = curMotorPosition;
        //                         stepsToTake = 0;
        //                 }
        //         }
        //         else if ( stepsToTake < 0) {
        //                 myMotor1->onestep(BACKWARD, INTERLEAVE);
        //                 myMotor2->onestep(BACKWARD, INTERLEAVE);
        //                 curMotorPosition--;
        //         }
        //         else {
        //                 #ifdef DEBUG
        //                 if (speed_timer_flag){
        //                   Serial.println(millis()-speed_timer);
        //                   speed_timer_flag = false;
        //                 }
        //                 #endif
        //                 myMotor1->release();
        //                 myMotor2->release();
        //         }
        //}
       else if (xStepsToTake || yStepsToTake){
           if ( xStepsToTake > 0) {
                   xTranslation->onestep(FORWARD, INTERLEAVE);
                   xStepsToTake--;
           }
           else if ( xStepsToTake < 0) {
                   xTranslation->onestep(BACKWARD, INTERLEAVE);
                   xStepsToTake++;
           }
           else {
                   xTranslation->release();
           }
           //Y Stage Translation
           if ( yStepsToTake > 0) {
                   yTranslation->onestep(FORWARD, INTERLEAVE);
                   yStepsToTake--;
           }
           else if ( yStepsToTake < 0) {
                   yTranslation->onestep(BACKWARD, INTERLEAVE);
                   yStepsToTake++;
           }
           else {
                   yTranslation->release();
           }

       }
       else{
                //single step encoder feedback control
                //not running if in return mode
                //should investigate what happens if we dont use that else
                move_motor_to_position_with_feedback();

                //X Stage Translation


        }


}

void move_motor_to_position_with_feedback(){
//must confirm which encoder reads which motor
//otherwise you end up with one turning forever
    if ((safeMotorEncoderPositionA < newEncoderPosition - hysteresisA) && read_switch(1)==1 ) {
        // switch stops collision with cell plate
        motorA_on = true;
        myMotor2->onestep(FORWARD, INTERLEAVE);

    }
    else if ((safeMotorEncoderPositionA > newEncoderPosition + hysteresisA) && read_switch(2)==1 ) {
        motorA_on = true;
        myMotor2->onestep(BACKWARD, INTERLEAVE);

    }
    else{
        motorA_on = false;
        myMotor2->release();
        hysteresisA = HYSTERESIS_GAP;
    }
    if ((safeMotorEncoderPositionB < newEncoderPosition - hysteresisB) && read_switch(1)==1) {
        motorB_on = true;
        myMotor1->onestep(FORWARD, INTERLEAVE);
    }
    else if ((safeMotorEncoderPositionB > newEncoderPosition + hysteresisB) && read_switch(2)==1) {
            motorB_on = true;
            myMotor1->onestep(BACKWARD, INTERLEAVE);
    }
    else{
        motorB_on = false;
        myMotor1->release();
        hysteresisB = HYSTERESIS_GAP;
    }
  }

// void move_motor_steps_with_feedback(){
// //must confirm which encoder reads which motor
// //otherwise you end up with one turning forever
//   if ( encoderStepsToTake > 0){
//       if ( safeMotorEncoderPositionA < encoderStepsToTake) {
//           if(read_switch(1)==1) {//stop collision with cell plate
//               myMotor2->onestep(FORWARD, INTERLEAVE);
//               // myMotor2->onestep(FORWARD, INTERLEAVE);
//               //curMotorPosition++;
//           }
//       }
//       else{
//           myMotor2->release();
//       }
//       if ( safeMotorEncoderPositionB < encoderStepsToTake) {
//           if(read_switch(1)==1) {//stop collision with cell plate
//               //myMotor1->onestep(FORWARD, INTERLEAVE);
//               myMotor1->onestep(FORWARD, INTERLEAVE);
//               //curMotorPosition++;
//           }
//       }
//       else{
//           myMotor1->release();
//       }
//   }
//   if ( encoderStepsToTake < 0){
//       if ( safeMotorEncoderPositionA > encoderStepsToTake) {
//           if(read_switch(2)==1) {//stop collision with lower paddle
//                   myMotor2->onestep(BACKWARD, INTERLEAVE);
//                   // myMotor2->onestep(FORWARD, INTERLEAVE);
//                   //curMotorPosition++;
//           }
//       }
//       else{
//           myMotor2->release();
//       }
//       if ( safeMotorEncoderPositionB > encoderStepsToTake) {
//           if(read_switch(2)==1) {//stop collision with lower paddle
//                   //myMotor1->onestep(FORWARD, INTERLEAVE);
//                   myMotor1->onestep(BACKWARD, INTERLEAVE);
//                   //curMotorPosition++;
//           }
//       }
//       else{
//           myMotor1->release();
//       }
//   }
// }

void return_to_start_step(){
        //non blocking state machine to bring elevator down to where limit switch collides and then back up until it's free
        //now needs to compensate for encoder feeback control implementation
        enum state {DOWN, UP, EXIT, ERROR};
        static int state = DOWN;
        switch (state)
        {
        case DOWN:
                if(digitalRead(SWITCH_2_PIN) == 1) { //safety in case of limit switch failure
                        myMotor1->onestep(BACKWARD, INTERLEAVE);
                        myMotor2->onestep(BACKWARD, INTERLEAVE);
                        curMotorPosition--;
                        // if(curMotorPosition < -5000) {
                        //         state = ERROR;//error condition
                        // }
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
                state = DOWN;
                // update motor encoder feedback control value to prevent elevator from going back up
                // also has benefit of leveling the elevator to the higher encoder
                if (safeMotorEncoderPositionA > safeMotorEncoderPositionB){
                  newEncoderPosition = safeMotorEncoderPositionA;
                }else{
                  newEncoderPosition = safeMotorEncoderPositionB;
                }
                encoderZero = newEncoderPosition;

                break;
        // case ERROR:
        //         shut_down_everything();
        //         return_flag = false;
        //         curMotorPosition = 0;
        //         newMotorPosition = 0;
        //         state = DOWN;
        //         break;

        }
        //correct for mechanical hysteresis in limit switch
        //read_switch(1) ensures no collision with tissue culture
        // while(digitalRead(SWITCH_2_PIN)==0) {
        //         myMotor1->onestep(FORWARD, INTERLEAVE);
        //         myMotor2->onestep(FORWARD, INTERLEAVE);
        //         curMotorPosition++;
        // }
}

/*
shut_down_everything initiates catastrophe mode
turns off all relays and sets global catastrophe flag to true
*/
void shut_down_everything(){
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


/*
Interrupt Service Routine for counting steps from the motor encoders

A little bit inscrutable, but it's short and it works
*/
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
