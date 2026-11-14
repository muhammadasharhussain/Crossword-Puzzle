#pragma once
#include "types.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>

class Dictionary {
private:
    std::vector<WordEntry> words;
    // Map character -> list of indices in 'words' that contain this character
    std::unordered_map<char, std::vector<int>> letterIndexMap;

public:
    bool loadFromFile(const std::string& filepath) {
        words.clear();
        letterIndexMap.clear();

        std::ifstream file(filepath);
        if (!file.is_open()) {
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            // Trim carriage returns if Windows CRLF
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if (line.empty()) continue;

            std::stringstream ss(line);
            std::string word, clue;
            if (std::getline(ss, word, ',') && std::getline(ss, clue)) {
                // Convert word to uppercase
                std::transform(word.begin(), word.end(), word.begin(), ::toupper);
                
                int wordIdx = static_cast<int>(words.size());
                words.push_back({word, clue});

                // Index each character for fast intersection lookup
                for (char ch : word) {
                    letterIndexMap[ch].push_back(wordIdx);
                }
            }
        }
        return !words.empty();
    }

    const std::vector<WordEntry>& getAllWords() const {
        return words;
    }

    size_t size() const {
        return words.size();
    }

    std::vector<WordEntry> getRandomSubset(size_t count, unsigned int seed = 42) const {
        std::vector<WordEntry> copy = words;
        std::mt19937 g(seed);
        std::shuffle(copy.begin(), copy.end(), g);
        if (copy.size() > count) {
            copy.resize(count);
        }
        return copy;
    }
};
