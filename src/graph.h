#pragma once
#include "types.h"
#include <vector>
#include <queue>

// Week 13 DSA: Graph Adjacency List & BFS/DFS Traversal
class CrosswordGraph {
private:
    int numVertices;
    std::vector<std::vector<int>> adjList;

public:
    CrosswordGraph(int vertices = 0) : numVertices(vertices) {
        adjList.resize(vertices);
    }

    void reset(int vertices) {
        numVertices = vertices;
        adjList.clear();
        adjList.resize(vertices);
    }

    void addEdge(int u, int v) {
        if (u >= 0 && u < numVertices && v >= 0 && v < numVertices && u != v) {
            adjList[u].push_back(v);
            adjList[v].push_back(u);
        }
    }

    // Build word intersection graph: words are nodes, shared cells are edges
    static CrosswordGraph buildFromPlacements(const std::vector<PlacedWord>& placements) {
        int n = static_cast<int>(placements.size());
        CrosswordGraph g(n);

        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                // If words intersect at the same coordinate, connect them with an edge
                if (doIntersect(placements[i], placements[j])) {
                    g.addEdge(i, j);
                }
            }
        }
        return g;
    }

    // BFS Graph Traversal to verify all words form a single connected component
    bool isConnected() const {
        if (numVertices <= 1) return true;

        std::vector<bool> visited(numVertices, false);
        std::queue<int> q; // Week 5: Queue ADT

        visited[0] = true;
        q.push(0);
        int visitedCount = 0;

        while (!q.empty()) {
            int u = q.front();
            q.pop();
            visitedCount++;

            for (int neighbor : adjList[u]) {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    q.push(neighbor);
                }
            }
        }

        return visitedCount == numVertices;
    }

private:
    static bool doIntersect(const PlacedWord& a, const PlacedWord& b) {
        // Parallel words cannot cross each other in standard grid crossword
        if (a.dir == b.dir) return false;

        const PlacedWord& across = (a.dir == ACROSS) ? a : b;
        const PlacedWord& down = (a.dir == DOWN) ? a : b;

        // Check if across word spans down's col, and down spans across's row
        bool colInRange = (down.col >= across.col && down.col < across.col + static_cast<int>(across.word.length()));
        bool rowInRange = (across.row >= down.row && across.row < down.row + static_cast<int>(down.word.length()));

        return colInRange && rowInRange;
    }
};
