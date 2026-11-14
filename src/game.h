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

// ANSI color escape sequences
namespace Color {
    const char* const RESET       = "\033[0m";
    const char* const BOLD        = "\033[1m";
    const char* const DIM         = "\033[2m";
    const char* const RED         = "\033[31m";
    const char* const GREEN       = "\033[32m";
    const char* const YELLOW      = "\033[33m";
    const char* const BLUE        = "\033[34m";
    const char* const MAGENTA     = "\033[35m";
    const char* const CYAN        = "\033[36m";
    const char* const WHITE       = "\033[37m";
    const char* const BG_DARK     = "\033[48;5;236m";
    const char* const BRIGHT_CYAN = "\033[96m";
    const char* const BRIGHT_YEL  = "\033[93m";
}

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
            std::cout << Color::RED << "[Error] Could not load dictionary from " << dictPath << Color::RESET << "\n";
            return false;
        }

        // Generate puzzle with 6 to 8 words
        bool success = false;
        std::random_device rd;
        for (int attempt = 0; attempt < 25; ++attempt) {
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
            std::cout << Color::YELLOW << "[Notice] Generating standard layout...\n" << Color::RESET;
            CrosswordGenerator::generate(dictionary.getAllWords(), 5, grid, placements, 42);
        }

        score = 0;
        hintsUsed = 0;
        startTime = std::chrono::steady_clock::now();
        return true;
    }

    void displayBoard() const {
        std::cout << "\n" << Color::CYAN << Color::BOLD 
                  << "========================== CROSSWORD PUZZLE ==========================" 
                  << Color::RESET << "\n";
        
        std::cout << "     ";
        for (int c = 0; c < GRID_SIZE; ++c) {
            std::cout << Color::DIM << std::setw(2) << c << " " << Color::RESET;
        }
        std::cout << "\n    " << Color::DIM << "+" << std::string(GRID_SIZE * 3, '-') << "+" << Color::RESET << "\n";

        for (int r = 0; r < GRID_SIZE; ++r) {
            std::cout << Color::DIM << std::setw(2) << r << "  |" << Color::RESET;
            for (int c = 0; c < GRID_SIZE; ++c) {
                char ch = grid.getChar(r, c);
                if (ch == ' ') {
                    // Empty cell outside puzzle: clean space (no fullstops)
                    std::cout << "   ";
                } else if (grid.isRevealed(r, c)) {
                    // Solved/revealed letter in bright bold green
                    std::cout << Color::GREEN << Color::BOLD << " " << ch << " " << Color::RESET;
                } else {
                    // Check if a clue starts at this position
                    int clueNum = getClueStartingAt(r, c);
                    if (clueNum > 0) {
                        // Clue number in bold yellow
                        std::cout << Color::YELLOW << Color::BOLD;
                        if (clueNum < 10) std::cout << " " << clueNum << " ";
                        else std::cout << clueNum << " ";
                        std::cout << Color::RESET;
                    } else {
                        // Unsolved puzzle slot: clean dash in cyan
                        std::cout << Color::CYAN << " - " << Color::RESET;
                    }
                }
            }
            std::cout << Color::DIM << "|" << Color::RESET << "\n";
        }
        std::cout << "    " << Color::DIM << "+" << std::string(GRID_SIZE * 3, '-') << "+" << Color::RESET << "\n";
    }

    void displayClues() const {
        std::cout << "\n" << Color::MAGENTA << Color::BOLD 
                  << "=============================== CLUES ===============================" 
                  << Color::RESET << "\n";
        
        std::cout << Color::BRIGHT_YEL << Color::BOLD << "--- ACROSS ---" << Color::RESET << "\n";
        for (const auto& pw : placements) {
            if (pw.dir == ACROSS) {
                std::cout << Color::BOLD << "  " << pw.clueNumber << "A. " << Color::RESET << pw.clue;
                if (pw.isSolved) {
                    std::cout << Color::GREEN << Color::BOLD << " [SOLVED: " << pw.word << "]" << Color::RESET;
                } else {
                    std::cout << Color::DIM << " (" << pw.word.length() << " letters)" << Color::RESET;
                }
                std::cout << "\n";
            }
        }

        std::cout << "\n" << Color::BRIGHT_YEL << Color::BOLD << "--- DOWN ---" << Color::RESET << "\n";
        for (const auto& pw : placements) {
            if (pw.dir == DOWN) {
                std::cout << Color::BOLD << "  " << pw.clueNumber << "D. " << Color::RESET << pw.clue;
                if (pw.isSolved) {
                    std::cout << Color::GREEN << Color::BOLD << " [SOLVED: " << pw.word << "]" << Color::RESET;
                } else {
                    std::cout << Color::DIM << " (" << pw.word.length() << " letters)" << Color::RESET;
                }
                std::cout << "\n";
            }
        }
        std::cout << Color::MAGENTA << "=====================================================================" << Color::RESET << "\n";
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
            std::cout << Color::RED << ">> Invalid clue format! Use format like '1A' or '2D'." << Color::RESET << "\n";
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
                    std::cout << Color::YELLOW << ">> Clue " << clueCode << " is already solved!" << Color::RESET << "\n";
                    return;
                }
                if (cleaned == pw.word) {
                    pw.isSolved = true;
                    revealWord(pw);
                    score += 100;
                    std::cout << Color::GREEN << Color::BOLD 
                              << ">> Correct! Word \"" << pw.word << "\" placed into the grid! (+100 pts)" 
                              << Color::RESET << "\n";
                } else {
                    std::cout << Color::RED << ">> Incorrect guess for " << clueCode << ". Keep trying!" << Color::RESET << "\n";
                }
                break;
            }
        }

        if (!found) {
            std::cout << Color::RED << ">> Clue \"" << clueCode << "\" not found in current puzzle." << Color::RESET << "\n";
        }
    }

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
            std::cout << Color::YELLOW << ">> No remaining unrevealed letters to give a hint for!" << Color::RESET << "\n";
            return;
        }

        std::random_device rd;
        std::mt19937 g(rd());
        std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
        UnsolvedCell chosen = candidates[dist(g)];

        grid.setRevealed(chosen.r, chosen.c, true);
        hintsUsed++;
        score = std::max(0, score - 15);

        std::cout << Color::YELLOW << Color::BOLD 
                  << ">> [HINT] Letter '" << grid.getChar(chosen.r, chosen.c)
                  << "' revealed at row " << chosen.r << ", col " << chosen.c 
                  << "! (-15 pts)" << Color::RESET << "\n";

        checkAndUpdateFullyRevealedWords();
    }

    void run() {
        std::cout << "\n" << Color::CYAN << Color::BOLD 
                  << "=====================================================================\n"
                  << "                     CLI CROSSWORD PUZZLE GAME                       \n"
                  << "=====================================================================" 
                  << Color::RESET << "\n";
        std::cout << Color::BOLD << "Commands:\n" << Color::RESET;
        std::cout << "  " << Color::BRIGHT_CYAN << "<ClueCode> <WORD>" << Color::RESET 
                  << " : Answer a clue (e.g. '1A APPLE' or '2D WATER')\n";
        std::cout << "  " << Color::BRIGHT_CYAN << "hint" << Color::RESET 
                  << "              : Randomly reveal one unsolved letter (-15 pts)\n";
        std::cout << "  " << Color::BRIGHT_CYAN << "clues" << Color::RESET 
                  << "             : Redisplay clue list\n";
        std::cout << "  " << Color::BRIGHT_CYAN << "leaderboard" << Color::RESET 
                  << "       : Display top players from BST\n";
        std::cout << "  " << Color::BRIGHT_CYAN << "quit" << Color::RESET 
                  << "              : Exit the game\n";
        std::cout << Color::CYAN << "=====================================================================" << Color::RESET << "\n";

        while (!isCompleted()) {
            displayBoard();
            displayClues();

            std::cout << "\n" << Color::BOLD << "Current Score: " << Color::GREEN << score << Color::RESET 
                      << Color::BOLD << " | Hints Used: " << Color::YELLOW << hintsUsed << Color::RESET << "\n";
            std::cout << Color::BOLD << "Enter command: > " << Color::RESET;

            std::string line;
            if (!std::getline(std::cin, line)) break;

            std::stringstream ss(line);
            std::string cmd;
            if (!(ss >> cmd)) continue;

            std::string lowerCmd = cmd;
            std::transform(lowerCmd.begin(), lowerCmd.end(), lowerCmd.begin(), ::tolower);

            if (lowerCmd == "quit" || lowerCmd == "exit") {
                std::cout << Color::YELLOW << ">> Exiting game. Goodbye!\n" << Color::RESET;
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
                    std::cout << Color::RED 
                              << ">> Unknown command or missing word guess. Format: <clue> <word> (e.g. '1A APPLE')\n" 
                              << Color::RESET;
                }
            }
        }

        // Game completed
        auto endTime = std::chrono::steady_clock::now();
        int secondsTaken = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count());
        score = std::max(0, score + 200 - (secondsTaken / 3));

        displayBoard();
        std::cout << "\n" << Color::GREEN << Color::BOLD 
                  << "=====================================================================\n"
                  << "    CONGRATULATIONS! You have completed the entire crossword puzzle! \n"
                  << "=====================================================================" 
                  << Color::RESET << "\n";
        std::cout << "Final Score: " << Color::GREEN << Color::BOLD << score << Color::RESET << "\n";
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
                std::cout << Color::GREEN << Color::BOLD 
                          << ">> [Auto-Solved] \"" << pw.word << "\" is now completely revealed!" 
                          << Color::RESET << "\n";
            }
        }
    }
};
