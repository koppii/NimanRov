#ifndef _URLPARSER_
#define _URLPARSER_

#include <Arduino.h>

// how much serial data we expect before a newline
const unsigned int MAX_INPUT = 200;
// the maximum length of paramters we accept
const int MAX_PARAM = 100;

void processGet ();
void processData ();
bool processIncomingByte (const byte inByte);

#endif
