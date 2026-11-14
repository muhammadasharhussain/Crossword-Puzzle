#pragma once
#include "types.h"
#include <vector>
#include <string>
#include <iostream>

class CrosswordGrid {
private:
    char board[GRID_SIZE][GRID_SIZE];
    int uses[GRID_SIZE][GRID_SIZE];      // Tracks how many words share each cell
    bool revealed[GRID_SIZE][GRID_SIZE];  // Tracks which cells are revealed to player

public:
    CrosswordGrid() {
        clear();
    }

    void clear() {
        for (int r = 0; r < GRID_SIZE; ++r) {
            for (int c = 0; c < GRID_SIZE; ++c) {
                board[r][c] = ' ';
                uses[r][c] = 0;
                revealed[r][c] = false;
            }
        }
    }

    char getChar(int r, int c) const {
        if (r < 0 || r >= GRID_SIZE || c < 0 || c >= GRID_SIZE) return ' ';
        return board[r][c];
    }

    bool isRevealed(int r, int c) const {
        if (r < 0 || r >= GRID_SIZE || c < 0 || c >= GRID_SIZE) return false;
        return revealed[r][c];
    }

    void setRevealed(int r, int c, bool val) {
        if (r >= 0 && r < GRID_SIZE && c >= 0 && c < GRID_SIZE) {
            revealed[r][c] = val;
        }
    }

    int getUses(int r, int c) const {
        if (r < 0 || r >= GRID_SIZE || c < 0 || c >= GRID_SIZE) return 0;
        return uses[r][c];
    }

    // Check if word can be placed legally according to crossword rules
    // isFirstWord allows placing without requiring an existing crossing letter
    bool canPlace(const std::string& word, int row, int col, Direction dir, bool isFirstWord = false) const {
        int len = static_cast<int>(word.length());

        // 1. Grid boundary check
        if (dir == ACROSS) {
            if (row < 0 || row >= GRID_SIZE || col < 0 || col + len > GRID_SIZE) return false;
        } else {
            if (row < 0 || row + len > GRID_SIZE || col < 0 || col >= GRID_SIZE) return false;
        }

        // 2. Cell before start must be empty (boundary check)
        int beforeR = (dir == ACROSS) ? row : row - 1;
        int beforeC = (dir == ACROSS) ? col - 1 : col;
        if (beforeR >= 0 && beforeR < GRID_SIZE && beforeC >= 0 && beforeC < GRID_SIZE) {
            if (board[beforeR][beforeC] != ' ') return false;
        }

        // 3. Cell after end must be empty (boundary check)
        int afterR = (dir == ACROSS) ? row : row + len;
        int afterC = (dir == ACROSS) ? col + len : col;
        if (afterR >= 0 && afterR < GRID_SIZE && afterC >= 0 && afterC < GRID_SIZE) {
            if (board[afterR][afterC] != ' ') return false;
        }

        int crossings = 0;

        // 4. Check each letter of the candidate word
        for (int i = 0; i < len; ++i) {
            int r = (dir == ACROSS) ? row : row + i;
            int c = (dir == ACROSS) ? col + i : col;

            if (board[r][c] != ' ') {
                // Must match existing letter exactly
                if (board[r][c] != word[i]) {
                    return false;
                }
                crossings++;
            } else {
                // Newly placed letter cannot touch parallel/adjacent words on flanks
                if (dir == ACROSS) {
                    if (r - 1 >= 0 && board[r - 1][c] != ' ') return false;
                    if (r + 1 < GRID_SIZE && board[r + 1][c] != ' ') return false;
                } else {
                    if (c - 1 >= 0 && board[r][c - 1] != ' ') return false;
                    if (c + 1 < GRID_SIZE && board[r][c + 1] != ' ') return false;
                }
            }
        }

        // Subsequent words MUST intersect at least one already placed word
        if (!isFirstWord && crossings == 0) {
            return false;
        }

        return true;
    }

    int countCrossings(const std::string& word, int row, int col, Direction dir) const {
        int len = static_cast<int>(word.length());
        int crossings = 0;
        for (int i = 0; i < len; ++i) {
            int r = (dir == ACROSS) ? row : row + i;
            int c = (dir == ACROSS) ? col + i : col;
            if (board[r][c] == word[i]) {
                crossings++;
            }
        }
        return crossings;
    }

    // Place word on grid and increment reference count for shared cells
    void place(const std::string& word, int row, int col, Direction dir) {
        int len = static_cast<int>(word.length());
        for (int i = 0; i < len; ++i) {
            int r = (dir == ACROSS) ? row : row + i;
            int c = (dir == ACROSS) ? col + i : col;
            board[r][c] = word[i];
            uses[r][c]++;
        }
    }

    // Unplace / backtrack word, only clearing cells whose uses drop to 0
    void remove(const std::string& word, int row, int col, Direction dir) {
        int len = static_cast<int>(word.length());
        for (int i = 0; i < len; ++i) {
            int r = (dir == ACROSS) ? row : row + i;
            int c = (dir == ACROSS) ? col + i : col;
            uses[r][c]--;
            if (uses[r][c] <= 0) {
                board[r][c] = ' ';
                uses[r][c] = 0;
            }
        }
    }
};
