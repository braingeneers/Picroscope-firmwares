/*
   Copyright (c) 2015, Majenko Technologies
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

 * * Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.

 * * Redistributions in binary form must reproduce the above copyright notice, this
     list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.

 * * Neither the name of Majenko Technologies nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   Modified by Pierre Baudin
*/

/* Connect to a WiFi access point and provide a web server on it. */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <Wire.h>
#include "MCP4728.h"

/* Set these to your desired credentials. */
ESP8266WiFiMulti wifiMulti;
ESP8266WebServer server(80);

const char *ssid = "TP-LINK_PiScope";
const char *password = "raspberry";

String header;

MCP4728 dac;

const int dataPin = 16;   //Outputs the byte to transfer
const int loadPin = 12;   //Controls the internal transference of data in SN74HC595 internal registers
const int clockPin = 14;  //Generates the clock signal to control the transference of data

int led_brightness = 50;

String getValue(String data, char separator, int index);
void led_display(int i, bool on = false);
String getPage();
void handleLED();
void handleBrightness();
void handleRoot();


void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();

  wifiMulti.addAP(ssid, password);

  Serial.println("Connecting ...");

  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
    delay(250);
    Serial.print('.');
  }
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // Tell us what network we're connected to
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("LEDcontroller")) {              // Start the mDNS responder for esp8266.local
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }

  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");

  Serial.println("init DAC");
  pinMode(dataPin, OUTPUT);
  pinMode(loadPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  Wire.begin(4, 5);
  dac.attatch(Wire, 13);
  dac.readRegisters();

  dac.selectVref(MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V);
  dac.selectPowerDown(MCP4728::PWR_DOWN::GND_100KOHM, MCP4728::PWR_DOWN::GND_100KOHM, MCP4728::PWR_DOWN::GND_100KOHM, MCP4728::PWR_DOWN::GND_100KOHM);
  dac.selectGain(MCP4728::GAIN::X2, MCP4728::GAIN::X2, MCP4728::GAIN::X2, MCP4728::GAIN::X2);
  //dac.analogWrite(MCP4728::DAC_CH::A, 1850); //Range 1500-2200
  dac.analogWrite(MCP4728::DAC_CH::A, 1500 + ((led_brightness - 10) / 90.0) * 700);
  dac.analogWrite(MCP4728::DAC_CH::B, 0);
  dac.analogWrite(MCP4728::DAC_CH::C, 0);
  dac.analogWrite(MCP4728::DAC_CH::D, 0);
  dac.enable(true);
  dac.readRegisters();

  led_display(0);
}

int id = -1;
char a = 'n';
char b = 'n';

void loop() {
  server.handleClient();
  if (Serial.available() >= 2){
    // LED_id
    a = Serial.read(); // 1 or 0 on/off
    b = Serial.read(); // colon separator
    id = Serial.parseInt();
    //Serial.read(); //flush newline char

    Serial.println(a);
    Serial.println(b);
    Serial.println(id);

    if(id >= 0 && id <= 24)
      led_display(id, a == '1');

    while(Serial.available())
      Serial.read(); //disgusting blocking code to deal with flushing serial line

  }

}



String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void led_display(int i, bool on)
{
  static long data = 0;

  if (on)
    data = data | 1 << i;
  else
    data = data & ~(1 << i);

  if(i == 25) //all off condition
    data = 0;
  //Serial.println(data);
  //byte data = B01010101;
  digitalWrite(loadPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, (data >> 16) & B11111111);
  delay(10);
  shiftOut(dataPin, clockPin, MSBFIRST, (data >> 8) & B11111111);
  delay(10);
  shiftOut(dataPin, clockPin, MSBFIRST, data & B11111111);
  delay(10);
  digitalWrite(loadPin, HIGH);
}

String getPage()
{
  String page = "<html><head></head>";
  page += "<table>";
  /*
    for (int i = 0; i < 24; i++) {
    page += "<tr><td>LED " + String(i + 1) + "</td>";
    page += "<td><form action='/' method='POST'><button type='button submit' name='led' value='" + String(i) + ",1'>ON</button></form></td>";
    page += "<td><form action='/' method='POST'><button type='button submit' name='led' value='" + String(i) + ",0'>OFF</button></form></td>";
    page += "</tr><tr>";
    }*/
  for (int i = 0; i < 4; i++)
  {
    page += "<tr>";
    for (int j = 0; j < 6; j++)
    {
      page += "<td><form action='/' method='POST'><button type='button submit' name='led' value='" + String(i * 6 + j) + ",1'>LED" + String(i * 6 + j) + "</button></form></td>";
    }
    page += "</tr>";
  }
  page += "<tr><td>All LEDs</td>";
  page += "<td><form action='/' method='POST'><button type='button submit' name='led' value='100,0'>OFF</button></form></td>";
  page += "</tr><tr>";

  page += "<tr><td>Brightness</td>";
  page += "<td><form action='/' method='POST'>";
  page += "  <select name='brightness' id='brightness' onchange='this.form.submit()'>";

  for (int i = 1; i < 11; i++)
  {
    page += "<option value='" + String(i * 10);
    if (led_brightness == i * 10) {
      page += "' selected>" + String(i * 10) + "%</option>";
    }
    else {
      page += "'>" + String(i * 10) + "%</option>";
    }
  }

  page += "</select></form></td></tr>";
  page += "</table></body></html>";
  return page;
}
void handleLED() {
  int led_id = getValue(server.arg("led"), ',', 0).toInt();
  int led_status = getValue(server.arg("led"), ',', 1).toInt();

  Serial.println(led_id);
  Serial.println(led_status);
  if (led_id == 100) {
    Serial.println("all off");
    led_display(25);
  }
  else {
    if (led_status == 1)
    {
      led_display(led_id, true);
    }
    else {
      led_display(led_id, false);
    }
  }
}
void handleBrightness() {
  led_brightness = getValue(server.arg("brightness"), ',', 0).toInt();
  dac.analogWrite(MCP4728::DAC_CH::A, 1500 + ((led_brightness - 10) / 90.0) * 700);
  Serial.println(1500 + ((led_brightness - 10) / 90.0) * 700);
  Serial.println("set brightness to" + String(led_brightness));
}
void handleRoot()
{
  if ( server.hasArg("led") ) {
    handleLED();
  } else if ( server.hasArg("brightness") ) {
    handleBrightness();
  }
  server.send ( 200, "text/html", getPage() );

}
