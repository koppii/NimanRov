#ifndef _MOTORS_
#define _MOTORS_

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



// struct to store directions and power level of a pair of motors
struct motorPairParams {
  int dir1;
  int power1;
  int dir2;
  int power2;
};


// set direction and power level of a single motor
void setMotor();

// calculate and set directions and power levels of horizontal motors
void horizontalMotors(int x, int y);

// calculate and set directions and power levels of vertical motors
void verticalMotors(int x, int y);

// calculate directions and power levels of a pair of motors
struct motorPairParams calcMotorParams(int x, int y);

#endif
