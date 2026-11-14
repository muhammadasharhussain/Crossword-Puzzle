#pragma once
#include "types.h"
#include "grid.h"
#include "dictionary.h"
#include "generator.h"
#include "graph.h"
#include "bst.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>
#include <algorithm>
#include <sstream>

class CrosswordGame {
private:
    CrosswordGrid grid;
    Dictionary dictionary;
    std::vector<PlacedWord> placements;
    CrosswordGraph graph;
    LeaderboardBST leaderboard;
    std::string leaderboardFile = "data/leaderboard.txt";

    int score = 0;
    int hintsUsed = 0;
    std::chrono::steady_clock::time_point startTime;

public:
    CrosswordGame() {}

    bool initialize(const std::string& dictPath = "data/dictionary.txt") {
        leaderboard.loadFromFile(leaderboardFile);

        if (!dictionary.loadFromFile(dictPath)) {
            std::cout << "[Error] Could not load dictionary from " << dictPath << "\n";
            return false;
        }

        // Generate puzzle with 6 to 8 words
        bool success = false;
        std::random_device rd;
        for (int attempt = 0; attempt < 20; ++attempt) {
            unsigned int seed = rd() + attempt * 101;
            if (CrosswordGenerator::generate(dictionary.getAllWords(), 7, grid, placements, seed)) {
                // Check graph connectivity
                graph = CrosswordGraph::buildFromPlacements(placements);
                if (graph.isConnected() && placements.size() >= 4) {
                    success = true;
                    break;
                }
            }
        }

        if (!success) {
            std::cout << "[Warning] Generating fallback puzzle...\n";
            CrosswordGenerator::generate(dictionary.getAllWords(), 5, grid, placements, 42);
        }

        score = 0;
        hintsUsed = 0;
        startTime = std::chrono::steady_clock::now();
        return true;
    }

    void displayBoard() const {
        std::cout << "\n======================== CROSSWORD PUZZLE ========================\n";
        std::cout << "     ";
        for (int c = 0; c < GRID_SIZE; ++c) {
            std::cout << std::setw(2) << c << " ";
        }
        std::cout << "\n    +" << std::string(GRID_SIZE * 3, '-') << "+\n";

        for (int r = 0; r < GRID_SIZE; ++r) {
            std::cout << std::setw(2) << r << "  |";
            for (int c = 0; c < GRID_SIZE; ++c) {
                char ch = grid.getChar(r, c);
                if (ch == ' ') {
                    std::cout << " . "; // Empty/unused cell
                } else if (grid.isRevealed(r, c)) {
                    std::cout << " " << ch << " "; // Revealed letter
                } else {
                    // Check if a clue starts at this position
                    int clueNum = getClueStartingAt(r, c);
                    if (clueNum > 0) {
                        if (clueNum < 10) std::cout << " " << clueNum << " ";
                        else std::cout << clueNum << " ";
                    } else {
                        std::cout << " - "; // Hidden puzzle cell
                    }
                }
            }
            std::cout << "|\n";
        }
        std::cout << "    +" << std::string(GRID_SIZE * 3, '-') << "+\n";
    }

    void displayClues() const {
        std::cout << "\n============================= CLUES =============================\n";
        std::cout << "--- ACROSS ---\n";
        for (const auto& pw : placements) {
            if (pw.dir == ACROSS) {
                std::cout << "  " << pw.clueNumber << "A. " << pw.clue;
                if (pw.isSolved) std::cout << " [SOLVED: " << pw.word << "]";
                else std::cout << " (" << pw.word.length() << " letters)";
                std::cout << "\n";
            }
        }

        std::cout << "\n--- DOWN ---\n";
        for (const auto& pw : placements) {
            if (pw.dir == DOWN) {
                std::cout << "  " << pw.clueNumber << "D. " << pw.clue;
                if (pw.isSolved) std::cout << " [SOLVED: " << pw.word << "]";
                else std::cout << " (" << pw.word.length() << " letters)";
                std::cout << "\n";
            }
        }
        std::cout << "=================================================================\n";
    }

    bool isCompleted() const {
        for (const auto& pw : placements) {
            if (!pw.isSolved) return false;
        }
        return true;
    }

    void handleAnswer(const std::string& clueCode, const std::string& inputWord) {
        if (clueCode.empty()) return;

        char dirChar = std::toupper(clueCode.back());
        Direction targetDir = (dirChar == 'A') ? ACROSS : DOWN;
        std::string numPart = clueCode.substr(0, clueCode.size() - 1);

        int clueNum = 0;
        try {
            clueNum = std::stoi(numPart);
        } catch (...) {
            std::cout << ">> Invalid clue format! Use e.g. '1A' or '2D'.\n";
            return;
        }

        // Clean user word
        std::string cleaned = inputWord;
        std::transform(cleaned.begin(), cleaned.end(), cleaned.begin(), ::toupper);

        bool found = false;
        for (auto& pw : placements) {
            if (pw.clueNumber == clueNum && pw.dir == targetDir) {
                found = true;
                if (pw.isSolved) {
                    std::cout << ">> Clue " << clueCode << " is already solved!\n";
                    return;
                }
                if (cleaned == pw.word) {
                    pw.isSolved = true;
                    // Reveal all letters on grid
                    revealWord(pw);
                    score += 100;
                    std::cout << ">> Correct! Word \"" << pw.word << "\" placed into the grid! (+100 pts)\n";
                } else {
                    std::cout << ">> Incorrect guess for " << clueCode << ". Keep trying!\n";
                }
                break;
            }
        }

        if (!found) {
            std::cout << ">> Clue \"" << clueCode << "\" not found in current puzzle.\n";
        }
    }

    // Hint function: randomly reveals a single unrevealed letter in the grid
    void provideHint() {
        struct UnsolvedCell {
            int r, c;
        };
        std::vector<UnsolvedCell> candidates;

        for (const auto& pw : placements) {
            if (pw.isSolved) continue;
            int len = static_cast<int>(pw.word.length());
            for (int i = 0; i < len; ++i) {
                int r = (pw.dir == ACROSS) ? pw.row : pw.row + i;
                int c = (pw.dir == ACROSS) ? pw.col + i : pw.col;
                if (!grid.isRevealed(r, c)) {
                    candidates.push_back({r, c});
                }
            }
        }

        if (candidates.empty()) {
            std::cout << ">> No remaining unrevealed letters to give a hint for!\n";
            return;
        }

        std::random_device rd;
        std::mt19937 g(rd());
        std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
        UnsolvedCell chosen = candidates[dist(g)];

        grid.setRevealed(chosen.r, chosen.c, true);
        hintsUsed++;
        score = std::max(0, score - 15);

        std::cout << ">> [HINT] Letter '" << grid.getChar(chosen.r, chosen.c)
                  << "' revealed at (" << chosen.r << ", " << chosen.c << ")! (-15 pts)\n";

        // Check if any word became fully revealed
        checkAndUpdateFullyRevealedWords();
    }

    void run() {
        std::cout << "\n============================================================\n";
        std::cout << "            WELCOME TO DSA CLI CROSSWORD PUZZLE             \n";
        std::cout << "============================================================\n";
        std::cout << "Commands:\n";
        std::cout << "  <ClueCode> <WORD>  : Answer a clue (e.g. '1A STACK' or '2D HEAP')\n";
        std::cout << "  hint               : Randomly reveal one unsolved letter\n";
        std::cout << "  clues              : Redisplay clue list\n";
        std::cout << "  leaderboard        : Display top players\n";
        std::cout << "  quit               : Exit the game\n";
        std::cout << "============================================================\n";

        while (!isCompleted()) {
            displayBoard();
            displayClues();

            std::cout << "\nCurrent Score: " << score << " | Hints Used: " << hintsUsed << "\n";
            std::cout << "Enter command: > ";

            std::string line;
            if (!std::getline(std::cin, line)) break;

            // Trim whitespace
            std::stringstream ss(line);
            std::string cmd;
            if (!(ss >> cmd)) continue;

            std::string lowerCmd = cmd;
            std::transform(lowerCmd.begin(), lowerCmd.end(), lowerCmd.begin(), ::tolower);

            if (lowerCmd == "quit" || lowerCmd == "exit") {
                std::cout << ">> Exiting game. Goodbye!\n";
                return;
            } else if (lowerCmd == "hint") {
                provideHint();
            } else if (lowerCmd == "clues") {
                displayClues();
            } else if (lowerCmd == "leaderboard") {
                leaderboard.displayTop();
            } else {
                std::string wordGuess;
                if (ss >> wordGuess) {
                    handleAnswer(cmd, wordGuess);
                } else {
                    std::cout << ">> Unknown command or missing word guess. Format: <clue> <word> (e.g. '1A ARRAY')\n";
                }
            }
        }

        // Game completed
        auto endTime = std::chrono::steady_clock::now();
        int secondsTaken = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count());
        score = std::max(0, score + 200 - (secondsTaken / 3));

        displayBoard();
        std::cout << "\n🎉 CONGRATULATIONS! You have completed the entire crossword puzzle! 🎉\n";
        std::cout << "Final Score: " << score << "\n";
        std::cout << "Time Elapsed: " << secondsTaken << " seconds\n";

        std::cout << "\nEnter your name for the leaderboard: ";
        std::string playerName;
        if (std::getline(std::cin, playerName) && !playerName.empty()) {
            leaderboard.insert(playerName, score, secondsTaken);
            leaderboard.saveToFile(leaderboardFile);
            leaderboard.displayTop();
        }
    }

private:
    int getClueStartingAt(int r, int c) const {
        for (const auto& pw : placements) {
            if (pw.row == r && pw.col == c) {
                return pw.clueNumber;
            }
        }
        return -1;
    }

    void revealWord(const PlacedWord& pw) {
        int len = static_cast<int>(pw.word.length());
        for (int i = 0; i < len; ++i) {
            int r = (pw.dir == ACROSS) ? pw.row : pw.row + i;
            int c = (pw.dir == ACROSS) ? pw.col + i : pw.col;
            grid.setRevealed(r, c, true);
        }
    }

    void checkAndUpdateFullyRevealedWords() {
        for (auto& pw : placements) {
            if (pw.isSolved) continue;
            bool allRevealed = true;
            int len = static_cast<int>(pw.word.length());
            for (int i = 0; i < len; ++i) {
                int r = (pw.dir == ACROSS) ? pw.row : pw.row + i;
                int c = (pw.dir == ACROSS) ? pw.col + i : pw.col;
                if (!grid.isRevealed(r, c)) {
                    allRevealed = false;
                    break;
                }
            }
            if (allRevealed) {
                pw.isSolved = true;
                std::cout << ">> [Auto-Solved] \"" << pw.word << "\" is now completely revealed!\n";
            }
        }
    }
};
