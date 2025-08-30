#include <math.h>
#include <AccelStepper.h>
#include <Servo.h>

Servo myservo;
Servo pumpSwitch;
Servo valveSwitch;
AccelStepper stepperX(AccelStepper::DRIVER, 2, 5);
AccelStepper stepperY(AccelStepper::DRIVER, 3, 6);
AccelStepper stepperZ(AccelStepper::DRIVER, 4, 7);
const String dropatPrefix = "Dropat ";
const String pickupPrefix = "Pickup ";
const char delimiterString[] = " Z ";

const int MAX_COORDINATES = 10;

float pickupCoordinates[MAX_COORDINATES][3];
int pickupCount = 0;
float dropatCoordinates[MAX_COORDINATES][3];
int dropatCount = 0;

// Initializes serial communication for debugging and receiving commands.
void initializeSerial() {
  Serial.begin(9600);
}

// Checks for and processes incoming serial data.
void handleSerialInput() {
  if (Serial.available() > 0) {
    String receivedString = Serial.readStringUntil('\n');
    receivedString.trim();

    processCoordinateCommand(receivedString);
    checkYommamaCondition();

  }
}

// Parses a string command to extract and store coordinate data in the appropriate arrays.
void processCoordinateCommand(String receivedCommand) {
  String coordinateData = "";
  bool commandRecognized = false;

  if (receivedCommand.startsWith(pickupPrefix)) {
    coordinateData = receivedCommand.substring(pickupPrefix.length());
    pickupCount = 0;
    commandRecognized = true;

    int lastIndex = 0;
    int currentIndex = 0;

    if (coordinateData.length() > 0) {
      while (pickupCount < MAX_COORDINATES) {
        currentIndex = coordinateData.indexOf(delimiterString, lastIndex);

        String coordSetString;
        if (currentIndex == -1) {
          coordSetString = coordinateData.substring(lastIndex);
          coordSetString.trim();
        } else {
          coordSetString = coordinateData.substring(lastIndex, currentIndex);
          coordSetString.trim();
        }

        if (coordSetString.length() > 0) {
          int spaceIndex1 = coordSetString.indexOf(' ');
          int spaceIndex2 = coordSetString.indexOf(' ', spaceIndex1 + 1);

          if (spaceIndex1 != -1 && spaceIndex2 != -1) {
            float x_coord = coordSetString.substring(0, spaceIndex1).toFloat();
            float y_coord = coordSetString.substring(spaceIndex1 + 1, spaceIndex2).toFloat();
            float z_coord = coordSetString.substring(spaceIndex2 + 1).toFloat();

            pickupCoordinates[pickupCount][0] = x_coord;
            pickupCoordinates[pickupCount][1] = y_coord;
            pickupCoordinates[pickupCount][2] = z_coord;
            pickupCount++;
          }
        }

        if (currentIndex == -1) {
          break;
        }
        lastIndex = currentIndex + String(delimiterString).length();
      }
    }
  } else if (receivedCommand.startsWith(dropatPrefix)) {
    coordinateData = receivedCommand.substring(dropatPrefix.length());
    dropatCount = 0;
    commandRecognized = true;

    int lastIndex = 0;
    int currentIndex = 0;

    if (coordinateData.length() > 0) {
      while (dropatCount < MAX_COORDINATES) {
        currentIndex = coordinateData.indexOf(delimiterString, lastIndex);

        String coordSetString;
        if (currentIndex == -1) {
          coordSetString = coordinateData.substring(lastIndex);
          coordSetString.trim();
        } else {
          coordSetString = coordinateData.substring(lastIndex, currentIndex);
          coordSetString.trim();
        }

        if (coordSetString.length() > 0) {
          int spaceIndex1 = coordSetString.indexOf(' ');
          int spaceIndex2 = coordSetString.indexOf(' ', spaceIndex1 + 1);

          if (spaceIndex1 != -1 && spaceIndex2 != -1) {
            float x_coord = coordSetString.substring(0, spaceIndex1).toFloat();
            float y_coord = coordSetString.substring(spaceIndex1 + 1, spaceIndex2).toFloat();
            float z_coord = coordSetString.substring(spaceIndex2 + 1).toFloat();

            dropatCoordinates[dropatCount][0] = x_coord;
            dropatCoordinates[dropatCount][1] = y_coord;
            dropatCoordinates[dropatCount][2] = z_coord;
            dropatCount++;
          }
        }

        if (currentIndex == -1) {
          break;
        }
        lastIndex = currentIndex + String(delimiterString).length();
      }
    }
  }
}

float oldRad[3] = {1.57, 1.69, 0};
float newRad[3] = {0,0,0};
bool halt = false;
int refH = 0;


// Pauses execution until all stepper motors have finished their movements.
void waitForSteppers() {
  while (stepperX.isRunning() || stepperY.isRunning() || stepperZ.isRunning()) {
    stepperX.run();
    stepperY.run();
    stepperZ.run();
  }
}

// Calculates an angle of a triangle using the Law of Cosines.
float cosineLaw (float a, float b, float c) {
  double cosA = (b*b + c*c - a*a) / (2.0 * b * c);
  return acos(cosA);
}

// Converts a radian value to the equivalent number of steps for a given motor.
int radianToSteps(float radianInput, int motorInput) {
  if (motorInput == 2) {
    return static_cast<int>(1400 * radianInput / -3.1416);
  } else {
    return static_cast<int>(2100 * radianInput / -3.1416);
  }
}

// Converts a radian value to degrees.
int radianToDegrees(float radians) {
  return round(radians * 180.0 / PI);
}

// Adjusts an angle to be within the valid range of 0-180 degrees.
int angleAdjust (int input){
  int inputVer = (((input) % 360) + 360) % 360;
  return inputVer % 180;
}

// Checks if a given coordinate is within the arm's physical working boundaries.
bool outBounds(float x, float y, float z){
  int maxRadius = 300*300;
  int minRadius = 150*150;

  if((x<0)|| (y<0)){
    return true;
  }

  if (y>260){
    return true;
  }

  float circMotto = x*x + z*z;
  if((circMotto >maxRadius) || (circMotto < minRadius)){
    return true;
  }

  return false;
}

// Calculates the required joint angles (inverse kinematics) to reach a given Cartesian coordinate.
void inKin(float xCoor, float yCoor, float zCoor){

  float dirRad[3] = {0,0,0};
  dirRad[2] = atan2(xCoor,zCoor);

  yCoor = yCoor - 75;
  xCoor = xCoor - 41.644*sin(dirRad[2]);
  zCoor = zCoor - 41.644*cos(dirRad[2]);
  float magnitude = sqrt(xCoor*xCoor + yCoor*yCoor + zCoor*zCoor);

  dirRad[1] = cosineLaw(magnitude,220,177.5);

  dirRad[0] = asin(yCoor/magnitude) + cosineLaw(220,177.5,magnitude);

  for(int i=0;i<3;i++){
    newRad[i] = dirRad[i];
  }
}

// Adjusts stepper motor speeds and accelerations for proportional, linear movement.
void linProp(int xStepsC, int yStepsC, int zStepsC) {
  float stepMP = 800;
  float stepAc = 2000;

  int xAbs = abs(xStepsC);
  int yAbs = abs(yStepsC);
  int zAbs = abs(zStepsC);

  int maxSteps = max(xAbs, max(yAbs, zAbs));

  if (xAbs != 0) {
    float ratioX = (float)maxSteps / xAbs;
    stepperX.setMaxSpeed(stepMP / ratioX);
    stepperX.setAcceleration(stepAc / ratioX);
    stepperX.move(xStepsC);
  }

  if (yAbs != 0) {
    float ratioY = (float)maxSteps / yAbs;
    stepperY.setMaxSpeed(stepMP / ratioY);
    stepperY.setAcceleration(stepAc / ratioY);
    stepperY.move(yStepsC);
  }

  if (zAbs != 0) {
    float ratioZ = (float)maxSteps / zAbs;
    stepperZ.setMaxSpeed(stepMP / ratioZ);
    stepperZ.setAcceleration(stepAc / ratioZ);
    stepperZ.move(zStepsC);
  }
}

// Calculates and executes the stepper motor movements required to move from the old angles to the new angles.
void pivotTo(){
  float dirChange[3] = {0,0,0};
  for(int i=0;i<3;i++){
    dirChange[i] = newRad[i] - oldRad[i];
  }
  for(int i=0;i<3;i++){
    oldRad[i] = newRad[i];
  }
  for(int i=0;i<3;i++){
    dirChange[i] = radianToSteps(dirChange[i]*-1,i);
  }
  linProp(dirChange[0],dirChange[1],dirChange[2]);
}

// Activates the pump and valve servos to open.
void pumpOpen(){
  pumpSwitch.writeMicroseconds(2500);
  valveSwitch.writeMicroseconds(2500);
  delay(100);
}

// Deactivates the pump and valve servos to close.
void pumpClose(){
  pumpSwitch.writeMicroseconds(500);
  valveSwitch.writeMicroseconds(500);
  delay(100);
}

// Moves the servo to a specified angle relative to a reference.
void servoMove(int angle){
  angle = angle - radianToDegrees(oldRad[2]);
  angle = angleAdjust(angle);
  int delayTime = round((angle/60)*170);
  myservo.write(angle);
  delay(delayTime);
  delay(250);
}

// Executes the full sequence of movements to move the arm to a new coordinate.
void moveTo(float x, float y, float z){
  inKin(x,y,z);
  if(halt){
    return;
  }
  pivotTo();
  waitForSteppers();
  delay(250);
}

// Performs the full sequence for picking up an object at a given location.
void pickUp(float x,float y,float z,int degree){
  moveTo(x,refH,z);
  pumpOpen();
  delay(500);
  servoMove(degree);
  moveTo(x,y,z);
  delay(400);
  pumpClose();
  moveTo(x,refH,z);
}

// Performs the full sequence for dropping an object at a given location.
void dropAt(float x,float y,float z,int degree){
  moveTo(x,refH,z);
  delay(500);
  servoMove(degree);
  moveTo(x,y,z);
  pumpOpen();
  delay(400);
  moveTo(x,refH,z);
  pumpClose();
}

// Initializes the servos and performs an initial homing/starting movement for the arm.
void start(){
  myservo.attach(11);
  pumpSwitch.attach(10);
  valveSwitch.attach(9);
  refH = 250;
  linProp(-460, 860, 0);
  waitForSteppers();
  delay(250);
}

int jOne[3] = {0,0,0};
int jTwo[3] = {90,90,90};

// Checks if both pickup and drop-at coordinates have been received and, if so, executes the palletizing sequence.
void checkYommamaCondition() {
  if (pickupCount > 0 && dropatCount> 0) {
    for (int i = 0; i < pickupCount; i++) {
      pickUp(pickupCoordinates[i][0]*-1,pickupCoordinates[i][1],pickupCoordinates[i][2],jOne[i]);
      dropAt(dropatCoordinates[i][0]*-1,dropatCoordinates[i][1],dropatCoordinates[i][2],jTwo[i]);
    }
    pickupCount = 0;
    dropatCount = 0;
  }
}


// Arduino setup function, called once on startup.
void setup() {
  initializeSerial();
  start();
  myservo.write(90);

}


// Arduino main loop, which continuously checks for serial input.
void loop() {
  handleSerialInput();

}
