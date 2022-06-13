/**
 * @file ecu.ino
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-05-15
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <Arduino.h>

#define i_pinPedalRed A0
#define i_pinPedalBlack A1
#define o_pinThrottleRight 6
#define o_pinThrottleLeft 5
#define i_pinAngleSterringWheel 16
#define i_pinBackwards 19
#define i_pinNeutral 20
#define i_pinDrive 21
#define i_pinBrakeSignal 12
#define o_pinBrakeanRight 11
#define o_pinBrakeanLeft 10
#define i_pinDifferentialLock 13
#define i_pinSlowDrive 7
#define i_pinFastDrive 8
#define o_pinFootSwitchRight 4
#define o_pinFootSwitchLeft 3
#define i_pinVelocity 17


// Settings
#define fullThrottle 840
#define noThrottle 900

#define noThrottleBlack 810
#define fullThrottleBlack 860

#define angleMiddle 510     // Input from the analog pin, when steering wheel is centered
#define angleLeft 750         // Input, when steering wheel is all the way to the left
#define angleRight 290       // Input, when steering wheel is all the way to the right
#define maxSteeringOutput 30 // max. position of the wheels at maximum steering input in degree

// function prototypes
int velocityControl(int, int, int);
int sanityCheck();
int initPins();
void startUp();
void driveLoop();
void enableDrive();

// global vars
bool driveEnabled = false;
bool throttleEnabled = false;
double velocity = 0.0; // in m/s
enum Direction
{
  forwards,
  backwards,
  neutral
};

Direction driveDirection = neutral;


int velocityControl(int pedalInputRed, int pedalInputBlack, int angle)
{
  long vel = 0; // base velocity
  long velLeft = 0;
  long velRight = 0;
  float absAngleLeft = 0;
  float absAngleRight = 0;
  float temp;
  long pedalInputRedPar = analogRead(i_pinPedalRed);
  vel = map(pedalInputRed, fullThrottle, noThrottle, 255, 0);
  int long velPercentage = map(pedalInputRed, fullThrottle, noThrottle, 100, 0);
   /*// int valBlack = map(pedalInputBlack, fullThrottleBlack, noThrottleBlack, 100, 0); */


  if (!digitalRead(i_pinDifferentialLock))
  {
     if (angle > angleMiddle)
     { // we are going left
       temp = map(angle, angleMiddle, angleLeft, 100, 0);
       absAngleLeft = (1 - (temp / 100)) * maxSteeringOutput;
       absAngleRight = 1;
     }
     else if (angle < angleMiddle)
     { // we are going right
       temp = map(angle, angleRight, angleMiddle, 100, 0);
       absAngleRight = (temp / 100) * maxSteeringOutput;
       absAngleLeft = 1;
     }
     velLeft = int(vel * (1 - (absAngleLeft / 100)));
     velRight = int(vel * (1 - (absAngleRight / 100)));

    if(velPercentage > 100){
      velPercentage = 100;
    }
    if(velPercentage < 0){
      velPercentage = 0;
    } 
    if (velRight > 255)
    {
      velRight = 255;
    }
    if (velRight < 0)
    {
      velRight = 0;
    }
    if (velLeft > 255)
    {
      velLeft = 255;
    }
    if (velLeft < 0)
    {
      velLeft = 0;
    }
    analogWrite(o_pinThrottleRight, velRight);
    analogWrite(o_pinThrottleLeft, velLeft);
    return 0;
  }
  if (vel > 255)
  {
    vel = 255;
  }
  if (vel < 0)
  {
    vel = 0;
  }
  analogWrite(o_pinThrottleLeft, vel);
  analogWrite(o_pinThrottleRight, vel);
  return 1;
}

int sanityCheck()
{
  bool gpsFound = false;

  if (digitalRead(i_pinDrive) && digitalRead(i_pinBackwards))
  {
    //Serial.println("Setup stopped, multiple switch positions detected");
    return 1;
  }
  if (digitalRead(i_pinDrive) && digitalRead(i_pinNeutral))
  {
    //Serial.println("Setup stopped, multiple switch positions detected");
  }
  if (digitalRead(i_pinNeutral) && digitalRead(i_pinBackwards))
  {
    //Serial.println("Setup stopped, multiple switch positions detected");
    return 1;
  }

  if (analogRead(i_pinPedalRed) > (fullThrottle - 10) || analogRead(i_pinPedalRed) < (noThrottle + 20))
  {
    //Serial.println("Setup stopped, red throttle input not in defined range");
    return 2;
  }

  if (analogRead(i_pinPedalBlack) > (fullThrottleBlack + 10) || analogRead(i_pinPedalBlack) < (noThrottleBlack - 20))
  {
    //Serial.println("Setup stopped, black throttle input not in defined range");
    return 2;
  }

  if (analogRead(i_pinAngleSterringWheel) > (angleRight + 20) || analogRead(i_pinAngleSterringWheel) < (angleLeft - 20))
  {
    //Serial.println("Setup stopped, angle input not in defined range");
    return 3;
  }
  if (digitalRead(i_pinFastDrive) && digitalRead(i_pinSlowDrive))
  {
    //Serial.println("Setup stopped, drive mode switch in multiple positions or not working");
    return 4;
  }

  if (velocity > 3)
  {
    String meldung = "Setup stopped, velocity is " + String(velocity) + " m/s";
    //Serial.println(meldung);
    return 5;
  }
  return 0;
}

int initPins()
{
  pinMode(i_pinPedalRed, INPUT);
  pinMode(i_pinPedalBlack, INPUT);
  pinMode(i_pinAngleSterringWheel, INPUT);
  pinMode(i_pinBackwards, INPUT);
  pinMode(i_pinNeutral, INPUT);
  pinMode(i_pinDrive, INPUT);
  pinMode(i_pinBrakeSignal, INPUT);
  pinMode(i_pinDifferentialLock, INPUT);
  pinMode(i_pinSlowDrive, INPUT);
  pinMode(i_pinFastDrive, INPUT);
  pinMode(i_pinVelocity, INPUT);
  pinMode(o_pinBrakeanRight, OUTPUT);
  pinMode(o_pinBrakeanLeft, OUTPUT);
  pinMode(o_pinFootSwitchRight, OUTPUT);
  pinMode(o_pinFootSwitchLeft, OUTPUT);
  pinMode(o_pinThrottleLeft, OUTPUT);
  pinMode(o_pinThrottleRight, OUTPUT);
  return 0;
}

void startUp()
{

  initPins();
  //  int sanity = sanityCheck();
  int sanity = 0;
  if (sanity < 1)
  {
    //Serial.println("Setup succesful!");
  }
  else
  {
    Serial.print("Setup aborted with error code ");
    
    /*while (1)
    {
      delay(1000);
    }*/
  }
}

void driveLoop()
{
  if ((analogRead(i_pinDrive) > 1000) && driveDirection == neutral)
  {

    if (velocity < 3)
    {
      driveDirection = forwards;
      enableDrive();
    }
  }
  if (digitalRead(i_pinBackwards) && driveDirection == neutral)
  {
    if (velocity < 3)
    {
      driveDirection = backwards;
      enableDrive();
    }
  }
  if (analogRead(i_pinNeutral) > 1000)
  {
    driveDirection = neutral;
    digitalWrite(o_pinFootSwitchLeft, LOW);
    digitalWrite(o_pinFootSwitchRight, LOW);
    driveEnabled = false;
  }
  if ((driveDirection == backwards || driveDirection == forwards) && driveEnabled)
  {
    if (digitalRead(i_pinBrakeSignal))
    {
      digitalWrite(o_pinBrakeanLeft, HIGH);
      digitalWrite(o_pinBrakeanRight, HIGH);
    }
    else
    {
      digitalWrite(o_pinBrakeanLeft, LOW);
      digitalWrite(o_pinBrakeanRight, LOW);
    }
    velocityControl(analogRead(i_pinPedalRed), analogRead(i_pinPedalBlack), analogRead(i_pinAngleSterringWheel));
  }
}

void enableDrive()
{
  driveEnabled = true;
  digitalWrite(o_pinFootSwitchLeft, HIGH);
  digitalWrite(o_pinFootSwitchRight, HIGH);
}

/*
 *
 * Arduino setup and loop
 *
 */

void setup()
{
  Serial.begin(115200);
  startUp();
}

void loop()
{
  driveLoop();
}
