#pragma once
#include "types.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>

// Week 1 DSA: Arrays and Vector ADT (No Hash Maps / Hashing)
class Dictionary {
private:
    std::vector<WordEntry> words;

public:
    bool loadFromFile(const std::string& filepath) {
        words.clear();

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
                words.push_back({word, clue});
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
