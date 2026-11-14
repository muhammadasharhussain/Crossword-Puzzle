#pragma once
#include "types.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

// Week 11-12 DSA: Binary Search Tree (BST) for Leaderboard
struct BSTNode {
    LeaderboardEntry data;
    BSTNode* left;
    BSTNode* right;

    BSTNode(const LeaderboardEntry& entry)
        : data(entry), left(nullptr), right(nullptr) {}
};

class LeaderboardBST {
private:
    BSTNode* root;

    // BST Insertion ordered by score (higher score goes to right)
    BSTNode* insertHelper(BSTNode* node, const LeaderboardEntry& entry) {
        if (!node) {
            return new BSTNode(entry);
        }
        if (entry.score < node->data.score) {
            node->left = insertHelper(node->left, entry);
        } else {
            node->right = insertHelper(node->right, entry);
        }
        return node;
    }

    // Reverse Inorder Traversal (Right -> Root -> Left) to display scores highest to lowest
    void reverseInorder(BSTNode* node, int& rank, int maxEntries) const {
        if (!node || rank > maxEntries) return;

        reverseInorder(node->right, rank, maxEntries);

        if (rank <= maxEntries) {
            std::cout << "  " << rank << ". " << node->data.playerName
                      << " - " << node->data.score << " pts ("
                      << node->data.timeTakenSeconds << "s)\n";
            rank++;
        }

        reverseInorder(node->left, rank, maxEntries);
    }

    // Postorder Traversal to safely delete nodes and prevent memory leaks
    void destroyTree(BSTNode* node) {
        if (!node) return;
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }

    void collectAll(BSTNode* node, std::vector<LeaderboardEntry>& list) const {
        if (!node) return;
        collectAll(node->left, list);
        list.push_back(node->data);
        collectAll(node->right, list);
    }

public:
    LeaderboardBST() : root(nullptr) {}

    ~LeaderboardBST() {
        destroyTree(root);
    }

    void insert(const std::string& name, int score, int timeSeconds) {
        LeaderboardEntry entry{name, score, timeSeconds};
        root = insertHelper(root, entry);
    }

    void displayTop(int maxEntries = 5) const {
        std::cout << "\n================= LEADERBOARD (Top Scores) =================\n";
        if (!root) {
            std::cout << "  No records yet! Be the first to complete a puzzle!\n";
        } else {
            int rank = 1;
            reverseInorder(root, rank, maxEntries);
        }
        std::cout << "============================================================\n\n";
    }

    void saveToFile(const std::string& filepath) const {
        std::ofstream file(filepath);
        if (!file.is_open()) return;

        std::vector<LeaderboardEntry> list;
        collectAll(root, list);
        for (const auto& entry : list) {
            file << entry.playerName << "," << entry.score << "," << entry.timeTakenSeconds << "\n";
        }
    }

    void loadFromFile(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) return;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            size_t p1 = line.find(',');
            size_t p2 = line.rfind(',');
            if (p1 != std::string::npos && p2 != std::string::npos && p1 != p2) {
                std::string name = line.substr(0, p1);
                int score = std::stoi(line.substr(p1 + 1, p2 - p1 - 1));
                int timeSec = std::stoi(line.substr(p2 + 1));
                insert(name, score, timeSec);
            }
        }
    }
};
