#include "Motors.h"
#include <Arduino.h>

// Configuration of the 4 motors
// this should probably go into some settings.h
struct motorParams motorParamsData[4] = {
  {  2,  3,  4, "hleft" },
  {  5,  6,  7, "hright" },
  {  8,  9, 10, "vleft" },
  { 11, 12, 13, "vright" }
};


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
