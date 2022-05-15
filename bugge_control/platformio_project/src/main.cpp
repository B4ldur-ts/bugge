#include <Arduino.h>
// Pin defines
#define i_pinPedalRed 19
#define i_pinPedalBlack 20
#define o_pinThrottleRight 9
#define o_pinThrottleLeft 8
#define i_pinAngleSterringWheel 21
#define i_pinBackwards 24
#define i_pinNeutral 25
#define i_pinDrive 26
#define i_pinBrakeSignal 15
#define o_pinBrakeanRight 14
#define o_pinBrakeanLeft 13
#define i_pinDifferentialLock 16
#define i_pinSlowDrive 10 // TODO:
#define i_pinFastDrive 9  // TODO:
#define o_pinFootSwitchRight 7
#define o_pinFootSwitchLeft 6
#define i_pinVelocity 22

// Settings
#define fullThrottle 330
#define noThrottle 470

#define noThrottleBlack 335
#define fullThrottleBlack 404

// function prototypes
int velocityControl(int, int, int, int);

// glonal vars
bool driveEnabled = false;

void setup()
{
  pinMode(i_pinPedalRed, INPUT);
  pinMode(i_pinPedalBlack, INPUT);
  pinMode(o_pinThrottleLeft, OUTPUT);
  pinMode(o_pinThrottleRight, OUTPUT);
  Serial.begin(115200);
  Serial.println("Setup succesful!");
  Serial.println(" ");
}

void loop()
{
  if (digitalRead(i_pinDrive) || digitalRead(i_pinBackwards))
  {
    digitalWrite(o_pinFootSwitchLeft, HIGH);
    digitalWrite(o_pinFootSwitchRight, HIGH);
    driveEnabled = true;

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
    velocityControl(analogRead(i_pinPedalRed), analogRead(i_pinPedalBlack), analogRead(i_pinAngleSterringWheel), analogRead(i_pinVelocity));
  }
  else
  {
    digitalWrite(o_pinFootSwitchLeft, LOW);
    digitalWrite(o_pinFootSwitchRight, LOW);
    driveEnabled = false;
  }
}

int velocityControl(int pedalInputRed, int pedalInputBlack, int angle, int velocity)
{
  int val;
  int valPercentage = map(pedalInputRed, fullThrottle, noThrottle, 100, 0);
  int valBlack = map(pedalInputBlack, fullThrottleBlack, noThrottleBlack, 100, 0);

  if (!digitalRead(i_pinDifferentialLock))
  {
    // do some differential stuff
    return 0;
  }
  if (digitalRead(i_pinFastDrive))
  {
    val = map(pedalInputRed, fullThrottle, noThrottle, 255, 0); // Geschw Begrenzung (60 war ganz okay)
    val = int(val * exp(valPercentage));
  }
  else
  {
    if (valPercentage > 30)
    {
      val = map(pedalInputRed, fullThrottle, noThrottle, 125, 0);
    }
    else
    {
      val = map(pedalInputRed, fullThrottle, noThrottle, 50, 0);
    }
  }

  if (val > 255)
  {
    val = 255;
  }
  if (val < 0)
  {
    val = 0;
  }
  analogWrite(o_pinThrottleLeft, val);
  analogWrite(o_pinThrottleRight, val);
  return 1;
}
