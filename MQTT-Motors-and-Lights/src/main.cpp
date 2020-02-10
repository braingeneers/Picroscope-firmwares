/*
   This is a test sketch for the Adafruit assembled Motor Shield for Arduino v2
   It won't work with v1.x motor shields! Only for the v2's with built in PWM
   control

   For use with the Adafruit Motor Shield v2
   ---->	http://www.adafruit.com/products/1438
 */


#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include <PubSubClient.h>
#include "ESPmDNS.h"
#include "WiFi.h"
#include <stdio.h>
#include <ArduinoOTA.h>


#define DEBUG
//#define SWITCH_2_PIN 7
#define SWITCH_2_PIN 14
//#define SWITCH_1_PIN 12
#define SWITCH_1_PIN 19
//#define BLUE_LED_PIN 2
#define BLUE_LED_PIN 26
//#define WHITE_LED_PIN 3
#define WHITE_LED_PIN 25

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61);

// Connect a stepper motor with 200 steps per revolution (1.8 degree)
// to motor port #2 (M3 and M4)
//Adafruit_StepperMotor *myMotor = AFMS.getStepper(200, 2);
Adafruit_StepperMotor * myMotor1 = AFMS.getStepper(200,  1);
Adafruit_StepperMotor * myMotor2 = AFMS.getStepper(200,  2);



//byte index = 0;
int address = 0;
int ledPin = 3;

int step_count[2] = {0, 0};
int curMotorPosition = 0;
int newMotorPosition = 0;
//int curMotorPosition = 0;
//int newMotorPosition = 0;
int stepsToTake = 0;

int val = -1;
char a = 'n';
char b = 'n';
bool return_flag = false;


boolean read_switch(int lim_switch) {
        if(lim_switch==2)
        {
                return digitalRead(SWITCH_2_PIN);
        }
        else if(lim_switch==1)
        {
                //quick hack
                //return true;
                //return digitalRead(12);
                return digitalRead(SWITCH_1_PIN);
        }
        return -1;
}

void return_to_start(){
        while(digitalRead(SWITCH_1_PIN) && curMotorPosition > -5000) { //safety in case of limit switch failure
                myMotor1->onestep(BACKWARD, INTERLEAVE);
                myMotor2->onestep(BACKWARD, INTERLEAVE);
                curMotorPosition--;
        }
        //correct for mechanical hysteresis in limit switch
        //read_switch(1) ensures no collision with tissue culture
        while(digitalRead(SWITCH_1_PIN)==0) {
                myMotor1->onestep(FORWARD, INTERLEAVE);
                myMotor2->onestep(FORWARD, INTERLEAVE);
                curMotorPosition++;
        }
}

void return_to_start_step();

const char* ssid = "TP-LINK_PiScope";
const char* password =  "raspberry";
const char* clientName = "ESP32thingy_1";

//const char* mqtt_server = "10.1.10.88";
const char * serverHostname = "microscopehub";
const int mqtt_port = 1883;
char buff[20];

WiFiClient espClient;
PubSubClient client(espClient);

void reconnect();

void callback(char* topic, byte* payload, unsigned int length) {
        Serial.print("Message arrived [");
        Serial.print(topic);
        Serial.print("] ");
        for (int i=0; i<length; i++) {
                Serial.print((char)payload[i]);
                buff[i] = (char)payload[i];
        }
        buff[length] = '\0';
        sscanf (buff,"%c : %d",&a,&val);
        //sscanf("foo", "f%s", a);
        Serial.print(" a: ");
        Serial.print(a);
        Serial.print(" val: ");
        Serial.println(val, DEC);
        Serial.println();
}

void setup(){
        Serial.begin(115200); // set up Serial library at 9600 bps
        //Serial.println("starting: ");
        AFMS.begin(); // create with the default frequency 1.6KHz

        myMotor1->setSpeed(200); // 10 rpm
        myMotor2->setSpeed(200);

        pinMode(BLUE_LED_PIN, OUTPUT);
        pinMode(WHITE_LED_PIN, OUTPUT);
        pinMode(4, OUTPUT);
        digitalWrite(BLUE_LED_PIN, LOW);
        digitalWrite(WHITE_LED_PIN, LOW);
        pinMode(SWITCH_2_PIN, INPUT);
        pinMode(SWITCH_1_PIN, INPUT);




        WiFi.begin(ssid, password);
        Serial.println("Connecting to WiFi");
        while (WiFi.status() != WL_CONNECTED) {
                delay(500);
                Serial.print(".");
        }

        Serial.println(" Connected to the WiFi network");

        if (!MDNS.begin(clientName)) {
                Serial.println("Error setting up MDNS responder!");
        } else {
                Serial.println("Finished intitializing the MDNS client...");
        }

        IPAddress serverIp = MDNS.queryHost(serverHostname);
        while (serverIp.toString() == "0.0.0.0") {
                Serial.println("Trying again to resolve mDNS");
                delay(250);
                serverIp = MDNS.queryHost(serverHostname);
        }
        Serial.print("IP address of server: ");
        Serial.println(serverIp.toString());

        client.setServer(serverIp, mqtt_port);
        client.setCallback(callback);

        // ArduinoOTA.onStart([](){
        //         String type;
        //         if(ArduinoOTA.getCommand() == U_FLASH)
        //                 type = "sketch";
        //         else
        //                 type = "filesystem";
        //
        //         Serial.println("Starting update: " + type);
        // })
        //
        // .onEnd([]() {
        //         Serial.println("\nDone");
        //
        // })
        // .onProgress([](unsigned int progress, unsigned int total) {
        //         Serial.printf("Progress: %u%%/r", (progress / (total / 100)));
        // })
        // .onError([](ota_error_t error) {
        //         Serial.printf("Error[%u]: ", error);
        //         if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        //         else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        //         else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        //         else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        //         else if (error == OTA_END_ERROR) Serial.println("End Failed");
        // });


}
int timer = 0;

void loop() {
        if(abs(millis() - timer) > 3000) {
                timer = millis();
                Serial.println("running: ");
                if(WiFi.status() != WL_CONNECTED) {
                        //for safety
                        myMotor1->release();
                        myMotor2->release();

                        Serial.println("Lost connection");
                        if(WiFi.status() == WL_NO_SSID_AVAIL) {
                                Serial.println("SSID not found");
                        }
                        WiFi.begin(ssid, "raspberry");
                        Serial.println("Connecting to WiFi: Reconnect");
                        if(WiFi.status() == WL_CONNECTED) {
                                if (!client.connected()) {
                                        reconnect();
                                }
                        }
                        // while (WiFi.status() != WL_CONNECTED) {
                        //         delay(500);
                        //         Serial.print(".");
                        // }
                }
                else if (!client.connected()){
                  myMotor1->release();//probably unecessary, very cautious
                  myMotor2->release();
                  reconnect();
                }
        }

        client.loop();

        // ArduinoOTA.handle();

        // if (Serial.available() >= 2) {
        //         a = Serial.read();
        //         //for fast GFP blue_light response act here to avoid processing delays
        //         b = Serial.read();
        //         val = Serial.parseInt();
        //         //    //flush
        //         while(Serial.available()) {
        //                 Serial.read();
        //                 Serial.println("flushing");
        //         }
// #ifdef DEBUG
//         Serial.println(a);
//         Serial.println(b);
//         Serial.println(val, DEC);
//
// #endif
        // Serial.print("switch zero: ");Serial.print(read_switch(0));
        // Serial.print(" switch one: "); Serial.print(read_switch(1)); Serial.println();
        // delay(500);
        if ( a == 'c') {
                //Calibration case
                curMotorPosition = 0;
                newMotorPosition = 0;
                //EEPROM.update(address, 0);
        }
        if ( a == 'r') {
                //return to origin based on limit switch
                return_flag = true;
                //return_to_start();
                //curMotorPosition = 0;
                //newMotorPosition = 0;
                ////EEPROM.update(address, 0);
                a = 'n';
        }
        if ( a == 'm') {
                //    if ((val > -1000) && (val < 1000)) safety

                newMotorPosition = val;
                //save the new motor position into //EEPROM
                //EEPROM.update(address, val);
        }
        if(return_flag) {
                return_to_start_step();
        }
        else{
                stepsToTake = newMotorPosition - curMotorPosition;
                if ( stepsToTake > 0) {
                        if(true) {//read_switch(1)==1) {//stop collision with cell plate
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




}


void reconnect() {
        //if (!client.connected()) {
                Serial.print("Attempting MQTT connection...");
                // Attempt to connect
                if (client.connect(clientName)) {
                        Serial.println("connected");
                        // Subscribe
                        client.subscribe("motorControl");
                } else {
                        Serial.print("failed, rc=");
                        Serial.print(client.state());
                        Serial.println(" try again in 2 seconds");
                        // Wait 5 seconds before retrying
                        //delay(5000);
                }
        //}
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
                myMotor1->release();
                myMotor2->release();
                ////EEPROM.update(address, 0);
                state = DOWN;
                break;
        case ERROR:
                return_flag = false;
                curMotorPosition = 0;
                newMotorPosition = 0;
                myMotor1->release();
                myMotor2->release();
                //.update(address, 0);
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

// void setup() {
//         Serial.begin(9600);     // set up Serial library at 9600 bps
//         Serial.println("Stepper test!");
//
//         AFMS.begin(); // create with the default frequency 1.6KHz
//         //AFMS.begin(1000);  // OR with a different frequency, say 1KHz
//
//         myMotor1->setSpeed(10); // 10 rpm
// }
//
// void loop() {
//         Serial.println("Single coil steps");
//         myMotor1->step(100, FORWARD, SINGLE);
//         myMotor1->step(100, BACKWARD, SINGLE);
//
//         Serial.println("Double coil steps");
//         myMotor1->step(100, FORWARD, DOUBLE);
//         myMotor1->step(100, BACKWARD, DOUBLE);
//
//         Serial.println("Interleave coil steps");
//         myMotor1->step(100, FORWARD, INTERLEAVE);
//         myMotor1->step(100, BACKWARD, INTERLEAVE);
//
//         Serial.println("Microstep steps");
//         myMotor1->step(50, FORWARD, MICROSTEP);
//         myMotor1->step(50, BACKWARD, MICROSTEP);
// }
