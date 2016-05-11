#include "UrlParser.h"
#include "Motors.h"
#include "Commands.h"

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

  // Read each key value pair
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

        processCommand(key, value);

        // Do something with key and value of the current command

    }
    // Find the next command in input string
    command = strtok(0, "&");
  }

  postProcessCommand();


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
