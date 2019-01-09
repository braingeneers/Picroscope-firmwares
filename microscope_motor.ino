

/*
  This is a test sketch for the Adafruit assembled Motor Shield for Arduino v2
  It won't work with v1.x motor shields! Only for the v2's with built in PWM
  control

  For use with the Adafruit Motor Shield v2
  ---->  http://www.adafruit.com/products/1438
*/


#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();

//L -> R
//Yellow, Orange, NC, Brown, Redaaaaa
Adafruit_StepperMotor * myMotor1 = AFMS.getStepper(200,  1);
Adafruit_StepperMotor *myMotor2 = AFMS.getStepper(200,  2);

int step_count[2] = {0, 0};
int brightness = 100;
long step_time;
int delay_time = 20000;
void setup() {
  Serial.begin(115200);           // set up Serial library at 9600 bps
  Serial.println("Microscope Stepper controller");
  Serial.println("q to move up 100 steps and w to move down 100 steps");
  Serial.println("a to move up 10 steps and s to move down 10 steps");
  Serial.println("z to move up 1 steps and x to move down 1 steps");
  Serial.println("1 to increase brightness and 2 to decrease brightness");

  AFMS.begin();  // create with the default frequency 1.6KHz
  //AFMS.begin(1000);  // OR with a different frequency, say 1KHz
  pinMode(3, OUTPUT);
  analogWrite(3, brightness);
  myMotor1->setSpeed(200);  // 10 rpm
  myMotor2->setSpeed(200);
  step_time = millis();
}

void loop() {
  if (Serial.available() > 0) {

    int inByte = Serial.read();
    //Serial.println(inByte);
    switch (inByte) {
      case 'q':
        step_count[0] = step_count[0] + 100;
        step_count[1] = step_count[1] + 100;
        break;
      case 'w':
        step_count[0] = step_count[0] - 100;
        step_count[1] = step_count[1] - 100;
        break;
      case 'a':
        step_count[0] = step_count[0] + 10;
        step_count[1] = step_count[1] + 10;
        break;
      case 's':
        step_count[0] = step_count[0] - 10;
        step_count[1] = step_count[1] - 10;
        break;
      case 'z':
        step_count[0] = step_count[0] + 1;
        step_count[1] = step_count[1] + 1;
        break;
      case 'x':
        step_count[0] = step_count[0] - 1;
        step_count[1] = step_count[1] - 1;
        break;
      case '1':
        brightness = (brightness + 10);
        if (brightness > 255) {
          brightness = 255;
        }
        analogWrite(3, brightness);
        break;
      case '2':
        brightness = (brightness - 10) ;
        if (brightness < 0) {
          brightness = 0;
        }
        analogWrite(3, brightness);
        break;
      case '0':
        step_count[0] = 0;
        step_count[1] = 0;
    }
  }

  if (step_count[0] == 0)
  {
    myMotor1->release();
  }
  if (step_count[1] == 0)
  {
    myMotor2->release();
  }
  
  if (step_count[0] > 0)
  {
    myMotor1->step(1, FORWARD, SINGLE);
    step_count[0]--;
  }
  else if (step_count[0] < 0)
  {
    myMotor1->step(1, BACKWARD, SINGLE);
    step_count[0]++;
  }

  if (step_count[1] > 0)
  {
    myMotor2->step(1, FORWARD, SINGLE);
    step_count[1]--;
  }
  else if (step_count[1] < 0)
  {
    myMotor2->step(1, BACKWARD, SINGLE);
    step_count[1]++;
  }
  /*
    if (millis() - step_time >= delay_time)
    {
    /*
      myMotor->step(1, FORWARD, SINGLE);
      if (delay_time >= 2000)
      {
      delay(1000);
      myMotor->release();
      }*/
  //  step_time = millis();
  //}

}
