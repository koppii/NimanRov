#include <Arduino.h>
#include "Motors.h"

// Values for horizontal motors:
int hx = 0;
int hy = 0;
// values for vertical motors:
int vx = 0;
int vy = 0;

void processCommand(char* key, char* value) {
  //Serial.println("processing command ...");

  // save coordinates receveived from the joystick
  // they are processed later together
  if (strcmp(key,"hx")==0) { hx = atoi(value); }
  if (strcmp(key,"hy")==0) { hy = atoi(value); }
  if (strcmp(key,"vx")==0) { vx = atoi(value); }
  if (strcmp(key,"vy")==0) { vy = atoi(value); }

}

void postProcessCommand() {
  // send coordinates to motors
  horizontalMotors(hx, hy);
  verticalMotors(vx, vy);
}
