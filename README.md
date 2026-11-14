# Crossword Puzzle CLI (DSA Course Project)

A terminal-based Crossword Puzzle game designed and engineered using fundamental Data Structures and Algorithms (covering Topics from Weeks 1 to 13 of the CT159 DSA curriculum).

---

## 📌 Project Overview
This project is an interactive Command-Line Interface (CLI) Crossword Puzzle game built in C++17. The program dynamically loads dictionary entries, places words onto a crossword grid using recursive backtracking, validates crossword topology, and offers interactive gameplay with hints, score tracking, and leaderboard management.

---

## 🎯 Target DSA Concepts (Weeks 1 – 13)

| Week | Concept Covered | Application in Crossword Project |
|---|---|---|
| **Week 1** | ADTs, Linear & Binary Search | Dictionary lookup, binary search for clue validation |
| **Week 2-3** | Singly, Doubly & Circular Linked Lists | Bucket collision resolution in Hash Table, dynamic placement histories |
| **Week 4** | Recursion & Stacks | Backtracking crossword generator; Undo/Redo stack for word placements |
| **Week 5** | Queues & Circular Queues | BFS queue for grid connectivity analysis; hint queue |
| **Week 6-7** | Sorting (Bubble, Insertion, Selection, Merge, Quick) | Sorting words by length/frequency for placement priority |
| **Week 9** | Binary Tree Traversals | Tree representations for clue hierarchy |
| **Week 10** | Binary Heap, Priority Queue & Heap Sort | Max-Heap to prioritize crossword grid placements by intersection score |
| **Week 11-12**| Binary Search Tree (BST) / AVL Tree | Leaderboard storage (inorder traversal yields ranked scores) |
| **Week 13** | Graph Representation & Traversals (BFS/DFS) | Word intersection graph; BFS/DFS connectivity check |

---

## 🏛️ System Architecture

### Architectural Diagram
```
+-------------------------------------------------------------+
|                       CLI Engine / Game Loop                |
|           (User Input, Hint Commands, Score Tracker)        |
+------------------------------+------------------------------+
                               |
       +-----------------------+-----------------------+
       |                                               |
+------v----------------------+             +----------v----------+
|  Crossword Generator        |             |  Crossword Grid     |
|  - Recursive Backtracking   |             |  - 2D Grid Cells    |
|  - Max-Heap for Placements  |             |  - Uses Counter     |
|  - Stack for Unchoose/Undo  |             |  - Clue Numbering   |
+--------------+--------------+             +----------+----------+
               |                                       |
+--------------v--------------+             +----------v----------+
|  Word Dictionary & Hash     |             |  Word Graph         |
|  - Custom Hash Table        |             |  - Intersection Adj |
|  - Word/Clue pairs loader   |             |  - BFS Connectivity |
+-----------------------------+             +----------+----------+
                                                       |
                                            +----------v----------+
                                            |  Leaderboard BST    |
                                            |  - Ranked Scores    |
                                            +---------------------+
```

---

## 🛠️ Build & Run Instructions

### Prerequisites
- Modern C++17 compiler (`g++` or MSVC via Visual Studio 2022)

### Compilation (GCC)
```bash
g++ -std=c++17 -Wall -Wextra src/main.cpp -o Crossword.exe
./Crossword.exe
```

---

## 👥 Team
- **Person A**: Data structures (Hash Table, Max-Heap, Generator, Leaderboard BST)
- **Person B**: Game loop, Grid logic, Stack/Queue, Graph verification
