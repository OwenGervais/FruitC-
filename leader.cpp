#include "leader.hpp"
#include <fstream>
#include <algorithm>
#include <sstream>

using namespace std;

bool LoadLeaderboard(const string &fileName, vector<LeaderboardEntry> &entries)
{//loads leaderboard data from the file
    entries.clear(); //clear existing entries before loading new data
    ifstream file(fileName);
    if (!file.is_open()) return false; //if the file doesn't exist or can't be opened, return false
 
    string line;
    while (getline(file, line)) //read each line of the file, which should contain a name and score separated by space
    {
        if (line.empty()) continue;

        string name;
        int score;
        stringstream ss(line);
        if (ss >> name >> score) //if the line is properly formatted, add it to the entries vector
        {
            entries.push_back({name, score});
        }
    }

    return true;
}

bool SaveLeaderboard(const string &fileName, const vector<LeaderboardEntry> &entries)
{ //saves the leaderboard data to the file, overwriting existing data
    ofstream file(fileName);
    if (!file.is_open()) return false;

    for (const auto &entry : entries) //write each entry to the file in the format "name score"
    {
        file << entry.name << " " << entry.score << "\n";
    }

    return true;
}

void AddScoreToLeaderboard(const string &fileName, const string &name, int score)
{ //adds a new score to the leaderboard, keeping only the top 5 scores sorted in descending order
    vector<LeaderboardEntry> entries;
    LoadLeaderboard(fileName, entries);
    entries.push_back({name, score});

    sort(entries.begin(), entries.end(), [](const LeaderboardEntry &a, const LeaderboardEntry &b)
    { //sort in descending order based on score
        return a.score > b.score;
    });

    if (entries.size() > 5) //keep only top 5 scores
    {
        entries.resize(5);
    }

    SaveLeaderboard(fileName, entries);
}
