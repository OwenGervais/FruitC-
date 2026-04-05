#pragma once

#include <raylib.h>

class Player
{
public:
    int x; // keep track of left and right movement
    int y; // just the player's y starting position
    int xSpeed; // rate of movement left and right

    //function declarations for drawing the player and updating their position based on keyboard input
    void DrawPlayer();
    void Update();
};
