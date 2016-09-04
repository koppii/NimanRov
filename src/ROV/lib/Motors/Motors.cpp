#include "Motors.h"
#include <Arduino.h>
#include <Servo.h>

// Configuration of the 4 motors
// this should probably go into some settings.h
struct motorParams motorParamsData[4] = {
  {  8, 0, "hleft" },
  {  9, 0, "hright" },
  { 11, 0, "vleft" },
  { 12, 0, "vright" }
};

struct motorPairParams hMotorParams;
struct motorPairParams vMotorParams;

Servo motors[4];

void setupMotors() {
  Serial.println(F("Setting up motors ..."));
  // TODO: for loop over all motors ...
  for (int i = 0; i < 4; i = i+1) {
    motors[i].attach(motorParamsData[i].pin);
    setMotor(i, 0, 0, 1);
  }
}

// set direction and power level of a single motor
void setMotor(int id, int dir, int power, int gear) {
  Serial.print(F("Setting motor "));
  Serial.print(motorParamsData[id].name);
  Serial.print(F(" in direction "));
  Serial.print(dir);
  Serial.print(F(" with power "));
  Serial.println(power);
  Serial.print(F(" with gear "));
  Serial.println(gear);


  int us = MOTORSTOP;
  // we change the direction, so stop the motor first
  /*if (dir != 0 && dir != motorParamsData[id].lastDir) {
    for (int i; i < 10; i = i+1) {
      motors[id].writeMicroseconds(MOTORSTOP);
      delay(10);
    }
  }
  */
  int range = (MOTORENDUS-MOTORSTOP)/MAXGEAR*gear;
  if (dir == 1) {
    us = MOTORSTOP+(range*power/100);
  } else if (dir == -1) {
    us = MOTORSTOP-(range*power/100);
  }
  Serial.print(F("writeMicroseconds: "));
  Serial.println(us);
  motors[id].writeMicroseconds(us);
}

// calculate and set directions and power levels of horizontal motors
void horizontalMotors(int x, int y, int gear) {
  Serial.print(F("horizontalMotors: x="));
  Serial.print(x);
  Serial.print(F(", y="));
  Serial.println(y);
  Serial.print(F(", gear="));
  Serial.println(gear);

  hMotorParams = calcMotorParams(hMotorParams, x, y, gear);

  setMotor(HLEFTMOTOR,  hMotorParams.dir1, hMotorParams.power1, hMotorParams.gear);
  setMotor(HRIGHTMOTOR, hMotorParams.dir2, hMotorParams.power2, hMotorParams.gear);

}

// calculate and set directions and power levels of vertical motors
void verticalMotors(int x, int y, int gear) {
  Serial.print(F("verticalMotors: x="));
  Serial.print(x);
  Serial.print(F(", y="));
  Serial.println(y);
  Serial.print(F(", gear="));
  Serial.println(gear);

  vMotorParams = calcMotorParams(vMotorParams, x, y, gear);

  setMotor(VLEFTMOTOR,  vMotorParams.dir1, vMotorParams.power1, vMotorParams.gear);
  setMotor(VRIGHTMOTOR, vMotorParams.dir2, vMotorParams.power2, vMotorParams.gear);
}

// calculate directions and power levels of a pair of motors
struct motorPairParams calcMotorParams(struct motorPairParams mParams, int x, int y, int gear) {
  //struct motorPairParams mParams;

  mParams.dir1=0;
  mParams.power1=0;
  mParams.dir2=0;
  mParams.power2=0;

  // set gear
  if (gear == 1 && mParams.gear < MAXGEAR) {
    mParams.gear++;
  }
  if (gear == -1 && mParams.gear > 0) {
    mParams.gear--;
  }
  int threshold=5;
  int maxVal=100;
  int minVal=-100;
  if (x > maxVal) { x = maxVal; }
  if (x < minVal) { x = minVal; }
  if (y > maxVal) { y = maxVal; }
  if (y < minVal) { y = minVal; }

  // forward
  if (y > threshold) {
    mParams.dir1=1;
    mParams.dir2=1;
    // fwd right
    if (x > threshold) {
      mParams.power1=y;
      mParams.power2=(100-x)*y/100;
    // fwd left
    } else if (x < 0-threshold) {
      mParams.power1=(100+x)*y/100;
      mParams.power2=y;
    // fwd straight
    } else {
      mParams.power1=y;
      mParams.power2=y;
    }
// back
} else if (y < 0-threshold) {
    mParams.dir1=-1;
    mParams.dir2=-1;
    // back right
    if (x > threshold) {
      mParams.power1=-y;
      mParams.power2=(100-x)*-y/100;
    } else if (x < 0-threshold) {
      mParams.power1=(100+x)*-y/100;
      mParams.power2=-y;
    } else {
      mParams.power1=-y;
      mParams.power2=-y;
    }
  // rotate on site
  } else {
    if (x > threshold) {
      mParams.dir1=1;
      mParams.dir2=-1;
      mParams.power1=x;
      mParams.power2=x;
    } else if (x < 0-threshold) {
      mParams.dir1=-1;
      mParams.dir2=1;
      mParams.power1=-x;
      mParams.power2=-x;
    }
  }
  return mParams;
}
