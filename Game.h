#ifndef Game_h
#define Game_h

#include "Mole.h"
#include <Arduino.h>
#include <Servo.h>

class Game {
    public:
        Game(int activeMoles = 5);
        void addTime(unsigned long ms);
        double getAverageTime() const;
        double getHitRate() const;
        //void turnOff();
        void raiseMole();
        void retractMole();
        void update();
        bool isIdle();
        bool canAdapt();
        void begin();
        //void reset();
        void restart();

    private:
        static const int WINDOW = 5;
        static const unsigned long START_WINDOW_MS = 3000;
        static const int IDLE_MISSES = 5;
        static const unsigned long MIN_WINDOW_MS = 1000;
        static const unsigned long MAX_WINDOW_MS = 5000;
        static const int ADAPT_EVERY = 5;
        static const int LEVELS_PER_EXTRA_MOLE = 2;
        int level;
        int maxMolesNow() const;
        bool anyMoleRaised() const;
        Mole * moles[5];//indices correspond to arduino pins to servos
        int buzzerPin;
        int activeMoles;               // how many moles are connected (1 to 5)
        //bool switchState;
        double times[WINDOW];//rolling array keep track of the time taken on last 5 hits, initialized to -1
        int nextIndex;
        int count;
        unsigned long windowMs;
        unsigned long nextRaiseAt;
        bool outcomes[WINDOW];
        int outcomeIndex;
        int outcomeCount;
        int consecutiveMisses;
        int roundsSinceAdapt;
        
        void recordOutcome(bool wasHit);
        void adaptWindow();
        void playCue(bool faster);
        void resetState();
};


#endif 