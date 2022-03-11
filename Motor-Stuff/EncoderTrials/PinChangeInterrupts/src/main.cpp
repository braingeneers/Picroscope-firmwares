#include <Arduino.h>
#include <Adafruit_MotorShield.h>
#include <SPI.h>
#include <pins_arduino.h>
#include <avr/interrupt.h>


const byte encoderPinA = A0;//outputA digital pin2
const byte encoderPinB = 8;//outoutB digital pin3
volatile int count = 0;
int protectedCount = 0;
int previousCount = 0;

const byte encoderPinC = 2;//outputA digital pin2
const byte encoderPinD = 3;//outoutB digital pin3
volatile int count2 = 0;
int protectedCount2 = 0;
int previousCount2 = 0;

int val = 0;

//void isrA();
//void isrB();

volatile uint8_t stateA = 0;
volatile uint8_t stateB = 0;

void isrC();
void isrD();

//#define readA bitRead(PINC, PC0)
#define readA bitRead(PINB, PB1)
#define readB bitRead(PINB, PB0)
uint8_t bus = B00000000;

//#define readA PINB | PB1
//#define readB PINB | PB0

#define readC digitalRead(2)
#define readD digitalRead(3)

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61);

// Connect a stepper motor with 200 steps per revolution (1.8 degree)
// to motor port #2 (M3 and M4)
Adafruit_StepperMotor *myStepper = AFMS.getStepper(200, 1);

// Connect a stepper motor with 200 steps per revolution (1.8 degree)
// to motor port #1 (M1 and M2)
Adafruit_StepperMotor *myStepper2 = AFMS.getStepper(200, 2);


void setup() {

  Serial.begin(115200);

  if (!AFMS.begin()) {         // create with the default frequency 1.6KHz
  // if (!AFMS.begin(1000)) {  // OR with a different frequency, say 1KHz
    Serial.println("Could not find Motor Shield. Check wiring.");
    while (1);
  }

  PCICR |= B00000001;
  PCMSK0 |= B00000011;
  //PCMSK1 |= B00000001;
 
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  pinMode(encoderPinC, INPUT_PULLUP);
  pinMode(encoderPinD, INPUT_PULLUP);

  stateA = readA;
  stateB = readB;

  bus = PINB;
  Serial.println(PINB);

  //attachInterrupt(digitalPinToInterrupt(encoderPinA), isrA, CHANGE);
  //attachInterrupt(digitalPinToInterrupt(encoderPinB), isrB, CHANGE);

  attachInterrupt(digitalPinToInterrupt(encoderPinC), isrC, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinD), isrD, CHANGE);

  delay(1000);
}

void loop() {

  noInterrupts();

  protectedCount = count;

  protectedCount2 = count2;

  interrupts();

  if(protectedCount != previousCount) {
    Serial.print("og = ");
    Serial.println(protectedCount);
  }

  if(protectedCount2 != previousCount2) {
    Serial.print("new = ");
    Serial.println(protectedCount2);
  }
  
  previousCount = protectedCount;

  previousCount2 = protectedCount2;

  if (Serial.available() >= 0) {
 
    val = Serial.parseInt();
    //Serial.println(val);
    delay(1000);

    if(val > 0){
      for(int i = 0; i < val; i++){
        myStepper->onestep(FORWARD, INTERLEAVE);
        delay(5);
        myStepper2->onestep(FORWARD, INTERLEAVE);
        delay(5);
      }
    }else if(val < 0){
      for(int i = 0; i > val; i--){
        myStepper->onestep(BACKWARD, INTERLEAVE);
        delay(5);
        myStepper2->onestep(BACKWARD, INTERLEAVE);
        delay(5);
      }
    }

  }

}

/*
void isrA() {
  if(readB != readA) {
    count ++;
  } else {
    count --;
  }
}
void isrB() {
  if (readA == readB) {
    count ++;
    } else {
    count --;
  }
}
*/

void isrC() {
  if(readD != readC) {
    count2 ++;
  } else {
    count2 --;
  }
}
void isrD() {
  if (readC == readD) {
    count2 ++;
    } else {
    count2 --;
  }
}


ISR(PCINT0_vect) {

  if((bus == 60 && PINB == 61) || (bus == 61 && PINB == 63) || (bus == 63 && PINB == 62) || (bus == 62 && PINB == 60)){

    count--;

  }else if((bus == 60 && PINB == 62) || (bus == 62 && PINB == 63) || (bus == 63 && PINB == 61) || (bus == 61 && PINB == 60)){

    count++;

  }

  bus = PINB;

}


/*
ISR(PCINT0_vect) {

  Serial.println(PINB);

  uint8_t b = readB;

  if(stateB != b){

    if (readA == b) {
      count ++;
      } else {
      count --;
    } 

    stateB = b;

  }
*/
  /*

  uint8_t a = readA;
  uint8_t b = readB;

  if(stateA != a){

    if(b != a) {
      count ++;
    } else {
      count --;
    }

    stateA = a;

  }else if(stateB != b){

    if (a == b) {
      count ++;
      } else {
      count --;
    } 

    stateB = b;

  }
*/
//} 

/*
ISR(PCINT1_vect){

  uint8_t a = readA;

  if(readB != a) {
      count ++;
    } else {
      count --;
    }

  stateA = a;

}
*/