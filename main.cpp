#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <raylib.h>
#include "leader.hpp"
#include "fruit.hpp"
#include "player.hpp"

using namespace std;

void UpdateFruitList(Fruit arr[], int count, int screenHeight, Player &player, int &score, int &speedBoost, int &lives)
{ //handles updating position of fruits, checking for collisions with the player, and applying the appropriate score/lives changes based on the type of fruit and whether it was caught or missed
    Vector2 headPos = {(float)player.x, (float)player.y - 110}; //player collision positions
    Rectangle bodyRect = {(float)player.x - 30, (float)player.y - 100, 60, 80};

    for (int i = 0; i < count; i++)
    {
        if (!arr[i].active) //skip inactive fruits
            continue;

        arr[i].UpdatePosition();
        Vector2 fruitPos = {(float)arr[i].x, (float)arr[i].y};

        if (CheckCollisionCircles(headPos, 50, fruitPos, 20) || CheckCollisionCircleRec(fruitPos, 20, bodyRect))
        { //if the fruit collides with either the player's head or body, it's considered caught
            arr[i].active = false;
            arr[i].OnPickup(score, speedBoost, lives);
        }
        else if (arr[i].y > screenHeight)
        { //if the fruit falls past the bottom of the screen, it's considered missed
            arr[i].active = false;
            arr[i].OnMiss(score, lives);
        }
    }
}

int main () 
{
    const int fruitCount = 10; //fruit array size
    const int specialFruitCount = 1; //only 1 special fruit at a time can change later maybe
    const int badFruitCount = 1; //only 1 bad fruit at a time
    const int baseFruitSpeed = 5; //base fall speed
    const int basePlayerSpeed = 8; //base player speed

    const int screenWidth = 800; //screen dimensions
    const int screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "Fruit Catch Game"); //window title
    SetTargetFPS(60); //game fps
    
    Player player;
    Fruit fruits[fruitCount];
    SpecialFruit specialFruits[specialFruitCount]; //only 1 special fruit, made an array for easier update with normal fruits, and possiible future changes
    BadFruit badFruits[badFruitCount]; //only 1 bad fruit

    player.x = screenWidth / 2; //define player variables
    player.y = screenHeight;

    int score = 0; //player score
    int lives = 3; //player lives, can be used to end game after missing too many fruits
    int globalSpawnTimer = 0; //global timer to control fruit spawn timing
    int spawnDelay = 60; //delay between fruit spawns in frames (60 frames = 1 second at 60 FPS)
    int globalSpeedBoost = 0; //tracks the cumulative speed boost from special fruits
    const string leaderboardFile = "leaderboard.txt"; //file to store leaderboard data
    enum class GameState { Title, Playing, GameOver }; //game state enum to control flow of the game
    vector<LeaderboardEntry> leaderboard; //vector to hold leaderboard entries loaded from file
    GameState gameState = GameState::Title; //start at title screen
    bool scoreSaved = false; //flag to track if the player's score has been saved to the leaderboard after game over after a restart
    string currentName; //string to hold the player's name input after game over
    LoadLeaderboard(leaderboardFile, leaderboard); //load leaderboard data
    
    // define normal fruits
    for (int i = 0; i < fruitCount; i++)
    {
        fruits[i].points = 1; //default points for a normal fruit, can be adjusted per class
        fruits[i].active = false;
        fruits[i].x = 0;
        fruits[i].y = -100; //start off-screen
    }

    //define special fruit
    for (int i = 0; i < specialFruitCount; i++)
    {
        specialFruits[i].points = 3; //more points
        specialFruits[i].active = false;
        specialFruits[i].x = 0;
        specialFruits[i].y = -100; //start off-screen
    }

    //define bad fruit
    for (int i = 0; i < badFruitCount; i++)
    {
        badFruits[i].points = 3; //converts to negative points in OnPickup
        badFruits[i].active = false;
        badFruits[i].x = 0;
        badFruits[i].y = -100; //start off-screen
    }

    while(!WindowShouldClose()) //game loop
    {
        if (gameState == GameState::Title)
        {
            BeginDrawing();
            ClearBackground(BLACK);
            DrawText("Fruit Catch Game", screenWidth / 2 - 220, screenHeight / 2 - 80, 50, YELLOW);
            DrawText("Press E to start", screenWidth / 2 - 150, screenHeight / 2 + 20, 30, WHITE);
            EndDrawing();

            if (IsKeyPressed(KEY_E)) //press E to start the game
            {
                gameState = GameState::Playing;
            }
            continue; //skip the rest of the loop until the game starts
        }

        if (gameState == GameState::Playing && lives <= 0) //if player runs out of lives, end the game
        {
            gameState = GameState::GameOver;
        }

        if (gameState == GameState::GameOver) //game over screen for displaying final score, entering name for leaderboard, and showing top scores with options to restart or exit
        {
            if (!scoreSaved)
            {
                int key = GetCharPressed();
                while (key > 0)
                {   
                    if ((key >= 32) && (key <= 125) && (currentName.size() < 3)) //limit name input to 3 characters and filter for valid characters
                    {
                        char ch = static_cast<char>(key);
                        if (isalnum(static_cast<unsigned char>(ch)))
                        {
                            if (isalpha(static_cast<unsigned char>(ch)))
                                ch = static_cast<char>(toupper(static_cast<unsigned char>(ch)));
                            currentName.push_back(ch);
                        }
                    }
                    key = GetCharPressed();
                }

                if (IsKeyPressed(KEY_BACKSPACE) && !currentName.empty()) //allow backspace to delete characters in name input
                {
                    currentName.pop_back();
                }
            }

            BeginDrawing();
            ClearBackground(BLACK);
            int gameOverFontSize = 80;
            int gameOverTextWidth = MeasureText("Game Over!", gameOverFontSize);
            DrawText("Game Over!", screenWidth / 2 - gameOverTextWidth / 2, screenHeight / 2 - 80, gameOverFontSize, RED);
            DrawText(TextFormat("Final Score: %d", score), screenWidth / 2 - 120, screenHeight / 2 + 10, 30, WHITE);

            if (!scoreSaved) //if the score hasn't been saved yet, show name input prompt and save score to leaderboard when ready, otherwise show leaderboard and options to restart or exit
            {
                string displayName = currentName;
                while (displayName.size() < 3)
                    displayName.push_back('_');

                DrawText("Enter your name:", screenWidth / 2 - 160, screenHeight / 2 + 40, 24, WHITE);
                DrawText(displayName.c_str(), screenWidth / 2 - 50, screenHeight / 2 + 80, 40, YELLOW);
                DrawText("Press ENTER when ready", screenWidth / 2 - 150, screenHeight / 2 + 140, 20, LIGHTGRAY);

                if (currentName.size() == 3 && IsKeyPressed(KEY_ENTER)) //only save the score if the player has entered a 3 character name and presses enter
                {
                    AddScoreToLeaderboard(leaderboardFile, currentName, score);
                    LoadLeaderboard(leaderboardFile, leaderboard);
                    scoreSaved = true;
                }
            }
            else
            {
                DrawText("Top Scores:", screenWidth / 2 - 120, screenHeight / 2 + 60, 26, GOLD);

                for (int i = 0; i < (int)leaderboard.size() && i < 5; i++) //display top 5 scores from the leaderboard
                {
                    DrawText(TextFormat("%d. %s - %d", i + 1, leaderboard[i].name.c_str(), leaderboard[i].score),
                    screenWidth / 2 - 140, screenHeight / 2 + 100 + i * 28, 20, RAYWHITE);
                }

                DrawText("Press R to restart", screenWidth / 2 - 180, screenHeight / 2 + 260, 22, YELLOW);
                DrawText("Press E to exit", screenWidth / 2 - 180, screenHeight / 2 + 300, 22, YELLOW);
                if (IsKeyPressed(KEY_R)) //resets game variables to start a new game
                {
                    score = 0;
                    lives = 3;
                    globalSpawnTimer = 0;
                    globalSpeedBoost = 0;
                    gameState = GameState::Playing;
                    scoreSaved = false;
                    currentName.clear();
                    for (int i = 0; i < fruitCount; i++)
                    {
                        fruits[i].active = false;
                        fruits[i].y = -100;
                    }
                    for (int i = 0; i < specialFruitCount; i++)
                    {
                        specialFruits[i].active = false;
                        specialFruits[i].y = -100;
                    }
                    for (int i = 0; i < badFruitCount; i++)
                    {
                        badFruits[i].active = false;
                        badFruits[i].y = -100;
                    }
                    player.x = screenWidth / 2;
                }
                else if (IsKeyPressed(KEY_E)) //exit the game
                {
                    break;
                }
            }

            EndDrawing();
            continue;
        }

        BeginDrawing();

        //global speed updates
        int currentFruitSpeed = baseFruitSpeed + globalSpeedBoost;
        player.xSpeed = basePlayerSpeed + globalSpeedBoost * 2;;
        spawnDelay = max(10, 60 - globalSpeedBoost * 10); //decrease spawn delay as speed increases, with a minimum cap

        //update active fruit speeds
        for (int i = 0; i < fruitCount; i++)
        {
            if (fruits[i].active)
                fruits[i].speed = currentFruitSpeed;
        }
        for (int i = 0; i < specialFruitCount; i++)
        {
            if (specialFruits[i].active)
            {
                specialFruits[i].speed = currentFruitSpeed;
            }
        }
        for (int i = 0; i < badFruitCount; i++)
        {
            if (badFruits[i].active)
            {
                badFruits[i].speed = currentFruitSpeed;
            }
        }

        //updating
        player.Update();

        if (globalSpawnTimer > 0) //spawn timer countdown
        {
            globalSpawnTimer--;
        }
        else
        {
            if (score > 0 && score % 20 == 0 && !specialFruits[0].active) //spawn special fruit every 20 points if not already active
            {
                SpawnFruit(specialFruits[0], screenWidth, currentFruitSpeed);
                globalSpawnTimer = spawnDelay;
            }
            else if (score > 0 && score % 15 == 0 && !badFruits[0].active) //spawn bad fruit every 15 points if not spawning a special fruit and not already active
            {
                SpawnFruit(badFruits[0], screenWidth, currentFruitSpeed);
                globalSpawnTimer = spawnDelay;
            }
            else
            {
                for (int i = 0; i < fruitCount; i++) //finds an inactive fruit to spawn a new one
                {
                    if (!fruits[i].active)
                    {
                        SpawnFruit(fruits[i], screenWidth, currentFruitSpeed);
                        globalSpawnTimer = spawnDelay;
                        break;
                    }
                }
            }
        }

        UpdateFruitList(fruits, fruitCount, screenHeight, player, score, globalSpeedBoost, lives);
        UpdateFruitList(specialFruits, specialFruitCount, screenHeight, player, score, globalSpeedBoost, lives);
        UpdateFruitList(badFruits, badFruitCount, screenHeight, player, score, globalSpeedBoost, lives);

        //drawing
        ClearBackground(SKYBLUE); //sky/background color
        DrawRectangle(0, screenHeight - 100, screenWidth, 100, GREEN); //ground color

        for (int i = 0; i < fruitCount; i++)
        {
            fruits[i].DrawFruit();
        }
        for (int i = 0; i < specialFruitCount; i++)
        {
            specialFruits[i].DrawFruit();
        }
        for (int i = 0; i < badFruitCount; i++)
        {
            badFruits[i].DrawFruit();
        }
        player.DrawPlayer();

        //score
        DrawText(TextFormat("Score: %d", score), 20, 20, 40, BLACK);
        //lives
        DrawText(TextFormat("Lives: %d", lives), 20, 70, 40, BLACK);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}