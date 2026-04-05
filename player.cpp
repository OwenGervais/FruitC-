#include "player.hpp"

void Player::DrawPlayer() //draws the player's head and body as simple shapes for collision detection
{
    DrawCircle(x, y - 110, 50, RED);
    DrawRectangle(x - 30, y - 100, 60, 80, RED);
}

void Player::Update() //player left and right movement based on keyboard input, with boundaries to prevent moving off screen
{
    if (IsKeyDown(KEY_LEFT) && x - 30 > 0)
    {
        x -= xSpeed;
    }
    if (IsKeyDown(KEY_RIGHT) && x + 30 < 800)
    {
        x += xSpeed;
    }
}
