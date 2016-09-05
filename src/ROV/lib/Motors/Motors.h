#ifndef _MOTORS_
#define _MOTORS_

bool DEBUG = true;

// Motor specific parametros
const int HLEFTMOTOR=0;
const int HRIGHTMOTOR=1;
const int VLEFTMOTOR=2;
const int VRIGHTMOTOR=3;

const int MOTORSTARTUS=1060;
const int MOTORENDUS=1860;
//const int MOTORRANGE=(MOTORENDUS-MOTORSTARTUS)/2;
const int MOTORSTOP=MOTORSTARTUS+((MOTORENDUS-MOTORSTARTUS)/2);
const int MAXGEAR=4;

// structure to store parameters for each motor
// input pins of L293 and name
struct motorParams {
  int pin;
  int lastDir;
  char name[10];
};

// struct to store directions and power level of a pair of motors
struct motorPairParams {
  int dir1;
  int power1;
  int dir2;
  int power2;
  int gear;
};

// setup Motors, set pins to input/output mode, etc.
void setupMotors();

// set direction and power level of a single motor
void setMotor();
void setMotor(int id, int dir, int power, int gear);

// calculate and set directions and power levels of horizontal motors
void horizontalMotors(int x, int y, int gear);

// calculate and set directions and power levels of vertical motors
void verticalMotors(int x, int y, int gear);

// calculate directions and power levels of a pair of motors
struct motorPairParams calcMotorParams(struct motorPairParams mParams, int x, int y, int gear);

#endif
