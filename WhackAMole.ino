#include "Game.h"

Game game(5);   // 4 moles connected for simulation; change parameter 0-5 for however many moles desired

void setup()
{
    game.begin();
}

void loop()
{
    game.update();
}