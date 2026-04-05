#pragma once

#include <string>
#include <vector>

using namespace std;

struct LeaderboardEntry //struct to hold a player's name and score for the leaderboard
{
    string name;
    int score;
};

//function declarations for loading, saving, and adding scores to the leaderboard
bool LoadLeaderboard(const string &fileName, vector<LeaderboardEntry> &entries);
bool SaveLeaderboard(const string &fileName, const vector<LeaderboardEntry> &entries);
void AddScoreToLeaderboard(const string &fileName, const string &name, int score);
