#include <Arduino.h>
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

#define angleMiddle 127      // Input from the analog pin, when steering wheel is centered
#define angleLeft 1          // Input, when steering wheel is all the way to the left
#define angleRight 255       // Input, when steering wheel is all the way to the right
#define maxSteeringOutput 30 // max. position of the wheels at maximum steering input in degree

// function prototypes
int velocityControl(int, int, int, int);
int sanityCheck();
int initPins();
void startUp();
void driveLoop();
void enableDrive();

// global vars
bool driveEnabled = false;
bool throttleEnabled = false;
enum Direction
{
    forwards,
    backwards,
    neutral
};

Direction driveDirection = neutral;

int velocityControl(int pedalInputRed, int pedalInputBlack, int angle, int velocity)
{
    float vel = 0; // base velocity
    float temp = 0;
    float velLeft = 0;
    float velRight = 0;
    float absAngleLeft = 0;
    float absAngleRight = 0;

    float velPercentage = map(pedalInputRed, fullThrottle, noThrottle, 100, 0);
    // int valBlack = map(pedalInputBlack, fullThrottleBlack, noThrottleBlack, 100, 0);

    if (digitalRead(i_pinFastDrive))
    {
        vel = map(pedalInputRed, fullThrottle, noThrottle, 255, 0); // Geschw Begrenzung (60 war ganz okay)
        vel = int(vel * exp(velPercentage));
    }
    else
    {
        if (velPercentage > 30)
        {
            vel = map(pedalInputRed, fullThrottle, noThrottle, 125, 0);
        }
        else
        {
            vel = map(pedalInputRed, fullThrottle, noThrottle, 50, 0);
        }
    }

    if (!digitalRead(i_pinDifferentialLock))
    {
        /* if (angle < angleMiddle)
         { // we are going left
           temp = map(angle, angleMiddle, angleLeft, 100, 0);
           absAngleLeft = (1 - (temp / 100)) * maxSteeringOutput;
           absAngleRight = 1;
         }
         else if (angle > angleMiddle)
         { // we are going right
           temp = map(angle, angleRight, angleMiddle, 100, 0);
           absAngleRight = (temp / 100) * maxSteeringOutput;
           absAngleLeft = 1;
         }
         velLeft = int(vel * (1 - (absAngleLeft / 100)));
         velRight = int(vel * (1 - (absAngleRight / 100))); */

        Serial.println(velLeft);
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
        // Only use when angle indicator is not working
        velLeft = (255 * (velPercentage / 100));
        velRight = (255 * (velPercentage / 100));
        // analogWrite(o_pinThrottleRight, int (velRight));
        analogWrite(o_pinThrottleLeft, int(velLeft));
        return 0;
    }
    vel = (255 * (velPercentage / 100));
    if (vel > 255)
    {
        vel = 255;
    }
    if (vel < 0)
    {
        vel = 0;
    }
    Serial.println(velLeft);
    Serial.println("Vel set");
    analogWrite(o_pinThrottleLeft, vel);
    // analogWrite(o_pinThrottleRight, vel);
    return 1;
}

int sanityCheck()
{
    if (digitalRead(i_pinDrive) && digitalRead(i_pinBackwards))
    {
        Serial.println("Setup stopped, multiple switch positions detected");
        return 1;
    }
    if (digitalRead(i_pinDrive) && digitalRead(i_pinNeutral))
    {
        Serial.println("Setup stopped, multiple switch positions detected");
    }
    if (digitalRead(i_pinNeutral) && digitalRead(i_pinBackwards))
    {
        Serial.println("Setup stopped, multiple switch positions detected");
        return 1;
    }

    if (analogRead(i_pinPedalRed) > (fullThrottle - 10) || analogRead(i_pinPedalRed) < (noThrottle + 20))
    {
        Serial.println("Setup stopped, red throttle input not in defined range");
        return 2;
    }

    if (analogRead(i_pinPedalBlack) > (fullThrottleBlack + 10) || analogRead(i_pinPedalBlack) < (noThrottleBlack - 20))
    {
        Serial.println("Setup stopped, black throttle input not in defined range");
        return 2;
    }

    if (analogRead(i_pinAngleSterringWheel) > (angleRight + 20) || analogRead(i_pinAngleSterringWheel) < (angleLeft - 20))
    {
        Serial.println("Setup stopped, angle input not in defined range");
        return 3;
    }
    if (digitalRead(i_pinFastDrive) && digitalRead(i_pinSlowDrive))
    {
        Serial.println("Setup stopped, drive mode switch in multiple positions or not working");
        return 4;
    }

    // TODO: add velocity check

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
    // int sanity = sanityCheck();
    int sanity = 0;
    if (sanity < 1)
    {
        Serial.println("Setup succesful!");
        Serial.println(" ");
    }
    else
    {
        Serial.print("Setup aborted with error code ");
        Serial.println(sanity);
        while (1)
        {
            delay(1000);
        }
    }
}

void driveLoop()
{
    if (digitalRead(i_pinDrive) && driveDirection == neutral)
    {
        Serial.println("Forward");
        driveDirection = forwards;
        enableDrive();
    }
    if (digitalRead(i_pinBackwards) && driveDirection == neutral)
    {
        Serial.println("Backward");
        driveDirection = backwards;
        enableDrive();
    }
    if (analogRead(i_pinNeutral) > 1000)
    {
        Serial.println("Neutral");
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
        velocityControl(analogRead(i_pinPedalRed), analogRead(i_pinPedalBlack), analogRead(i_pinAngleSterringWheel), analogRead(i_pinVelocity));
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
