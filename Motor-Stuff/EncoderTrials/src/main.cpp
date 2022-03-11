#include <Arduino.h>

#include <Adafruit_MotorShield.h>
#include <SPI.h>

const byte encoderPinA = 2;//outputA digital pin2
const byte encoderPinB = 3;//outoutB digital pin3
volatile int count = 0;
int protectedCount = 0;
int previousCount = 0;

int val = 0;

void isrA();
void isrB();

#define readA bitRead(PIND,2)//faster than digitalRead()
#define readB bitRead(PIND,3)//faster than digitalRead()

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61);

// Connect a stepper motor with 200 steps per revolution (1.8 degree)
// to motor port #2 (M3 and M4)
Adafruit_StepperMotor *myStepper = AFMS.getStepper(200, 2);


void setup() {

  Serial.begin (115200);


  if (!AFMS.begin()) {         // create with the default frequency 1.6KHz
  // if (!AFMS.begin(1000)) {  // OR with a different frequency, say 1KHz
    Serial.println("Could not find Motor Shield. Check wiring.");
    while (1);
  }


  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(encoderPinA), isrA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), isrB, CHANGE);

  delay(2000);
}

void loop() {

  noInterrupts();
  protectedCount = count/3;
  interrupts();

  //Serial.println(protectedCount);

  //while(1);
  
  if(protectedCount != previousCount) {
    Serial.println(protectedCount);
  }
  
  previousCount = protectedCount;

  if (Serial.available() >= 0) {
 
    val = Serial.parseInt();
    //Serial.println(val);
    delay(1000);

    if(val > 0){
      for(int i = 0; i < val; i++){
        myStepper->onestep(FORWARD, SINGLE);
      }
    }else if(val < 0){
      for(int i = 0; i > val; i--){
        myStepper->onestep(BACKWARD, SINGLE);
      }
    }

  }

}

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