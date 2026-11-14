#pragma once
#include <string>
#include <vector>

const int GRID_SIZE = 15;

enum Direction {
    ACROSS = 0,
    DOWN = 1
};

struct WordEntry {
    std::string word;
    std::string clue;
};

struct PlacedWord {
    int clueNumber;
    std::string word;
    std::string clue;
    int row;
    int col;
    Direction dir;
    bool isSolved;
};

struct CandidatePlacement {
    int row;
    int col;
    Direction dir;
    int crossings;

    // For Max-Heap priority queue comparison (more crossings = higher priority)
    bool operator<(const CandidatePlacement& other) const {
        return crossings < other.crossings;
    }
};

struct LeaderboardEntry {
    std::string playerName;
    int score;
    int timeTakenSeconds;
};
