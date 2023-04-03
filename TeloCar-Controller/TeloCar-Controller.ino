/*
* Transmitter Code for TeloCar 
* Written by Christopher Lai
* For Dynamic Drone Racing Project
* 
* Last Updated: 2/23/2023
*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

RF24 radio(7, 8); // CE, CSN
const byte address[6] = "00501";
const uint8_t  channel = 123;

// Analog Mapping Constraints
int lowmap = 10;
int highmap = 250;
int centermap = (highmap - lowmap) / 2;

// Joystick thresholds
int joy_highthresh = 150;
int joy_lowthresh = 100;

// Speed calculations
int offset = 93; // Finds out where the center of the "dead zone" is
int boundary = 2; // Finds out how far off from the center to run the vehicle, 2 by default
int upoff = 4;
int downoff = 7;


// Real calculations
int real_angle = 0;
int real_speed = 0;

// Define the digital inputs
#define lBTN 9  // Joystick button 1
#define rBTN 10  // Joystick button 2
#define switchL 0 // Left Switch
#define switchR 1 // Right Switch
#define switchMode 5 // Mode Switch

// Max size 32 bytes because of buffer limit
struct CMD_Packet {
  byte leftstickx;
  byte leftsticky;
  byte rightstickx;
  byte rightsticky;
  byte btnleft;
  byte btnright;
  byte potleft;
  byte potright;
  byte modeSW;
  byte leftSW;
  byte rightSW;
};

// Create struct for specialized sending code ("Smart Controller")
struct SMRT_Packet {
  byte speed;
  byte angle;
  byte mode;
  byte safety;
};

// Make command packet
CMD_Packet packet;

// Make smart pakcet
SMRT_Packet smart;

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_LOW,0);
  radio.setChannel(channel);
  radio.stopListening();
  Serial.println("Sending");

  // Check for OLED Availability
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("LOADING...");
  display.setCursor(0, 20);
  display.println("Setting Up");
  display.setCursor(0, 40);
  display.println("TeloCar!");
  display.display(); 
  delay(2500);
  
  //Set Up Controls
  pinMode(lBTN, INPUT_PULLUP);
  pinMode(rBTN, INPUT_PULLUP);
  pinMode(switchL, INPUT_PULLUP);
  pinMode(switchR, INPUT_PULLUP);
  pinMode(switchMode, INPUT_PULLUP);
}

void loop() {
  
  // Get all the data from the controller inputs
  get_data();

  // Calculate data for speed, angle
  calc_control();

  // Send the whole data from the structure to the receiver
  radio.write(&smart, sizeof(smart));

  // Print stuff out to OLED upon conditions
  if (smart.safety) {OLED_MainData();}
  else {OLED_SAFETY();}
  
  delay(5);
}

// [FUNCTION] Calculate speed and angle values from controller data and assign them to smart struct
void calc_control()
{
  // Following conditionals calculate speed constraints
  if (packet.leftSW){ // If left switch is in the left position
    offset = map(packet.potleft, lowmap, highmap, 90, 96); // Offset can only be between 0-5 depending on vehicle
  }
  else{ // If left switch is in the right position
    boundary = map(packet.potleft, lowmap, highmap, 0, 20); // High/Low starts moving forward/backwards past value of 2
  }

  // Following conditionals map joystick values to speed
  if (packet.leftsticky > joy_highthresh){ // If joystick is in the up-ish position
    smart.speed = offset + upoff + map(packet.leftsticky, centermap, highmap, 0, boundary);
  }
  else if (packet.leftsticky < joy_lowthresh){ // If joystick is in the down-ish position
    smart.speed = offset - downoff - map(packet.leftsticky, centermap, lowmap, 0, boundary);
  }
  else
  {
    smart.speed = offset; // Puts car back in neutral mode
  }

  // Calculate angle
  smart.angle = map(packet.rightstickx, lowmap, highmap, 0, 180);

  // Send Mode
  smart.mode = packet.modeSW;

  // Safety Switch
  smart.safety = packet.rightSW;
  if (!smart.safety){ // If the safety is turned on, reset car states!
    smart.speed = offset;
    smart.angle = 90;
    smart.mode = 1;
  }

  // Calculate real angle and speed to implement in OLED display
  real_angle = map(smart.angle, 0, 180, -45, 45);
  real_speed = map(smart.speed, offset - boundary, offset + boundary, -boundary, boundary);
}

// [FUNCTION] Poll data from the controller and assign them to packet struct
void get_data()
{
  // Read all analog inputs and map them to one Byte value
  //Gather Left Stick Values
  packet.leftstickx = map(analogRead(A1),0,1023,lowmap,highmap);
  packet.leftsticky = map(analogRead(A0),0,1023,highmap,lowmap);
  
  //Gather Right Stick Values
  packet.rightstickx = map(analogRead(A3),0,1023,lowmap,highmap);
  packet.rightsticky = map(analogRead(A2),0,1023,highmap,lowmap);
  
  // Read all digital inputs
  packet.btnleft = digitalRead(lBTN);
  packet.btnright = digitalRead(rBTN);
  packet.modeSW = digitalRead(switchMode);
  packet.leftSW = digitalRead(switchL);
  packet.rightSW = digitalRead(switchR);

  // Read potentiometer values and assign them to packet
  packet.potleft = map(analogRead(A6),0,1023,highmap,lowmap);
  packet.potright = map(analogRead(A7),0,1023,highmap,lowmap);
}

void OLED_display()
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(15,0);
  display.println("CONTROLS");
  display.setTextSize(1);
  display.setCursor(0,20);
  display.println("LX: " + String(packet.leftstickx));
  display.setCursor(0,30);
  display.println("LY: " + String(packet.leftsticky));
  display.setCursor(0,40);
  display.println("RX: " + String(packet.rightstickx));
  display.setCursor(0,50);
  display.println("RY: " + String(packet.rightsticky));
  display.setCursor(64,20);
  display.println("BTN: " + String(packet.btnleft) + " | " + String(packet.btnright));
  display.setCursor(64,30);
  display.println("SW: " + String(packet.leftSW) + " | " + String(packet.rightSW));
  display.setCursor(64,40);
  display.println("POTL: " + String(packet.potleft));
  display.setCursor(64,50);
  display.println("POTR: " + String(packet.potright));
  display.display();
}

void OLED_MainData()
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(15,0);
  display.println("CONTROLS");
  display.setTextSize(1);
  display.setCursor(0,20);
  display.println("SPEED: " + String(real_speed));
  display.setCursor(0,30);
  display.println("ANGLE: " + String(real_angle));
  display.setCursor(0,40);
  if (smart.mode) {display.println("MODE: M");}
  else {display.println("MODE: A");}
  display.setCursor(0, 50);
  display.println("RF CONN");
  display.setCursor(64,20);
  display.println("OFFSET: " + String(offset));
  display.setCursor(64,30);
  display.println("BOUND: " + String(boundary));
  display.setCursor(64,40);
  display.println("R_SP: " + String(smart.speed));
  display.setCursor(64,50);
  display.println("R_AN: " + String(smart.angle));
  display.display();
}

void OLED_SAFETY()
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(20,0);
  display.println("WARNING");
  display.setCursor(0,20);
  display.println("SAFETY ON");
  display.setCursor(0,40);
  display.println("NO DRIVE");
  display.display();
}
