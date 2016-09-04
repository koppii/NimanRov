/*
 NIMAN SUB ROV

 NimanRov is a project to build a Remotely Operated underwater Vehicle.
 This is the code which will run on the arduino board in the ROV itself.


 Acknowledgements:

 Based on Ethernet WebServer Example: https://www.arduino.cc/en/Tutorial/WebServer
 Parsing of GET requests: based on http://arduino.stackexchange.com/questions/13388/arduino-webserver-faster-alternative-to-indexof-for-parsing-get-requests
 and http://arduino.stackexchange.com/questions/1013/how-do-i-split-an-incoming-string

 */

#include "Arduino.h"
#include <SPI.h>
#include <Ethernet.h>

#include "UrlParser.h"
#include "Motors.h"
#include "Commands.h"
#include <Servo.h>


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 3, 3);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print(F("server is at "));
  Serial.println(Ethernet.localIP());

  // setup Motors
  setupMotors();

}

// get ROV status
String getRovStatus() {
  String rovStatus = "{ status: OK }";
  return rovStatus;
}

void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();

  // we get a request
  if (client) {
    Serial.println(F("Client connected"));
    // an http request ends with a blank line
    boolean done = false;
    while (client.connected() && !done) {
      while (client.available () > 0 && !done) {
        done = processIncomingByte (client.read ());
      }
    }  // end of while client connected

    // get ROV status values as json string
    String rovStatus = getRovStatus();

    // send a standard http response header
    client.println(F("HTTP/1.1 200 OK"));
    client.println(F("Content-Type: text/json"));
    client.println(F("Connection: close"));  // close after completion of the response
    client.println();   // end of HTTP header
    client.println(rovStatus);

    // give the web browser time to receive the data
    delay(10);
    // close the connection:
    client.stop();
    Serial.println(F("Client disconnected"));
  }  // end of got a new client
}  // end of loop
