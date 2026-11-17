# Crossword Puzzle CLI (DSA Course Project) - Formal Project Report
**Course:** CT159 Data Structures, Algorithms & Applications  
**Institution:** NED University of Engineering & Technology, Department of Computer Science & IT  
**Scope:** Weeks 1 to 13 (Strictly Adhered, Zero Hash Maps)  
**Repository:** [muhammadasharhussain/Crossword-Puzzle](https://github.com/muhammadasharhussain/Crossword-Puzzle)

---

## 1. General Project Overview & Objectives

The primary objective of this project is to design and engineer an interactive, terminal-based **Crossword Puzzle Game** developed in standard C++17. The program highlights the practical synthesis of core Data Structures and Algorithms taught across Weeks 1 through 13 of the CT159 curriculum.

### Core Features & Algorithmic Insights:
- **Procedural Crossword Synthesis:** Rather than loading static, predetermined puzzle templates, the engine synthesizes unique, solvable crosswords in real-time from a vocabulary pool.
- **Greedy Optimization via Max-Heap:** Employs an overloaded Priority Queue to evaluate all grid coordinates in $O(\log K)$ time, greedily selecting coordinates that maximize crossing density.
- **Topological Integrity via Graph BFS:** Formulates words as vertices and letter crossings as undirected edges, using a FIFO queue to guarantee single-component puzzle coherence (no disconnected islands).
- **Dynamic Score Tracking & Leaderboard:** Utilizes a custom, pointer-based Binary Search Tree (BST) to maintain sorted high scores, rendered via Reverse Inorder traversal without auxiliary array sorting.
- **In-Place Terminal Visuals:** Features ANSI escape code coloring with native Win32 Console buffer clearing to prevent terminal re-printing clutter.
- **Dual-Mode Hint System:** Supports both global random hints and targeted clue-specific letter reveals with custom score penalties.

---

## 2. DSA Curriculum Mapping & Complexity Matrix

| Week | Data Structure / Algorithm | File Location | Purpose & Implementation | Time Complexity | Space Complexity |
|---|---|---|---|---|---|
| **Week 1** | 2D Arrays / Matrices | `src/grid.h` | 15x15 board matrix and `uses[15][15]` reference counter | $O(1)$ random cell access | $O(N^2)$ with $N=15$ |
| **Week 1** | Sequential Vector ADT | `src/dictionary.h` | Contiguous array storage for dictionary words | $O(1)$ append / traversal | $O(W)$ words in memory |
| **Week 2-3**| Dynamic Linked Nodes | `src/bst.h` | Dynamic tree node allocation (`BSTNode* left, right`) | $O(1)$ node creation | $O(M)$ allocated nodes |
| **Week 4** | Recursion & Stacks | `src/generator.h` | Recursive backtracking & undo stack for placement state | $O(1)$ push / pop | $O(K)$ stack depth |
| **Week 5** | Queues (FIFO) | `src/graph.h` | Breadth-First Search (BFS) traversal queue for graph validation | $O(1)$ enqueue / dequeue | $O(V)$ queue capacity |
| **Week 6-7**| Sorting (Quick / Merge) | `src/generator.h` | Clue ordering top-to-bottom and left-to-right via coordinate comparator | $O(V \log V)$ | $O(\log V)$ auxiliary |
| **Week 9** | Tree Traversals | `src/bst.h` | Reverse Inorder (descending rankings) & Postorder (memory deallocation) | $O(M)$ complete traversal | $O(H)$ recursion stack |
| **Week 10**| Binary Heap / Priority Queue| `src/generator.h` | Max-Heap ranking candidate placements by intersection crossings | $O(\log K)$ insert / extract | $O(K)$ candidate pool |
| **Week 11-12**| Binary Search Tree (BST)| `src/bst.h` | Dynamic leaderboard maintaining sorted scores | Avg $O(\log M)$, Worst $O(M)$ | $O(M)$ nodes |
| **Week 13**| Graph Adjacency List & BFS | `src/graph.h` | Undirected intersection graph and BFS connectivity check | Graph: $O(V^2)$, BFS: $O(V + E)$ | $O(V + E)$ |

---

## 3. High-Level System Architecture & Flow

### 3.1 Architecture Block Diagram

```
+-----------------------------------------------------------------------------------------+
|                                      main.cpp                                           |
|       - Enables ANSI Virtual Terminal processing on Windows (SetConsoleMode)            |
|       - Instantiates and starts CrosswordGame                                           |
+-------------------------------------------+---------------------------------------------+
                                            |
+-------------------------------------------v---------------------------------------------+
|                                    CrosswordGame (src/game.h)                           |
|       - Prompts player for Difficulty (Easy: 4 words, Medium: 7, Hard: 10)              |
|       - Native Windows buffer clearing (FillConsoleOutputCharacter / SetConsoleCursor)  |
|       - Interactive Game Loop: input parser, timer, hints (random & targeted), score    |
+----+----------------------+-----------------------+----------------------+--------------+
     |                      |                       |                      |
+----v------------+  +------v-------------+  +------v------------+  +------v--------------+
|  Dictionary     |  |   CrosswordGrid    |  | CrosswordGenerator|  | CrosswordGraph      |
| (dictionary.h)  |  |    (src/grid.h)    |  | (src/generator.h) |  |   (src/graph.h)     |
| - Sequential    |  | - 15x15 char board |  | - Backtracking    |  | - Adjacency list    |
|   Array/Vector  |  | - uses[r][c] ref   |  | - Max-Heap order  |  | - BFS queue         |
|   (Week 1 ADT)  |  |   counter          |  | - Placement stack |  | - Verifies 1 island |
+-----------------+  | - reveal masks     |  +-------------------+  +---------------------+
                     +--------------------+                                |
                                                                   +-------v-------------+
                                                                   |  LeaderboardBST     |
                                                                   |    (src/bst.h)      |
                                                                   | - Dynamic BST Nodes |
                                                                   | - Reverse Inorder   |
                                                                   | - Postorder free    |
+------------------------------------------------------------------+---------------------+
|                                    src/types.h                                          |
|      Shared definitions: Direction, WordEntry, PlacedWord, CandidatePlacement           |
+-----------------------------------------------------------------------------------------+
```

---

## 4. Detailed Component Implementation & Code Walkthrough

### 4.1 Grid Reference Counting (`src/grid.h`)
Crosswords inherently involve shared intersecting letters. Simply zeroing a cell upon word removal breaks crossing words. To solve this, a parallel reference counter matrix `int uses[15][15]` is maintained.

```cpp
void place(const std::string& word, int row, int col, Direction dir) {
    int len = static_cast<int>(word.length());
    for (int i = 0; i < len; ++i) {
        int r = (dir == ACROSS) ? row : row + i;
        int c = (dir == ACROSS) ? col + i : col;
        board[r][c] = word[i];
        uses[r][c]++; // Reference count increment
    }
}

void remove(const std::string& word, int row, int col, Direction dir) {
    int len = static_cast<int>(word.length());
    for (int i = 0; i < len; ++i) {
        int r = (dir == ACROSS) ? row : row + i;
        int c = (dir == ACROSS) ? col + i : col;
        uses[r][c]--; // Decrement counter
        if (uses[r][c] <= 0) {
            board[r][c] = ' '; // Only clear when no other word uses this cell
            uses[r][c] = 0;
        }
    }
}
```

### 4.2 Candidate Ranking via Max-Heap (`src/generator.h`)
Candidate placements across all 225 coordinates are filtered through `canPlace()`. Valid candidates are inserted into a `std::priority_queue<CandidatePlacement>` (Max-Heap), which uses an overloaded `operator<` to float candidate coordinates with maximum crossings to the top.

```cpp
struct CandidatePlacement {
    int row, col;
    Direction dir;
    int crossings;
    bool operator<(const CandidatePlacement& other) const {
        return crossings < other.crossings; // Higher crossings = Top of Heap
    }
};

// In CrosswordGenerator::generate():
std::priority_queue<CandidatePlacement> maxHeap;
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
if (!maxHeap.empty()) {
    CandidatePlacement best = maxHeap.top(); // O(1) retrieval of best crossing
    grid.place(candidateWord, best.row, best.col, best.dir);
}
```

### 4.3 Topological Graph Verification via BFS (`src/graph.h`)
Placed words are represented as vertices and shared cells as edges. BFS traversal starting from vertex 0 traverses the graph. If `visitedCount == numVertices`, the crossword forms one single interconnected structure.

```cpp
bool isConnected() const {
    if (numVertices <= 1) return true;
    std::vector<bool> visited(numVertices, false);
    std::queue<int> q; // Week 5: FIFO Queue ADT

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
    return visitedCount == numVertices; // Guaranteed single connected component
}
```

### 4.4 BST Leaderboard & Tree Traversals (`src/bst.h`)
High scores are stored in a Binary Search Tree. Reverse Inorder traversal (`Right -> Root -> Left`) prints ranked scores descending in $O(M)$ without re-sorting. Postorder traversal (`Left -> Right -> Root`) ensures child nodes are deallocated before parent nodes.

```cpp
// Reverse Inorder: Right -> Root -> Left (Prints Highest Scores First)
void reverseInorder(BSTNode* node, int& rank, int maxEntries) const {
    if (!node || rank > maxEntries) return;
    reverseInorder(node->right, rank, maxEntries); // Visit higher scores
    if (rank <= maxEntries) {
        std::cout << rank << ". " << node->data.playerName 
                  << " - " << node->data.score << " pts\n";
        rank++;
    }
    reverseInorder(node->left, rank, maxEntries);  // Visit lower scores
}

// Postorder Traversal: Left -> Right -> Root (Prevents Memory Leaks)
void destroyTree(BSTNode* node) {
    if (!node) return;
    destroyTree(node->left);
    destroyTree(node->right);
    delete node; // Deletes parent only after children are deallocated
}
```

---

## 5. Engineering Impediments Faced & Resolutions

1. **Shared Cell Deletion:** Overlapping letters erased during backtracking.
   - *Resolution:* Engineered `uses[15][15]` reference matrix to safely delete cells only when usage drops to 0.
2. **Parallel Flank Word Collisions:** Unrelated words placing adjacent side-by-side.
   - *Resolution:* Added strict flank isolation in `grid.canPlace()`.
3. **Disconnected Island Crosswords:** Words forming 2 or 3 independent floating groups.
   - *Resolution:* Modeled words as graph nodes and used BFS traversal with `std::queue` to guarantee full connectivity.
4. **Terminal Output Duplication:** ANSI escape codes failing to wipe Windows console buffers.
   - *Resolution:* Implemented native Win32 Console API calls (`FillConsoleOutputCharacter`, `SetConsoleCursorPosition`) with `std::system("cls")` fallback.

---

## 6. Viva Voce Examination Defense Summary

- **Why Adjacency List over Matrix?** Sparse graph ($V \approx 8, E \le 12$). Saves space ($O(V + E)$ vs $O(V^2)$) and speeds up neighbor lookups.
- **Why Max-Heap over Linear Array?** $O(\log K)$ priority extraction versus $O(K \log K)$ sorting of all 225 coordinates.
- **Why Reverse Inorder?** Standard Inorder yields ascending order. Traversing Right subtree first yields descending high scores in $O(M)$ time.
- **Why Postorder for Deletion?** Bottom-up traversal ensures child pointers are freed before the parent node is deleted, preventing dangling pointers and memory leaks.

