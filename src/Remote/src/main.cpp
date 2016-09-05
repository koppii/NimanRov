/*
 NIMAN SUB ROV - Remote control

 NimanRov is a project to build a Remotely Operated underwater Vehicle.
 This is the code which will run on the arduino board in the ROV itself.

 */

#include "Arduino.h"
#include <SPI.h>
#include <Ethernet.h>

// ETHERNET SETUP
// Enter a MAC address for your controller below.
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0E, 0xFE, 0x40 };

// set IP address of client
IPAddress ip(192,168,3,199);

// set IP address of server:
IPAddress server(192,168,3,3);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

/* Not used
unsigned long lastConnectionTime = 0;             // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10L * 1000L; // max delay between updates, in milliseconds
// the "L" is needed to use long type numbers
*/

#include <JoystickShield.h> // include JoystickShield Library
JoystickShield joystickShield; // create an instance of JoystickShield object

// running average for joystick values
bool RUNNINGAVERAGE = false;
const int numReadings = 5;
int readIndex = 0;
int readingsHX[numReadings];
int totalHX = 0;
int lastHX = 0;
int readingsHY[numReadings];
int totalHY = 0;
int lastHY = 0;
int lastHGear = 0;
int readingsVX[numReadings];
int totalVX = 0;
int lastVX = 0;
int readingsVY[numReadings];
int totalVY = 0;
int lastVY = 0;

// min delay between shifting gears up / down
const unsigned long gearShiftMinDelay = 1L * 1000L; // min delay between updates, in milliseconds
unsigned long lastGearShiftDown = 0;
unsigned long lastGearShiftUp = 0;

bool DEBUG = true;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // start the Ethernet connection:
  // congifure using IP address instead of DHCP:
  Ethernet.begin(mac, ip);
  // give the Ethernet shield a second to initialize:
  delay(1000);
  // print the Ethernet board/shield's IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());

  // new calibration function
  Serial.println("Calibrating JoyStick");
  joystickShield.calibrateJoystick();
  // predefined Joystick to Pins 0 and 1.
  // Change it if you are using a different shield
  // setJoystickPins(0, 1);
  delay(1000);
  // predefined buttons to the following pins.
  // change it if you are using a different shield.
  // setButtonPins(pinJoystickButton, pinUp, pinRight, pinDown, pinLeft, pinF, pinE);
  // to deactivate a button use a pin outside of the range of the arduino e.g. 255, but not above
  // setButtonPins(8, 2, 3, 4, 5, 7, 6);

}



// this method makes a HTTP connection to the server:
void httpRequest(int hx, int hy, int hgear) {
  /*Serial.print("x: ");
  Serial.print(hx);
  Serial.print(", y: ");
  Serial.println(hy);*/

  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(server, 80)) {
    // send the HTTP PUT request:
    String url = "/?hx=" + String(hx) + "&hy=" + String(hy) + "&hgear=" + String(hgear);
    Serial.println("Calling URL: " + url);
    client.println("GET " + url + " HTTP/1.0");
    client.println("User-Agent: nimanrov-remote");
    client.println("Connection: close");
    client.println();

  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:

  delay(50);
  if (DEBUG) {
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
    }
  }

    // note the time that the connection was made:
//    lastConnectionTime = millis();
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}


void loop() {

  joystickShield.processEvents(); // process events

  int hx = 0;
  int hy = 0;
  int hgear = 0;
  if (RUNNINGAVERAGE) {
    totalHX = totalHX - readingsHX[readIndex];
    readingsHX[readIndex] = joystickShield.xAmplitude();
    totalHX = totalHX + readingsHX[readIndex];
    hx = totalHX/numReadings;

    totalHY = totalHY - readingsHY[readIndex];
    readingsHY[readIndex] = joystickShield.yAmplitude();
    totalHY = totalHY + readingsHY[readIndex];
    hy = totalHY/numReadings;

    readIndex = readIndex + 1;
    if (readIndex >= numReadings) {
      readIndex = 0;
    }

  } else {
    hx = joystickShield.xAmplitude();
    hy = joystickShield.yAmplitude();
  }

  // Shift gear up or down
  if (joystickShield.isEButton()) {
    if (millis() - lastGearShiftUp > gearShiftMinDelay) {
      if (DEBUG) {
        Serial.print(F("E Button Clicked, shift gear up"));
        Serial.println(millis());
      }
      hgear = 1;
      lastGearShiftUp = millis();
    }
  }
  if (joystickShield.isFButton()) {
    if (millis() - lastGearShiftDown > gearShiftMinDelay) {
      Serial.println("F Button Clicked, shift gear down") ;
      hgear = -1;
      lastGearShiftDown = millis();
    }
  }

  if (hx != lastHX || hy != lastHY || hgear != lastHGear) { // || millis() - lastConnectionTime > postingInterval) {
    httpRequest(hx, hy, hgear);
    lastHX=hx;
    lastHY=hy;
    lastHGear=hgear;
  } else {
    if (DEBUG) {
      //Serial.println("Coordinates did not change");
    }
  }
  delay(50);


/*
  // if ten seconds have passed since your last connection,
  // then connect again and send data:
  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  }
*/
}
