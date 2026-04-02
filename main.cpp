#include <iostream>
#include <raylib.h>

using namespace std;

//Game Summary:
//In this game, the player controls a character that moves left and right to catch falling fruits
//The player earns points for each fruit caught, and the game ends when a fruit is missed
//Every 30 seconds, a special type of fruit will fall that increases the speed of the falling fruits, but also the player, making the game more challenging
//occasionally, a bad fruit will fall, which the player must avoid catching, as it will decrease their score

class Player
{
    public:
        int x; //keep tracks of left and right movement
        int y; //just the players y starting position
        int xSpeed; //rate of movement left and right

        void DrawPlayer()
        {
            DrawCircle(x, y - 130, 50, RED); //head
            DrawRectangle(x - 30, y - 120, 60, 80, RED); //body
        }

        void Update() //movement controls update
        {
            if(IsKeyDown(KEY_LEFT) && x - 30 > 0)
            {
                x -= xSpeed;
            }
            if(IsKeyDown(KEY_RIGHT) && x + 30 < 800)
            {
                x += xSpeed;
            }
        }
};

class Fruit
{
    public:
        int x; //spawn x position
        int y; //tracks falling position
        int speed; //fall speed
        bool active; //disables fruit when caught or missed

        void DrawFruit()
        {
            if (!active) return; //don't draw if not active
            DrawCircle(x, y, 20, ORANGE); //normal fruit
        }

        void UpdatePosition() //fall speed update
        {
            if (active)
            {
                y += speed;
            }
        }
};

const int fruitCount = 5; //fruit array size
const int gridCount = 8; //divides the screen into 8 sections for fruit spawning
const int spawnDelay = 60; //delay between fruit spawns in frames (60 frames = 1 second at 60 FPS)

int GetRandomGridX(int screenWidth) //splits the screen into 8 sections and returns a random x position within one of those sections for fruit spawning
{
    int gridWidth = screenWidth / gridCount; //width of each grid section
    int gridIndex = GetRandomValue(0, gridCount - 1); //randomly selects a grid section index
    return gridIndex * gridWidth + gridWidth / 2; //returns the x position of the center of the randomly selected grid section
}

void SpawnFruit(Fruit &fruit, int screenWidth) //initializes fruit properties and spawns it at a random x position above the screen
{
    fruit.x = GetRandomGridX(screenWidth);
    fruit.y = -20; //start above the screen
    fruit.active = true;
}

Player player;
Fruit fruits[fruitCount];

int main () 
{
    const int screenWidth = 800; //screen dimensions
    const int screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "Fruit Catch Game"); //window title
    SetTargetFPS(60); //game fps

    player.x = screenWidth / 2; //define player variables
    player.y = screenHeight;
    player.xSpeed = 7;

    int score = 0;
    int globalSpawnTimer = 0;

    // define fruits
    for (int i = 0; i < fruitCount; i++)
    {
        fruits[i].speed = 5;
        fruits[i].active = false;
        fruits[i].x = 0;
        fruits[i].y = -100; //start off-screen
    }

    while(!WindowShouldClose()) //game loop
    {
        BeginDrawing();

        //updating
        player.Update();

        if (globalSpawnTimer > 0) //spawn timer countdown
        {
            globalSpawnTimer--;
        }
        else
        {
            for (int i = 0; i < fruitCount; i++) //finds an inactive fruit to spawn a new one
            {
                if (!fruits[i].active)
                {
                    SpawnFruit(fruits[i], screenWidth);
                    globalSpawnTimer = spawnDelay;
                    break;
                }
            }
        }

        for (int i = 0; i < fruitCount; i++) //update all active fruits to fall and check for collisions with the player
        {
            if (fruits[i].active)
            {
                fruits[i].UpdatePosition();
                Vector2 fruitPos = {(float)fruits[i].x, (float)fruits[i].y};
                Vector2 headPos = {(float)player.x, (float)player.y - 130};
                Rectangle bodyRect = {(float)player.x - 30, (float)player.y - 120, 60, 80};

                if (CheckCollisionCircles(headPos, 50, fruitPos, 20) || CheckCollisionCircleRec(fruitPos, 20, bodyRect))
                {
                    score++;
                    fruits[i].active = false;
                }
                else if (fruits[i].y > screenHeight)
                {
                    fruits[i].active = false;
                    //lose game or lose life logic could go here
                }
            }
        }

        ClearBackground(SKYBLUE); //sky/background color
        DrawRectangle(0, screenHeight - 100, screenWidth, 100, GREEN); //ground color

        for (int i = 0; i < fruitCount; i++)
        {
            fruits[i].DrawFruit();
        }
        player.DrawPlayer();

        //score
        DrawText(TextFormat("Score: %d", score), 20, 20, 40, BLACK);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}