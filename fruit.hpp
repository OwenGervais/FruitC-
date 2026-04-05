#pragma once

#include <raylib.h>

class Fruit
{
public:
    int x; // spawn x position
    int y; // tracks falling position
    int speed; // fall speed
    int points; // points awarded when caught
    bool active; // disables fruit when caught or missed

    virtual ~Fruit() = default;
    virtual void DrawFruit();
    virtual void OnPickup(int &score, int &speedBoost, int &lives); 
    virtual void OnMiss(int &score, int &lives);
    void UpdatePosition();
};

class SpecialFruit : public Fruit
{ //special fruits give more points, increase player lives, and increase the speed of all fruits to make the game more challenging
public:
    void DrawFruit() override;
    void OnPickup(int &score, int &speedBoost, int &lives) override;
};

class BadFruit : public Fruit
{ //bad fruits take away points and lives if caught, but give points if missed
public:
    void DrawFruit() override;
    void OnPickup(int &score, int &speedBoost, int &lives) override;
    void OnMiss(int &score, int &lives) override;
};

int GetRandomGridX(int screenWidth, int gridCount);
void SpawnFruit(Fruit &fruit, int screenWidth, int speed);
