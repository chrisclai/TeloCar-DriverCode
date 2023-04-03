/*
* Ardupot Receiver Code
* Christopher Lai
* CPP Hyperloop Edition
* Last Updated: 8/26/2022
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <Servo.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define speedPin 10
#define turnPin 9

// Define Servo Object
Servo speed;
Servo angle;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

RF24 radio(7, 8); // CE, CSN
const byte address[6] = "00501";
const uint8_t  channel = 123;

bool conn = false;

// Variables
int speedPos = 93;
int anglePos = 90;

int speedHighLim = 105;
int speedLowLim = 80;
int angleHighLim = 180;
int angleLowLim = 0;

// Create struct for specialized sending code ("Smart Controller")
struct SMRT_Packet {
  byte speed;
  byte angle;
  byte mode;
  byte safety;
};

SMRT_Packet smart;

void setup() {
  // Set Up Serial Comms - Debug
  Serial.begin(115200);

  // Set up Radio
  radio.begin();
  radio.openReadingPipe(1, address);
  radio.setPALevel(RF24_PA_LOW,0);
  radio.setChannel(channel);
  Serial.println("Starting Radio");

  // Define pinouts
  speed.attach(speedPin);
  angle.attach(turnPin);

  // Default Values
  speed.write(95);
  angle.write(90);

  // Check for OLED Availability
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  //to recieve
  radio.startListening();
}

void loop() {
  
  if (radio.available()) {
    conn = true;
    radio.read(&smart, sizeof(smart));

    // Will delay code, only use if necessary
    // debug(packet);

    // Check for Serial Input (Autonomous Control)
    RPiSerial();

    // Driver Code
    if (!smart.safety)  // KILL SWITCH
    {
      kill_switch();
    }
    else if (!smart.mode) // AUTO
    {
      RPiSerial();
      thresholdCheck();
      speed.write(speedPos);
      angle.write(anglePos);  
    }
    else // MANUAL
    {
      speedPos = smart.speed;
      anglePos = smart.angle;
      speed.write(speedPos);
      angle.write(anglePos);   
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    if (conn)
    {
      display.println("RF: YES");
    }
    else
    {
      display.println("RF: NO");
    }
    display.setCursor(64,0);
    if (!smart.mode)
    {
      display.println("M: AUTO");
    }
    else
    {
      display.println("M: MANUAL");
    }
    
    display.setCursor(0,12);
    display.println("ANGLE: " + String(anglePos));
    display.setCursor(0,24);
    display.println("SPEED: " + String(speedPos));
    display.display();
    
    delay(50);
    
  }
  else {
    Serial.println("NO CONNECTION");
    conn = false;

    kill_switch();

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,12);
    display.println("NO CONNECT");
    display.display();
  }
}

// [Function] KILL SWITCH ACTION FOR EMERGENCIES
void kill_switch()
{
  speedPos = 93;
  anglePos = 90;
  speed.write(speedPos);
  angle.write(anglePos);
}

// [Function] Checks to see if speed or angle is higher or lower than threshold
void thresholdCheck()
{
  if (speedPos > speedHighLim)
  {
    speedPos = speedHighLim;
  }
  else if (speedPos < speedLowLim)
  {
    speedPos = speedLowLim;
  }

  if (anglePos > angleHighLim)
  {
    anglePos = angleHighLim;
  }
  else if (anglePos < angleLowLim)
  {
    anglePos = angleLowLim;
  }
}

// [Function] Checks to see if serial data has been recieved from the Raspberry Pi
void RPiSerial()
{
  if (Serial.available() > 0)
  {
    char command = Serial.read();
    // Currently, if string recieved == "brakeon", turn LED on, vice versa for off
    if (command == 'f')
    {
      speedPos += 1;
    }
    else if (command == 'l')
    {
      anglePos -= 10;
    }
    else if (command == 'b')
    {
      speedPos -= 1;
    }
    else if (command == 'c')
    {
      anglePos += 10;
    }
  }
}
