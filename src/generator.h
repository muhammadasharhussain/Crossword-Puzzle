#pragma once
#include "types.h"
#include "grid.h"
#include "dictionary.h"
#include <vector>
#include <queue>
#include <stack>
#include <algorithm>
#include <random>

// Week 4 (Recursion & Stacks), Week 10 (Priority Queues & Max-Heap)
class CrosswordGenerator {
public:
    static bool generate(const std::vector<WordEntry>& pool,
                         int targetWordCount,
                         CrosswordGrid& grid,
                         std::vector<PlacedWord>& placedWords,
                         unsigned int seed = 12345) {
        grid.clear();
        placedWords.clear();

        if (pool.empty()) return false;

        std::vector<WordEntry> words = pool;
        std::mt19937 g(seed);
        std::shuffle(words.begin(), words.end(), g);

        // Place the 1st word horizontally in the center of the grid
        const WordEntry& first = words[0];
        int startR = GRID_SIZE / 2;
        int startC = std::max(0, (GRID_SIZE - static_cast<int>(first.word.length())) / 2);

        grid.place(first.word, startR, startC, ACROSS);
        placedWords.push_back({1, first.word, first.clue, startR, startC, ACROSS, false});

        // Track placed state using an undo stack (Week 4: Stacks)
        std::stack<PlacedWord> history;
        history.push(placedWords.back());

        std::vector<bool> usedInPuzzle(words.size(), false);
        usedInPuzzle[0] = true;

        // Iteratively try to place words with maximum crossings using Max-Heap
        for (int step = 1; step < targetWordCount && step < static_cast<int>(words.size()); ++step) {
            bool placedOne = false;

            for (size_t wIdx = 1; wIdx < words.size(); ++wIdx) {
                if (usedInPuzzle[wIdx]) continue;

                const std::string& candidateWord = words[wIdx].word;

                // Week 10: Max-Heap Priority Queue to rank best placements
                std::priority_queue<CandidatePlacement> maxHeap;

                // Evaluate every possible coordinate and direction
                for (int r = 0; r < GRID_SIZE; ++r) {
                    for (int c = 0; c < GRID_SIZE; ++c) {
                        for (Direction dir : {ACROSS, DOWN}) {
                            if (grid.canPlace(candidateWord, r, c, dir, false)) {
                                int crossings = grid.countCrossings(candidateWord, r, c, dir);
                                maxHeap.push({r, c, dir, crossings});
                            }
                        }
                    }
                }

                // If valid placements exist, pick the best candidate (top of max-heap)
                if (!maxHeap.empty()) {
                    CandidatePlacement best = maxHeap.top();
                    grid.place(candidateWord, best.row, best.col, best.dir);
                    placedWords.push_back({
                        0, // will assign clue number later
                        candidateWord,
                        words[wIdx].clue,
                        best.row,
                        best.col,
                        best.dir,
                        false
                    });
                    usedInPuzzle[wIdx] = true;
                    placedOne = true;
                    break;
                }
            }

            if (!placedOne) {
                // If no more words can cleanly cross, proceed with current count
                break;
            }
        }

        if (placedWords.empty()) return false;

        // Number clues based on top-left to bottom-right order
        assignClueNumbers(placedWords);

        return true;
    }

private:
    static void assignClueNumbers(std::vector<PlacedWord>& words) {
        // Sort by row, then col
        std::sort(words.begin(), words.end(), [](const PlacedWord& a, const PlacedWord& b) {
            if (a.row != b.row) return a.row < b.row;
            return a.col < b.col;
        });

        int currentClue = 1;
        for (size_t i = 0; i < words.size(); ++i) {
            if (i > 0 && words[i].row == words[i - 1].row && words[i].col == words[i - 1].col) {
                words[i].clueNumber = words[i - 1].clueNumber; // share same cell number
            } else {
                words[i].clueNumber = currentClue++;
            }
        }
    }
};
