#include "fruit.hpp"

void Fruit::DrawFruit() //draws generic fruit
{
    if (!active) return;
    DrawCircle(x, y, 20, ORANGE);
}

void Fruit::OnPickup(int &score, int &speedBoost, int &lives) //normal fruit gives points when caught
{
    score += points;
}

void Fruit::OnMiss(int &score, int &lives) //normal fruit takes away a life when missed
{
    lives -= 1;
}

void Fruit::UpdatePosition() //falls at a constant speed, but is modified by speed boost
{
    if (active)
    {
        y += speed;
    }
}

void SpecialFruit::DrawFruit() //draws special fruit with a different color
{
    if (!active) return;
    DrawCircle(x, y, 20, GOLD);
}

void SpecialFruit::OnPickup(int &score, int &speedBoost, int &lives)
{ //special fruit gives more points, increases player lives, and increases the speed of all fruits to make the game more challenging
    score += points;
    lives += 1;
    speedBoost += 1;
}

void BadFruit::DrawFruit() //draws bad fruit with a different color
{
    if (!active) return;
    DrawCircle(x, y, 20, PURPLE);
}

void BadFruit::OnPickup(int &score, int &speedBoost, int &lives)
{//bad fruit takes away points and lives if caught
    score -= points;
    lives -= 1;
}

void BadFruit::OnMiss(int &score, int &lives)
{//bad fruit gives points if missed, to encourage the player to avoid catching it
    score += points;
}

int GetRandomGridX(int screenWidth, int gridCount)
{//divides the screen into a grid and returns a random x position centered in one of the grid columns for fruit spawning
    int gridWidth = screenWidth / gridCount;
    int gridIndex = GetRandomValue(0, gridCount - 1);
    return gridIndex * gridWidth + gridWidth / 2;
}

void SpawnFruit(Fruit &fruit, int screenWidth, int speed)
{//spawns a fruit at a random x position at the top of the screen and sets it to active with the appropriate speed
    fruit.x = GetRandomGridX(screenWidth, 8);
    fruit.y = -20;
    fruit.active = true;
    fruit.speed = speed;
}
