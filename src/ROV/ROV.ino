/*
 NIMAN SUB ROV 

 NimanRov is a project to build a Remotely Operated underwater Vehicle.
 This is the code which will run on the arduino board in the ROV itself.


 Acknowledgements:

 Based on Ethernet WebServer Example: https://www.arduino.cc/en/Tutorial/WebServer
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
const int HLEFTMOTOR=0;
const int HRIGHTMOTOR=1;
const int VLEFTMOTOR=2;
const int VRIGHTMOTOR=3;

// structure to store parameters for each motor
// input pins of L293 and name
struct motorParams {
  int pinIn1;
  int pinIn2;
  int enablePin;
  char name[10];
};

// Configuration of the 4 motors
struct motorParams motorParamsData[4] = {
  {  2,  3,  4, "hleft" },
  {  5,  6,  7, "hright" },
  {  8,  9, 10, "vleft" },
  { 11, 12, 13, "vright" }
};

// struct to store directions and power level of a pair of motors
struct motorPairParams {
  int dir1;
  int power1;
  int dir2;
  int power2;
};

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

// get ROV status
String getRovStatus() {
  String rovStatus = "{ status: OK }";

  return rovStatus;
}

// set direction and power level of a single motor
void setMotor(int id, int dir, int power) {
  Serial.print(F("Setting motor "));
  Serial.print(motorParamsData[id].name); 
  Serial.print(F(" in direction "));
  Serial.print(dir);
  Serial.print(F(" with power "));
  Serial.println(power);
}

// calculate and set directions and power levels of horizontal motors
void horizontalMotors(int x, int y) {
  Serial.print(F("horizontalMotors: x="));
  Serial.print(x);
  Serial.print(F(", y="));
  Serial.println(y);

  struct motorPairParams hMotorParams = calcMotorParams(x, y);

  setMotor(HLEFTMOTOR,  hMotorParams.dir1, hMotorParams.power1);
  setMotor(HRIGHTMOTOR, hMotorParams.dir2, hMotorParams.power2);

}

// calculate and set directions and power levels of vertical motors
void verticalMotors(int x, int y) {
  Serial.print(F("verticalMotors: x="));
  Serial.print(x);
  Serial.print(F(", y="));
  Serial.println(y);

  struct motorPairParams vMotorParams = calcMotorParams(x, y);

  setMotor(VLEFTMOTOR,  vMotorParams.dir1, vMotorParams.power1);
  setMotor(VRIGHTMOTOR, vMotorParams.dir2, vMotorParams.power2);
}

// calculate directions and power levels of a pair of motors
struct motorPairParams calcMotorParams(int x, int y) {
  struct motorPairParams mParams;
  
  mParams.dir1=0;
  mParams.power1=0;
  mParams.dir2=0;
  mParams.power2=0;

  int threshold=20;
  int maxVal=100;
  int minVal=-100;
  if (x > maxVal) { x = maxVal; }
  if (x < minVal) { x = minVal; }
  if (y > maxVal) { y = maxVal; }
  if (y < minVal) { y = minVal; }
  
  // forward
  if (x > threshold) {
    mParams.dir1=1;
    mParams.dir2=1;
    // fwd right
    if (y > threshold) {
      mParams.power1=x;
      mParams.power2=(100-y)*x/100;
    } else if (y < 0-threshold) {
      mParams.power1=(100+y)*x/100;
      mParams.power2=x;
    } else {
      mParams.power1=x;
      mParams.power2=x;
    }
  // back
  } else if (x < 0-threshold) {
    mParams.dir1=-1;
    mParams.dir2=-1;
    if (y > threshold) {
      mParams.power1=-x;
      mParams.power2=(100-y)*-x/100;
    } else if (y < 0-threshold) {
      mParams.power1=(100+y)*-x/100;
      mParams.power2=-x;
    } else {
      mParams.power1=-x;
      mParams.power2=-x;
    }
  // rotate on site
  } else {
    if (y > threshold) {
      mParams.dir1=1;
      mParams.dir2=-1;
      mParams.power1=y;
      mParams.power2=y;
    } else if (y < 0-threshold) {
      mParams.dir1=-1;
      mParams.dir2=1;
      mParams.power1=-y;
      mParams.power2=-y;
    }
  }
  return mParams;
}

// Example GET line: GET /?hx=100&hy=25&vx=0&vy=0 HTTP/1.1
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

        // coordinates receveived from the joystick
        if (strcmp(key,"hx")==0) { hx = atoi(value); }
        if (strcmp(key,"hy")==0) { hy = atoi(value); }
        if (strcmp(key,"vx")==0) { vx = atoi(value); }
        if (strcmp(key,"vy")==0) { vy = atoi(value); }
    }
    // Find the next command in input string
    command = strtok(0, "&");
  }

  // send coordinates to motors
  horizontalMotors(hx, hy);
  verticalMotors(vx, vy);
  
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
//    client.println(F("{ foo: bar }"));

    // give the web browser time to receive the data
    delay(10);
    // close the connection:
    client.stop();
    Serial.println(F("Client disconnected"));
  }  // end of got a new client
}  // end of loop

