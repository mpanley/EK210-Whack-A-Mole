//Each mole object has two corresponding arduino pins, one to the servo and one to the button

#ifndef Mole_h
#define Mole_h

#include <Servo.h>

using namespace std;

class Mole {
    public:
        Mole(int servoPin, int buttonPin);
        void raiseMole();
        void retractMole();
        void update();
        void begin();
        void registerHit();
        void checkButton();
        bool wasHit() const;
        bool isRaised() const;
        bool buttonPressed() const;
        //bool isIdle() const;
        bool timedOut(unsigned long limitMS) const;
        unsigned long getTimeRaised() const;
        unsigned long getReactionTime() const;

    private:
        static const int UP_ANGLE = 90;
        static const int DOWN_ANGLE = 0;
        unsigned long simDelayMs;   // test only: when the pretend child "presses" the button

        Servo servo;
        int servoPin;
        int buttonPin;
        bool raised;
        bool hit;
        unsigned long raiseStart;
        unsigned long reactionTime;

};

#endif