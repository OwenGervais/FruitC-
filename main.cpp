#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cctype>
#include <raylib.h>

using namespace std;

//Game Summary:
//In this game, the player controls a character that moves left and right to catch falling fruits
//The player earns points for each fruit caught, and the game ends when a fruit is missed
//Every couple points, a special type of fruit will fall that increases the speed of the falling fruits, but also the player, making the game more challenging
//occasionally, a bad fruit will fall, which the player must avoid catching, as it will decrease their score

class Player
{
    public:
        int x; //keep tracks of left and right movement
        int y; //just the players y starting position
        int xSpeed; //rate of movement left and right

        void DrawPlayer()
        {
            DrawCircle(x, y - 110, 50, RED); //head
            DrawRectangle(x - 30, y - 100, 60, 80, RED); //body
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
        int points; //points awarded when caught
        bool active; //disables fruit when caught or missed

        virtual void DrawFruit()
        {
            if (!active) return; //don't draw if not active
            DrawCircle(x, y, 20, ORANGE); //normal fruit
        }

        virtual void OnPickup(int &score, int &speedBoost, int &lives) //special effect when player catches the fruit
        {
            score += points;
        }

        virtual void OnMiss(int &score, int &lives) //special effect when player misses the fruit
        {
            lives -= 1; //decrease lives on miss, can be used to end game after too many misses
        }

        void UpdatePosition() //fall speed update
        {
            if (active)
            {
                y += speed;
            }
        }
};

class SpecialFruit : public Fruit
{
    public:
        void DrawFruit() //overrides normal fruit drawing to be a different color
        {
            if (!active) return;
            DrawCircle(x, y, 20, GOLD);
        }

        void OnPickup(int &score, int &speedBoost, int &lives) override
        {
            score += points;
            lives += 1; //reward extra life for catching special fruit
            speedBoost += 1; //increase global speed by 1
        }
};

class BadFruit : public Fruit
{
    public:
        void DrawFruit() //overrides normal fruit drawing to be a different color
        {
            if (!active) return;
            DrawCircle(x, y, 20, PURPLE);
        }

        void OnPickup(int &score, int &speedBoost, int &lives) override
        {
            score -= points; //bad fruit penalizes
            lives -= 1; //also decreases a life for catching a bad fruit
        }

        void OnMiss(int &score, int &lives) override
        {
            score += points; //reward for avoiding bad fruit
        }
};

int GetRandomGridX(int screenWidth, int gridCount) //splits the screen into 8 sections and returns a random x position within one of those sections for fruit spawning
{
    int gridWidth = screenWidth / gridCount; //width of each grid section
    int gridIndex = GetRandomValue(0, gridCount - 1); //randomly selects a grid section index
    return gridIndex * gridWidth + gridWidth / 2; //returns the x position of the center of the randomly selected grid section
}

void SpawnFruit(Fruit &fruit, int screenWidth, int speed) //initializes fruit properties and spawns it at a random x position above the screen
{
    fruit.x = GetRandomGridX(screenWidth, 8);
    fruit.y = -20; //start above the screen
    fruit.active = true;
    fruit.speed = speed; //update speed for non active fruits
}

struct LeaderboardEntry
{
    string name;
    int score;
};

bool LoadLeaderboard(const string &fileName, vector<LeaderboardEntry> &entries)
{
    entries.clear();
    ifstream file(fileName);
    if (!file.is_open())
        return false;

    string line;
    while (getline(file, line))
    {
        if (line.empty())
            continue;

        string name;
        int score;
        stringstream ss(line);
        if (ss >> name >> score)
        {
            entries.push_back({name, score});
        }
    }

    return true;
}

bool SaveLeaderboard(const string &fileName, const vector<LeaderboardEntry> &entries)
{
    ofstream file(fileName);
    if (!file.is_open())
        return false;

    for (const auto &entry : entries)
    {
        file << entry.name << " " << entry.score << "\n";
    }

    return true;
}

void AddScoreToLeaderboard(const string &fileName, const string &name, int score)
{
    vector<LeaderboardEntry> entries;
    LoadLeaderboard(fileName, entries);
    entries.push_back({name, score});

    sort(entries.begin(), entries.end(), [](const LeaderboardEntry &a, const LeaderboardEntry &b)
    {
        return a.score > b.score;
    });

    if (entries.size() > 5) //keep only top 5 scores
    {
        entries.resize(5);
    }

    SaveLeaderboard(fileName, entries);
}

int main () 
{
    const int fruitCount = 10; //fruit array size
    const int specialFruitCount = 1; // only 1 special fruit at a time can change later maybe
    const int badFruitCount = 1; // only 1 bad fruit at a time
    const int baseFruitSpeed = 5; // base fall speed
    const int basePlayerSpeed = 8; // base player speed

    const int screenWidth = 800; //screen dimensions
    const int screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "Fruit Catch Game"); //window title
    SetTargetFPS(60); //game fps
    
    Player player;
    Fruit fruits[fruitCount];
    SpecialFruit specialFruits[specialFruitCount]; // only 1 special fruit
    BadFruit badFruits[badFruitCount]; // only 1 bad fruit

    player.x = screenWidth / 2; //define player variables
    player.y = screenHeight;

    int score = 0; //player score
    int lives = 3; //player lives, can be used to end game after missing too many fruits
    int globalSpawnTimer = 0; //global timer to control fruit spawn timing
    int spawnDelay = 60; //delay between fruit spawns in frames (60 frames = 1 second at 60 FPS)
    int globalSpeedBoost = 0; //tracks the cumulative speed boost from special fruits
    const string leaderboardFile = "leaderboard.txt";
    vector<LeaderboardEntry> leaderboard;
    bool gameStarted = false;
    bool gameOver = false;
    bool scoreSaved = false;
    string currentName;
    LoadLeaderboard(leaderboardFile, leaderboard);
    
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
        if (!gameStarted)
        {
            BeginDrawing();
            ClearBackground(BLACK);
            DrawText("Fruit Catch Game", screenWidth / 2 - 220, screenHeight / 2 - 80, 50, YELLOW);
            DrawText("Press E to start", screenWidth / 2 - 150, screenHeight / 2 + 20, 30, WHITE);
            EndDrawing();

            if (IsKeyPressed(KEY_E))
            {
                gameStarted = true;
            }
            continue;
        }

        if (!gameOver && lives <= 0)
        {
            gameOver = true;
        }

        if (gameOver)
        {
            if (!scoreSaved)
            {
                int key = GetCharPressed();
                while (key > 0)
                {
                    if ((key >= 32) && (key <= 125) && (currentName.size() < 3))
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

                if (IsKeyPressed(KEY_BACKSPACE) && !currentName.empty())
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

            if (!scoreSaved)
            {
                string displayName = currentName;
                while (displayName.size() < 3)
                    displayName.push_back('_');

                DrawText("Enter your name:", screenWidth / 2 - 160, screenHeight / 2 + 40, 24, WHITE);
                DrawText(displayName.c_str(), screenWidth / 2 - 50, screenHeight / 2 + 80, 40, YELLOW);
                DrawText("Press ENTER when ready", screenWidth / 2 - 150, screenHeight / 2 + 140, 20, LIGHTGRAY);

                if (currentName.size() == 3 && IsKeyPressed(KEY_ENTER))
                {
                    AddScoreToLeaderboard(leaderboardFile, currentName, score);
                    LoadLeaderboard(leaderboardFile, leaderboard);
                    scoreSaved = true;
                }
            }
            else
            {
                DrawText("Top Scores:", screenWidth / 2 - 120, screenHeight / 2 + 60, 26, GOLD);

                for (int i = 0; i < (int)leaderboard.size() && i < 5; i++)
                {
                    DrawText(TextFormat("%d. %s - %d", i + 1, leaderboard[i].name.c_str(), leaderboard[i].score),
                             screenWidth / 2 - 140, screenHeight / 2 + 100 + i * 28, 20, RAYWHITE);
                }

                DrawText("Press R to restart", screenWidth / 2 - 180, screenHeight / 2 + 260, 22, YELLOW);
                DrawText("Press E to exit", screenWidth / 2 - 180, screenHeight / 2 + 300, 22, YELLOW);
                if (IsKeyPressed(KEY_R))
                {
                    score = 0;
                    lives = 3;
                    globalSpawnTimer = 0;
                    globalSpeedBoost = 0;
                    gameOver = false;
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
                else if (IsKeyPressed(KEY_E))
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

        for (int i = 0; i < fruitCount; i++) //update all active fruits to fall and check for collisions with the player
        {
            if (fruits[i].active)
            {
                fruits[i].UpdatePosition();
                Vector2 fruitPos = {(float)fruits[i].x, (float)fruits[i].y};
                Vector2 headPos = {(float)player.x, (float)player.y - 110};
                Rectangle bodyRect = {(float)player.x - 30, (float)player.y - 100, 60, 80};

                if (CheckCollisionCircles(headPos, 50, fruitPos, 20) || CheckCollisionCircleRec(fruitPos, 20, bodyRect))
                {
                    fruits[i].active = false;
                    fruits[i].OnPickup(score, globalSpeedBoost, lives);
                }
                else if (fruits[i].y > screenHeight)
                {
                    fruits[i].active = false;
                    fruits[i].OnMiss(score, lives);
                }
            }
        }

        for (int i = 0; i < specialFruitCount; i++) //update special fruits
        {
            if (specialFruits[i].active)
            {
                specialFruits[i].UpdatePosition();
                Vector2 fruitPos = {(float)specialFruits[i].x, (float)specialFruits[i].y};
                Vector2 headPos = {(float)player.x, (float)player.y - 110};
                Rectangle bodyRect = {(float)player.x - 30, (float)player.y - 100, 60, 80};

                if (CheckCollisionCircles(headPos, 50, fruitPos, 20) || CheckCollisionCircleRec(fruitPos, 20, bodyRect))
                {
                    specialFruits[i].active = false;
                    specialFruits[i].OnPickup(score, globalSpeedBoost, lives);
                }
                else if (specialFruits[i].y > screenHeight)
                {
                    specialFruits[i].active = false;
                    specialFruits[i].OnMiss(score, lives);
                }
            }
        }

        for (int i = 0; i < badFruitCount; i++) //update bad fruits
        {
            if (badFruits[i].active)
            {
                badFruits[i].UpdatePosition();
                Vector2 fruitPos = {(float)badFruits[i].x, (float)badFruits[i].y};
                Vector2 headPos = {(float)player.x, (float)player.y - 110};
                Rectangle bodyRect = {(float)player.x - 30, (float)player.y - 100, 60, 80};

                if (CheckCollisionCircles(headPos, 50, fruitPos, 20) || CheckCollisionCircleRec(fruitPos, 20, bodyRect))
                {
                    badFruits[i].active = false;
                    badFruits[i].OnPickup(score, globalSpeedBoost, lives);
                }
                else if (badFruits[i].y > screenHeight)
                {
                    badFruits[i].active = false;
                    badFruits[i].OnMiss(score, lives);
                }
            }
        }

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