# Crossword Puzzle DSA Project: Comprehensive Architecture & Viva Guide
**Course:** CT159 Data Structures, Algorithms & Applications (NEDUET)  
**Topics Covered:** Weeks 1 – 13  
**Repository:** [muhammadasharhussain/Crossword-Puzzle](https://github.com/muhammadasharhussain/Crossword-Puzzle)

---

## Table of Contents
1. [Executive Summary & High-Level Architecture](#1-executive-summary--high-level-architecture)
2. [Data Structure Curriculum Mapping (Weeks 1 to 13)](#2-data-structure-curriculum-mapping-weeks-1-to-13)
3. [File-by-File Detailed Code & Variable Walkthrough](#3-file-by-file-detailed-code--variable-walkthrough)
   - [src/types.h](#file-1-srctypesh--data-types--models)
   - [src/dictionary.h](#file-2-srcdictionaryh--file-loading--hash-indexing)
   - [src/grid.h](#file-3-srcgridh--2d-matrix-adts--reference-counting)
   - [src/generator.h](#file-4-srcgeneratorh--backtracking-generator--max-heap)
   - [src/graph.h](#file-5-srcgraphh--graph-representation--bfs-connectivity)
   - [src/bst.h](#file-6-srcbsth--binary-search-tree-leaderboard)
   - [src/game.h](#file-7-srcgameh--cli-engine-ansi-coloring--hint-system)
   - [src/main.cpp](#file-8-srcmaincpp--windows-virtual-terminal--bootstrap)
4. [Engineering Challenges, Impediments & Their Resolutions](#4-engineering-challenges-impediments--their-resolutions)
5. [Complete Viva Voce Q&A Cheat Sheet](#5-complete-viva-voce-qa-cheat-sheet)

---

## 1. Executive Summary & High-Level Architecture

The CLI Crossword Puzzle game is an interactive terminal application engineered in **C++17**. It automatically loads word-clue pairs from disk, dynamically synthesizes a valid crossword layout using **backtracking and priority queues**, models word intersections as an **undirected graph** to guarantee connectivity via **Breadth-First Search (BFS)**, and records player high scores in a custom **Binary Search Tree (BST)** rendered through **reverse-inorder traversal**.

### Architectural Block Diagram

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
|       - Orchestrates Dictionary, Grid, Generator, Graph, and BST Leaderboard            |
|       - Interactive Game Loop: input parser, timer, hints (random & targeted), score    |
+----+----------------------+-----------------------+----------------------+--------------+
     |                      |                       |                      |
+----v------------+  +------v-------------+  +------v------------+  +------v--------------+
|  Dictionary     |  |   CrosswordGrid    |  | CrosswordGenerator|  | CrosswordGraph      |
| (dictionary.h)  |  |    (src/grid.h)    |  | (src/generator.h) |  |   (src/graph.h)     |
| - Loads file    |  | - 15x15 char board |  | - Backtracking    |  | - Adjacency list    |
| - Character     |  | - uses[r][c] ref   |  | - Max-Heap order  |  | - BFS queue         |
|   index hash-map|  |   counter          |  | - Placement stack |  | - Verifies 1 island |
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

## 2. Data Structure Curriculum Mapping (Weeks 1 to 13)

| Week | Data Structure / Algorithm | Used In | Why Selected? | Time Complexity | Space Complexity |
|---|---|---|---|---|---|
| **Week 1** | 2D Arrays / Matrices | `grid.h` (`CrosswordGrid`) | Direct coordinate random access to board cells `(r, c)` | Access: $O(1)$ | $O(N^2)$ where $N=15$ |
| **Week 2-3** | Linked Nodes & Dynamic Pointers | `bst.h` (`BSTNode`) | Dynamic node allocation for tree-based score ranking | Node creation: $O(1)$ | $O(M)$ nodes |
| **Week 4** | Recursion & Stacks | `generator.h` (`std::stack`) | Backtracking algorithm to place words and undo invalid placement choices | Push/Pop: $O(1)$ | $O(W)$ where $W$ is words placed |
| **Week 5** | Queues (FIFO) | `graph.h` (`std::queue`) | Level-order BFS traversal to verify puzzle connectivity | Enqueue/Dequeue: $O(1)$ | $O(V)$ where $V$ is words |
| **Week 6-7** | Sorting (Quick/Merge Sort) | `generator.h` (`std::sort`) | Orders placed words top-to-bottom, left-to-right for standard clue numbering | $O(V \log V)$ | $O(\log V)$ auxiliary |
| **Week 9** | Tree Traversals | `bst.h` (`reverseInorder`, `destroyTree`) | Reverse-inorder yields scores descending; postorder safely deallocates memory | $O(M)$ to traverse all nodes | $O(H)$ recursion stack ($H$ = height) |
| **Week 10** | Binary Heap / Priority Queue | `generator.h` (`std::priority_queue`) | Max-Heap ranks candidate word placements to maximize letter intersections | Insert: $O(\log K)$, Extract-Max: $O(\log K)$ | $O(K)$ candidate placements |
| **Week 11-12** | Binary Search Tree (BST) | `bst.h` (`LeaderboardBST`) | Stores and ranks player scores dynamically without re-sorting an entire array | Insert: $O(H)$, Average $O(\log M)$, Worst $O(M)$ | $O(M)$ |
| **Week 13** | Graph (Adjacency List) & BFS | `graph.h` (`CrosswordGraph`) | Words are vertices; shared crossing cells are edges. BFS guarantees a single connected crossword | Graph Build: $O(V^2)$, BFS: $O(V + E)$ | $O(V + E)$ |

---

## 3. File-by-File Detailed Code & Variable Walkthrough

---

### File 1: `src/types.h` — Data Types & Models

Defines the fundamental structures used across the application.

```cpp
const int GRID_SIZE = 15;
```
* **Explanation:** Constant setting the board dimensions to $15 \times 15$ cells. Standard crosswords use 15x15 grids because it fits 3 to 10 letter words comfortably while rendering within standard 80-column command prompt windows.

```cpp
enum Direction {
    ACROSS = 0,
    DOWN = 1
};
```
* **Explanation:** Enumeration representing orientation. `ACROSS` moves along row $r$ from left to right (column increments). `DOWN` moves along column $c$ from top to bottom (row increments).

```cpp
struct WordEntry {
    std::string word;
    std::string clue;
};
```
* **Explanation:** Simple dictionary container mapping an uppercase answer word to its clue sentence.

```cpp
struct PlacedWord {
    int clueNumber;
    std::string word;
    std::string clue;
    int row;
    int col;
    Direction dir;
    bool isSolved;
};
```
* **Explanation:** Represents a word anchored onto the board.
  * `clueNumber`: The numeric index displayed to the user (e.g. `1` in `1A`).
  * `row, col`: Starting coordinate of the word's first letter.
  * `dir`: Direction enum (`ACROSS` or `DOWN`).
  * `isSolved`: Flag flipped to `true` when the player answers correctly or hints reveal all letters.

```cpp
struct CandidatePlacement {
    int row;
    int col;
    Direction dir;
    int crossings;

    bool operator<(const CandidatePlacement& other) const {
        return crossings < other.crossings;
    }
};
```
* **DSA Concept (Week 10 - Heap comparator):** In C++, `std::priority_queue` is a **Max-Heap**. To rank candidate positions, `operator<` defines which placement has lower priority. By comparing `crossings < other.crossings`, placements that intersect the highest number of existing letters float to the top of the heap.

```cpp
struct LeaderboardEntry {
    std::string playerName;
    int score;
    int timeTakenSeconds;
};
```
* **Explanation:** The payload stored inside each node of the Leaderboard Binary Search Tree.

---

### File 2: `src/dictionary.h` — File Loading & Hash Indexing

* **DSA Concepts:** Arrays, Linear scan, Hash Tables / Inverted Index.

#### Class Variables:
* `std::vector<WordEntry> words`: Dynamic array storing loaded word-clue objects.
* `std::unordered_map<char, std::vector<int>> letterIndexMap`: **Inverted Index Hash Table**.
  * **Key:** A letter character (e.g., `'E'`).
  * **Value:** A list of array indices in `words` that contain that character.
  * **Efficiency:** When the generator wants to cross an existing letter on the board, it looks up candidates in average $O(1)$ time instead of scanning all dictionary entries linearly ($O(N)$).

#### Methods:
* `bool loadFromFile(const std::string& filepath)`:
  1. Opens `data/dictionary.txt` using `std::ifstream`.
  2. Strips Windows carriage return characters (`\r`).
  3. Parses each line using `std::stringstream` with `,` as delimiter.
  4. Capitalizes the word using `std::transform(..., ::toupper)`.
  5. Populates both `words` vector and `letterIndexMap`.
* `std::vector<WordEntry> getRandomSubset(size_t count, unsigned int seed)`:
  Shuffles a copy of dictionary words using Fisher-Yates (`std::shuffle`) with a pseudorandom Mersenne Twister engine (`std::mt19937`).

---

### File 3: `src/grid.h` — 2D Matrix ADTs & Reference Counting

* **DSA Concepts:** Week 1 2D Matrix operations, cell validation.

#### Class Variables:
* `char board[15][15]`: 2D character array storing the solution grid (`' '` for empty, `'A'`-`'Z'` for letters).
* `int uses[15][15]`: **The Reference Counting Matrix** (tracks how many words share each cell).
* `bool revealed[15][15]`: 2D boolean mask indicating which cells the player has discovered.

#### Methods:
* `void clear()`: Re-initializes all three matrices to blank / zero states.
* `char getChar(r, c)`, `bool isRevealed(r, c)`, `void setRevealed(r, c, val)`: Direct $O(1)$ matrix accessor and mutator methods with boundary checks.
* `bool canPlace(word, row, col, dir, isFirstWord)`:
  Validates 5 essential crossword rules:
  1. **Boundary check:** The word must fit inside the 15x15 boundaries without overflowing.
  2. **Start flank clearance:** The cell immediately preceding the first letter must be empty or off-grid.
  3. **End flank clearance:** The cell immediately succeeding the last letter must be empty or off-grid.
  4. **Cell matching & lateral isolation:**
     - If a cell is occupied, it **must** match `word[i]`.
     - If a cell is unoccupied, its perpendicular side neighbors must be empty (prevents parallel words from sticking side-by-side).
  5. **Connectivity:** Non-initial words must intersect at least one already-placed letter (`crossings > 0`).
* `void place(word, row, col, dir)`:
  Copies letters onto `board[r][c]` and increments `uses[r][c]++`.
* `void remove(word, row, col, dir)`:
  Backtracks a placement by decrementing `uses[r][c]--`. The cell character is only wiped to `' '` when `uses[r][c]` reaches 0.

---

### File 4: `src/generator.h` — Backtracking Generator & Max-Heap

* **DSA Concepts:** Week 4 Stacks / Backtracking, Week 10 Priority Queue, Week 6-7 Sorting.

#### Generator Pipeline (`CrosswordGenerator::generate`):
1. **Shuffles** dictionary words to ensure every play session generates a new puzzle.
2. **Anchor Placement:** Word 0 is placed horizontally in the center of the board ($row = 7$) with zero crossing restrictions.
3. **Candidate Evaluation:**
   For each remaining word, scans all $15 \times 15$ coordinates and both directions.
   If legal, creates a `CandidatePlacement` with its calculated crossings and inserts it into a `std::priority_queue<CandidatePlacement>` (**Max-Heap**).
4. **Greedy Selection:**
   Pops `maxHeap.top()` to select the candidate coordinate that maximizes crossword crossings. Commits the word to the grid and pushes its state onto an undo stack.
5. **Clue Numbering (`assignClueNumbers`):**
   Sorts all placed words using `std::sort` by row ascending, then column ascending.
   Iterates through sorted words: if an `ACROSS` word and `DOWN` word start at the same coordinate `(row, col)`, they share the same clue number.

---

### File 5: `src/graph.h` — Graph Representation & BFS Connectivity

* **DSA Concepts:** Week 13 Graph Adjacency List, Week 5 Queue ADT, Week 13 Breadth-First Search (BFS).

#### Class Variables:
* `int numVertices`: Total number of placed words in the puzzle.
* `std::vector<std::vector<int>> adjList`: Adjacency List representing intersections between words.

#### Methods:
* `static CrosswordGraph buildFromPlacements(placements)`:
  Pairs all words ($O(V^2)$) and runs `doIntersect(a, b)`. If an across word and a down word cross coordinate ranges, an undirected edge $(u, v)$ is registered:
  ```cpp
  adjList[u].push_back(v);
  adjList[v].push_back(u);
  ```
* `bool isConnected()`:
  Executes **Breadth-First Search (BFS)** using a `std::queue<int>`:
  1. Enqueues node 0 and marks `visited[0] = true`.
  2. While queue is non-empty, dequeues front node and explores all adjacent neighbors from `adjList[u]`.
  3. Increments `visitedCount`.
  4. Returns `visitedCount == numVertices`. If any word was left floating in isolation, BFS returns `false`, prompting the generator to retry.

---

### File 6: `src/bst.h` — Binary Search Tree Leaderboard

* **DSA Concepts:** Week 2-3 Dynamic Linked Pointers, Week 11-12 BST Insert, Week 9 Tree Traversals.

```cpp
struct BSTNode {
    LeaderboardEntry data;
    BSTNode* left;
    BSTNode* right;
};
```

#### BST Property:
- `left` child contains scores strictly less than `node->data.score`.
- `right` child contains scores greater than or equal to `node->data.score`.

#### Methods:
* `BSTNode* insertHelper(node, entry)`:
  Standard recursive BST insertion. If the tree is empty, allocates a new `BSTNode`. Traverses left or right based on score. Average insertion time is $O(\log M)$.
* `void reverseInorder(node, rank, maxEntries)`:
  Standard Inorder visits `Left -> Root -> Right` (ascending order).
  **Reverse Inorder** visits `Right -> Root -> Left`. This visits the highest scores first, printing a ranked leaderboard without requiring a secondary sorting pass.
* `void destroyTree(node)`:
  Recursively visits `destroyTree(left)`, then `destroyTree(right)`, then `delete node` (**Postorder Traversal**). This prevents memory leaks and dangling pointers by deleting child nodes before parent nodes.
* `void saveToFile()` & `void loadFromFile()`:
  Persists the leaderboard entries to disk (`data/leaderboard.txt`).

---

### File 7: `src/game.h` — CLI Engine, ANSI Coloring & Hint System

* **Features & Mechanics:**
  * **Color Namespace:** Definitions for ANSI escape sequences (`\033[32;1m` bold green, `\033[33;1m` bold yellow, `\033[36m` cyan).
  * **Clean Grid Renderer (`displayBoard`):** Replaces unused board coordinates with clean spaces (`"   "`), showing yellow clue starting numbers and cyan dashes (` - `) for unfilled puzzle slots.
  * **Difficulty Selector:** Lets the user pick:
    - 1: Easy (4 - 5 words)
    - 2: Medium (6 - 8 words)
    - 3: Hard (9 - 12 words)
  * **Dual Hint Subsystem (`provideHint(targetClue)`):**
    - `hint`: Randomly uncovers any unrevealed cell on the board (-15 pts).
    - `hint <ClueCode>`: Specifically selects an unrevealed letter belonging to that clue (e.g. `hint 1A`, -20 pts).
  * **Auto-Solve Detection (`checkAndUpdateFullyRevealedWords`):** When hints uncover all letters of a word, it automatically marks the word as solved.
  * **Score Algorithm:** $100 \text{ pts per correct word} - \text{hint penalties} + \max(0, 200 - \text{seconds} / 3)$.

---

### File 8: `src/main.cpp` — Windows Virtual Terminal & Bootstrap

* **Purpose:** Sets the Windows console mode flag `ENABLE_VIRTUAL_TERMINAL_PROCESSING` inside `#ifdef _WIN32` so legacy command prompts process ANSI colors properly.
* Initializes `CrosswordGame` with `data/dictionary.txt` and invokes `game.run()`.

---

## 4. Engineering Challenges, Impediments & Their Resolutions

### Impediment 1: Shared Cell Deletion during Backtracking
* **Problem:** In a crossword, intersecting words share letters (e.g., `DOOR` across and `RIVER` down share `'R'`). In simple grid implementations, removing `DOOR` would set all its cells to `' '`, inadvertently destroying the `'R'` in `RIVER`.
* **Resolution:** Implemented `uses[15][15]`. `place()` increments cell usage; `remove()` decrements usage. The character is only reset to `' '` when `uses[r][c] == 0`.

### Impediment 2: Parallel Flank Collisions
* **Problem:** Candidate words were initially placed right against other words (e.g. `CAT` on row 1, `DOG` on row 2 directly underneath), creating illegible letter clumps.
* **Resolution:** Added perpendicular flank isolation checks in `grid.canPlace()`. Any empty cell being filled by a new word must have empty side neighbors unless it is an intentional intersection.

### Impediment 3: Disconnected Puzzle Islands
* **Problem:** The generator placed words validly according to local rules, but generated 2 or 3 independent clusters on the board with no connection between them.
* **Resolution:** Modeled the crossword as an undirected graph (words = vertices, crossings = edges). Ran BFS using a queue. If `visitedCount < totalWords`, the graph has disconnected components and is rejected.

### Impediment 4: Clue Renumbering Discrepancies
* **Problem:** Words placed in random order initially received chronological clue numbers (e.g. Clue 1 on row 8, Clue 2 on row 1), and overlapping start positions received conflicting numbers.
* **Resolution:** Introduced `assignClueNumbers()` which sorts placed words top-to-bottom and left-to-right. Words sharing the same `(row, col)` start cell share the same clue number (e.g., `1A` and `1D`).

---

## 5. Complete Viva Voce Q&A Cheat Sheet

**Q1: Why did you choose an Adjacency List over an Adjacency Matrix for the word graph?**
> *Answer:* Crossword puzzles are sparse graphs. With $V \approx 8$ words and $E \le 12$ crossings, an adjacency matrix would allocate $V^2 = 64$ entries (mostly empty zeroes). An adjacency list consumes only $O(V + E)$ memory and allows BFS to inspect neighbors in $O(\text{deg}(v))$ time rather than scanning an entire matrix row in $O(V)$ time.

**Q2: What is the time and space complexity of BFS on your graph?**
> *Answer:* Time complexity is $O(V + E)$ where $V$ is the number of placed words and $E$ is the number of crossings. Space complexity is $O(V)$ to store the `visited` array and the BFS queue.

**Q3: Why is a Max-Heap used in the crossword generator?**
> *Answer:* For every word, hundreds of candidate coordinates and orientations are evaluated. Storing candidates in a Max-Heap allows $O(\log K)$ insertion and $O(\log K)$ extraction of the placement with the highest letter crossings, ensuring the generator greedily creates dense, well-interlocked puzzles.

**Q4: How does Reverse Inorder Traversal in a BST work?**
> *Answer:* Standard inorder traversal visits `Left -> Root -> Right`, outputting data in ascending order. Reverse inorder traverses `Right -> Root -> Left`. Because the right subtree holds higher values, it visits highest scores first, generating a descending leaderboard without needing a separate sorting algorithm.

**Q5: Why is Postorder Traversal necessary when destructing a BST?**
> *Answer:* In C++, deallocating a parent node before its children results in lost pointers and memory leaks. Postorder traversal (`Left -> Right -> Root`) ensures both child subtrees are completely freed before the parent node is deleted.

**Q6: What is the worst-case time complexity of BST insertion, and how is it mitigated in production?**
> *Answer:* If scores are inserted in already sorted order, the BST degenerates into a linear linked list with worst-case height $O(M)$ and insertion time $O(M)$. In commercial systems, an AVL Tree or Red-Black Tree uses tree rotations to maintain balanced height $O(\log M)$.

**Q7: How does your grid handle memory and coordinate lookups?**
> *Answer:* The grid is stored as a flat 2D array of static dimensions $15 \times 15$. Lookups `board[r][c]` operate in $O(1)$ constant time with zero dynamic allocation overhead.
