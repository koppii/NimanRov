/*
  NIMAN SUB ROV 

*/

/*

 Parsing of GET requests: based on http://arduino.stackexchange.com/questions/13388/arduino-webserver-faster-alternative-to-indexof-for-parsing-get-requests
 and http://arduino.stackexchange.com/questions/1013/how-do-i-split-an-incoming-string

 */


#include <SPI.h>
#include <Ethernet.h>

// how much serial data we expect before a newline
const unsigned int MAX_INPUT = 200;
// the maximum length of paramters we accept
const int MAX_PARAM = 100;

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


// Motor specific parametros
const int LEFTMOTOR=0;
const int RIGHTMOTOR=1;

int motorPins[] = { 3, 5 };
char* motorNames[]= { "left", "right" };

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
}

void setMotor(int id, int dir, int power) {
  Serial.print(F("Setting motor "));
  Serial.print(motorNames[id]); 
  Serial.print(F(" in direction "));
  Serial.print(dir);
  Serial.print(F(" with power "));
  Serial.println(power);
}

void horizontalMotors(int x, int y) {
  Serial.print(F("horizontalMotors: x="));
  Serial.print(x);
  Serial.print(F(", y="));
  Serial.println(y);

  // calculate directions and power for the horizontal motors
  int dirLeft=0;
  int powerLeft=0;
  int dirRight=0;
  int powerRight=0;

  int threshold=20;
  int maxVal=100;
  int minVal=-100;
  if (x > maxVal) { x = maxVal; }
  if (x < minVal) { x = minVal; }
  if (y > maxVal) { y = maxVal; }
  if (y < minVal) { y = minVal; }
  
  // forward
  if (x > threshold) {
    dirLeft=1;
    dirRight=1;
    // fwd right
    if (y > threshold) {
      powerLeft=x;
      powerRight=(100-y)*x/100;
    } else if (y < 0-threshold) {
      powerLeft=(100+y)*x/100;
      powerRight=x;
    } else {
      powerLeft=x;
      powerRight=x;
    }
  // back
  } else if (x < 0-threshold) {
    dirLeft=-1;
    dirRight=-1;
    if (y > threshold) {
      powerLeft=-x;
      powerRight=(100-y)*-x/100;
    } else if (y < 0-threshold) {
      powerLeft=(100+y)*-x/100;
      powerRight=-x;
    } else {
      powerLeft=-x;
      powerRight=-x;
    }
  // rotate on site
  } else {
    if (y > threshold) {
      dirLeft=1;
      dirRight=-1;
      powerLeft=y;
      powerRight=y;
    } else if (y < 0-threshold) {
      dirLeft=-1;
      dirRight=1;
      powerLeft=-y;
      powerRight=-y;
    }
  }
  setMotor(LEFTMOTOR, dirLeft, powerLeft);
  setMotor(RIGHTMOTOR, dirRight, powerRight);
}

// Example GET line: GET /?foo=bar HTTP/1.1
void processGet (const char * data) {
  //Serial.print(F("Data in processGet: "));
  //Serial.println(data);
  // find where the parameters start
  const char * paramsPos = strchr (data, '?');
  if (paramsPos == NULL)
    return;  // no parameters

  // find the trailing space
  const char * spacePos = strchr (paramsPos, ' ');
  if (spacePos == NULL)
    return;  // no space found
  // work out how long the parameters are
  int paramLength = spacePos - paramsPos - 1;
  // see if too long
  if (paramLength >= MAX_PARAM) {
    Serial.println(F("Parameters too long"));
    return;  // too long for us
  }

  // copy parameters into a buffer
  char param [MAX_PARAM];
  memcpy (param, paramsPos + 1, paramLength);  // skip the "?"
  param [paramLength] = 0;  // null terminator

  //Serial.print(F("Got param: "));
  //Serial.println(param);

  // Values for horizontal motors:
  int hx = 0;
  int hy = 0;
  // values for vertical motors:
  int vx = 0;
  int vy = 0;
  // Read each command pair 
  char* command = strtok(param, "&");
  while (command != 0) {
    //Serial.print(F("command: "));
    //Serial.println(command);
    // Split the command in two values
    char* separator = strchr(command, '=');
    if (separator != 0) {
        // Actually split the string in 2: replace ':' with 0
        *separator = 0;
        char* key = command;
        ++separator;
        char* value = separator;

        Serial.print(F("key - value: "));
        Serial.print(key);
        Serial.print(" - ");
        Serial.println(value);
        
        // Do something with key and value
        if (strcmp(key,"hx")==0) { hx = atoi(value); }
        if (strcmp(key,"hy")==0) { hy = atoi(value); }
        if (strcmp(key,"vy")==0) { vy = atoi(value); }
        if (strcmp(key,"vy")==0) { vy = atoi(value); }

    }
    // Find the next command in input string
    command = strtok(0, "&");
  }

  horizontalMotors(hx, hy);
//  verticalMotors(vx, hx);
}  // end of processGet

// here to process incoming serial data after a terminator received
void processData (const char * data) {
  //Serial.print(F("Data: "));
  //Serial.println(data);
  if (strlen (data) < 4)
    return;

  if (memcmp (data, "GET ", 4) == 0)
    processGet (&data [4]);
}  // end of processData

bool processIncomingByte (const byte inByte) {
  static char input_line [MAX_INPUT];
  static unsigned int input_pos = 0;
  switch (inByte) {
    case '\n':   // end of text
      input_line [input_pos] = 0;  // terminating null byte
      if (input_pos == 0)
        return true;   // got blank line
      // terminator reached! process input_line here ...
      processData (input_line);
      // reset buffer for next time
      input_pos = 0;  
      break;

    case '\r':   // discard carriage return
      break;

    default:
      // keep adding if not full ... allow for terminating null byte
      if (input_pos < (MAX_INPUT - 1))
        input_line [input_pos++] = inByte;
      break;
  }  // end of switch
  return false;    // don't have a blank line yet
} // end of processIncomingByte  


void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println(F("Client connected"));
    // an http request ends with a blank line
    boolean done = false;
    while (client.connected() && !done) {
      while (client.available () > 0 && !done) {
        done = processIncomingByte (client.read ());
      }
    }  // end of while client connected

    // send a standard http response header
    client.println(F("HTTP/1.1 200 OK"));
    client.println(F("Content-Type: text/json"));
    client.println(F("Connection: close"));  // close after completion of the response
    client.println();   // end of HTTP header
    client.println(F("{ foo: bar }"));

    // give the web browser time to receive the data
    delay(10);
    // close the connection:
    client.stop();
    Serial.println(F("Client disconnected"));
  }  // end of got a new client
}  // end of loop

