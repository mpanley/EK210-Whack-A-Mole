#include "Game.h"
static void startJingle(int pin);
static void endGameJingle(int pin);
static void speedUpJingle(int pin);
static void slowDownJingle(int pin);

Game::Game(int activeMoles)
{
    this->activeMoles = activeMoles;
    moles[0] = new Mole(2,7); //Initialize Mole objects; parameters are servo pins and button pins
    moles[1] = new Mole(3,8);
    moles[2] = new Mole(4,9);
    moles[3] = new Mole(5,10);
    moles[4] = new Mole(6,11);//change pin numbers later
    
    buzzerPin = 12; //Arduino pin corresponsing to buzzer

    resetState();
}

void Game::resetState()
{
    //switchState = true; //switch is on

    nextIndex = 0;
    count = 0;

    windowMs = START_WINDOW_MS;
    outcomeIndex = 0;
    outcomeCount = 0;
    consecutiveMisses = 0;
    roundsSinceAdapt = 0;
    nextRaiseAt = 0;
    level = 0;
}

void Game::begin()
{
    Serial.begin(9600);
    randomSeed(analogRead(A0));//for unconnected analog pin 0, reads noise
    for (int i=0; i < 5; i++)
    {
        moles[i]->begin();
        delay(300);
    }
    startJingle(buzzerPin);
    nextRaiseAt = millis()+1000;
}

void Game:: restart()//when idle, instead of turning off, just reset to beginning
{
    Serial.println("Idle: restarting game");
    for (int i = 0; i < 5; i++)
    {
        moles[i]->retractMole(); 
    }
    resetState();
    endGameJingle(buzzerPin);
    startJingle(buzzerPin);//can change; didn't know which we wanted
    nextRaiseAt = millis() + 1000;
}

double Game::getAverageTime() const
{
    //iterate thorugh time array, return average time
    if (count == 0) return 0;

    unsigned long sum = 0;
    for (int i = 0; i < count; i++)
    {
        sum+=times[i];
    }
    return (double)sum/count;
}

double Game::getHitRate() const
{
    if (outcomeCount==0)return 0;

    int hits = 0;
    for (int i=0;i<outcomeCount;i++)
    {
        if (outcomes[i])hits++;
    }
    return double(hits)/outcomeCount;
}

bool Game::canAdapt()//if we want to adapt the current time window
{
    return outcomeCount >= WINDOW;
}

void Game::raiseMole()
{
    
    if (millis() < nextRaiseAt) return;  // still in a pause
    if (anyMoleRaised()) return;      // wait until all moles are down

    int max = maxMolesNow();
    int count = random(1, max + 1);      // upper bound is exclusive

    int order[5];                              // shuffle so each mole is picked at most once
    for (int i = 0; i < activeMoles; i++) order[i] = i;

    for (int i = 0; i < count; i++)
    {
        int j = random(i, activeMoles);        // pick from the moles not used yet
        int tmp = order[i];
        order[i] = order[j];
        order[j] = tmp;
        moles[order[i]]->raiseMole();
    }

    Serial.print("Raising ");
    Serial.print(count);
    Serial.println(" mole(s)");
    //if not raised, raise random mole
    //servo rotates up, start timer -- call mole raise methos
}

void Game::retractMole()
{
    for (Mole * m : moles)
    {
        if (!m->isRaised()) continue;
        if (m->wasHit())
        {
            addTime(m->getReactionTime());
            recordOutcome(true);
            m->retractMole();
        }
        else if (m->timedOut(windowMs))
        {
            recordOutcome(false);
            m->retractMole();
        }
    }
    //if button hit, servo rotates back
    //add time to time array, call avergge time
    //call mole retract method
}
void Game::addTime(unsigned long ms)
{
    times[nextIndex] = ms;
    nextIndex = (nextIndex + 1) % WINDOW;   // wraps 4 -> 0, overwriting the oldest
    if (count < WINDOW) count++;
}

int Game::maxMolesNow() const
{
    int n = 1 + (level > 0 ? level : 0) / LEVELS_PER_EXTRA_MOLE;
    if (n > activeMoles) n = activeMoles;     // never more than are connected
    return n;
}

bool Game::anyMoleRaised() const
{
    for (int i = 0; i < 5; i++)
    {
        if (moles[i]->isRaised()) return true;
    }
    return false;
}

void Game::recordOutcome(bool wasHit)
{
    outcomes[outcomeIndex] = wasHit;
    outcomeIndex = (outcomeIndex + 1) % WINDOW;
    if (outcomeCount < WINDOW) outcomeCount++;

    if (wasHit) consecutiveMisses = 0;
    else consecutiveMisses++;

    roundsSinceAdapt++;
    Serial.print(wasHit ? "HIT  " : "MISS ");
    Serial.print("window=");
    Serial.print(windowMs);
    Serial.print("ms  hitRate=");
    Serial.print(getHitRate());
    Serial.print("  avgTime=");
    Serial.println(getAverageTime());
} 

void Game::adaptWindow()
{
    if (!canAdapt()) return;
    if (roundsSinceAdapt < ADAPT_EVERY) return;

    roundsSinceAdapt = 0;
    double rate = getHitRate();
    if (rate > 0.70)
    {
        unsigned long newWindow = (unsigned long)(windowMs * 0.9);
        if (newWindow < MIN_WINDOW_MS) newWindow = MIN_WINDOW_MS;
        if (newWindow != windowMs)
        {
            windowMs = newWindow;
            level++;
            Serial.print("Window shortened to ");
            Serial.print(windowMs);
            Serial.print("  level ");
            Serial.println(level);
            speedUpJingle(buzzerPin);
        }
    }
    else if (rate < 0.40)                          // struggling: lengthen the window
    {
        unsigned long newWindow = (unsigned long)(windowMs * 1.1);
        if (newWindow > MAX_WINDOW_MS) newWindow = MAX_WINDOW_MS;
        if (newWindow != windowMs)
        {
            windowMs = newWindow;
            level--;
            Serial.print("Window lengthened to ");
            Serial.print(windowMs);
            Serial.print("  level ");
            Serial.println(level);
            slowDownJingle(buzzerPin);
        }
    }

}

bool Game::isIdle()
{
    return consecutiveMisses >= IDLE_MISSES;
}

void Game::update()
{
    for (Mole * m : moles)
    {
        m->update();
    }
    retractMole();

    if (isIdle())
    {
        restart();
        return;
    }

    if (!anyMoleRaised()) adaptWindow();
    raiseMole();
}

static void startJingle(int pin)// C E G high-C: rising
{
    tone(pin, 523, 120);
    delay(140);
    tone(pin, 659, 120);
    delay(140);
    tone(pin, 784, 120);
    delay(140);
    tone(pin, 1047, 250);
    delay(270);
}

static void speedUpJingle(int pin)
{
    tone(pin, 659, 80);
    delay(100);
    tone(pin, 880, 80);
    delay(100);
    tone(pin, 1175, 160);
    delay(180);
}

static void slowDownJingle(int pin)
{
    tone(pin, 880, 100);
    delay(120);
    tone(pin, 659, 100);
    delay(120);
    tone(pin, 440, 200);
    delay(220);
}

static void endGameJingle(int pin)// G E C low-G: winding down
{
    tone(pin, 784, 150);
    delay(170);
    tone(pin, 659, 150);
    delay(170);
    tone(pin, 523, 150);
    delay(170);
    tone(pin, 392, 350);
    delay(370);
}

