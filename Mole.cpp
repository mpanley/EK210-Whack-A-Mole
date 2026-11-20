#include "Mole.h"
#include <Arduino.h>

#define SIMULATE_HITS 0   // set to 0 once real buttons are wired

static unsigned long getCurrentTime() { return millis(); }

Mole::Mole(int servoPin, int buttonPin)
{
    this->servoPin = servoPin;
    this->buttonPin = buttonPin;
    raised = false;
    hit = false;
    raiseStart = 0;
    reactionTime = 0;
    simDelayMs = 0;   // pretend reaction time (used only when simulating)
}

void Mole::begin()
{
    pinMode(buttonPin, INPUT_PULLUP);
    servo.attach(servoPin);
    servo.write(DOWN_ANGLE); 
}

void Mole::raiseMole()
{
    servo.write(UP_ANGLE);
    raiseStart = getCurrentTime();
    reactionTime = 0;
    hit = false;
    raised = true;
    simDelayMs = random(400, 3500);   // pretend reaction time (used only when simulating)

}

void Mole::retractMole()
{
    servo.write(DOWN_ANGLE);
    hit = false;
    raised = false;

}

void Mole::registerHit()
{
    if (raised && !hit)
    {
        hit = true;
        reactionTime = getCurrentTime() - raiseStart;
    }
}

void Mole::update()//continuously call update; iterate time, 
{
    checkButton();
}

void Mole::checkButton()
{
    /*if (raised && digitalRead(buttonPin) == LOW)
    {
        registerHit();
    }*/
#if SIMULATE_HITS
    if (raised && getTimeRaised() >= simDelayMs)
    {
        registerHit();
    }
#else
    if (raised && digitalRead(buttonPin) == LOW)
    {
        registerHit();
    }
#endif
}

bool Mole::wasHit() const {return hit;}

bool Mole::isRaised() const {return raised;}

bool Mole::timedOut(unsigned long limitMs) const
{
    return raised && !hit && getTimeRaised() > limitMs;
}

bool Mole:: buttonPressed() const
{
    return digitalRead(buttonPin) == LOW;
}

unsigned long Mole::getTimeRaised() const
{
    return raised ? (getCurrentTime()-raiseStart) : 0;
}

unsigned long Mole::getReactionTime() const {return reactionTime;}