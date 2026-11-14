# Crossword Puzzle DSA Project: Viva & Architecture Guide
**Course:** CT159 Data Structures, Algorithms & Applications (NEDUET)  
**Topics Covered:** Weeks 1 – 13

---

## 1. DSA Architecture & Big-O Complexity Summary

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

## 2. Core Modules Line-by-Line Breakdown

### A. The Grid Reference Counter (`uses[r][c]` in `grid.h`)
- **The Problem:** In a crossword, two words share an intersecting cell (e.g., `DOOR` horizontally and `RIVER` vertically share letter `'R'`). If `DOOR` is unplaced during backtracking, simply setting the cell to `' '` would accidentally erase the `'R'` of `RIVER`.
- **The Solution:** We maintain a parallel integer matrix `uses[GRID_SIZE][GRID_SIZE]`.
  - When placing a word: `uses[r][c]++`
  - When backtracking/removing a word: `uses[r][c]--`
  - The cell character is only wiped back to empty `' '` if `uses[r][c] == 0`.

### B. Recursive Backtracking Generator (`generator.h`)
- Places the first anchor word in the middle horizontally.
- For every subsequent word, scans all coordinates `(r, c)` and orientations (`ACROSS`, `DOWN`).
- Valid placements are evaluated using `canPlace()` (checks boundary limits, parallel flanks, and intersection letters).
- Evaluated candidates are pushed into a **Max-Heap** prioritized by number of intersections (`crossings`).
- The placement with the highest crossings is popped from the Max-Heap, committed to the grid, and pushed onto an **Undo Stack**.

### C. Graph BFS Connectivity (`graph.h`)
- **Why it matters:** A crossword is invalid if words exist in disconnected, floating clusters.
- **Algorithm:**
  1. Each word is a node (vertex $0 \dots V-1$).
  2. If word $i$ and word $j$ share a row and column intersection, an undirected edge $(i, j)$ is added to the Adjacency List.
  3. BFS runs from vertex 0 using `std::queue`. If the number of visited nodes equals $V$, the entire crossword is connected.

### D. BST Leaderboard (`bst.h`)
- Custom binary search tree node structure with `left`, `right`, and `data`.
- Left subtree contains scores smaller than root; Right subtree contains scores greater than or equal to root.
- **Reverse Inorder Traversal** (`Right -> Root -> Left`) prints the leaderboard from highest score to lowest score without needing a separate sorting step.
- **Postorder Traversal** (`Left -> Right -> Root`) ensures child nodes are deleted before parent nodes when destructing the tree.

---

## 3. High-Frequency Viva Questions & Answers

**Q1: Why use a Max-Heap instead of a simple sorted array in the generator?**
> *Answer:* When exploring candidate coordinates across a 15x15 grid, hundreds of potential placements are evaluated. Inserting into a heap takes $O(\log K)$ time, and extracting the optimal placement takes $O(\log K)$, compared to $O(K \log K)$ to sort the whole array or $O(K)$ linear scans.

**Q2: What is the worst-case time complexity of BST insertion, and how can it be prevented?**
> *Answer:* If scores arrive in strictly ascending order (e.g., 100, 200, 300), the BST degrades into a skewed linked list with worst-case height $O(M)$ and insertion time $O(M)$. To guarantee $O(\log M)$ balance in production, an AVL tree or Red-Black self-balancing tree is used.

**Q3: How does your graph representation detect if a puzzle has disconnected islands?**
> *Answer:* We use an Adjacency List `std::vector<std::vector<int>> adjList`. We start BFS from node 0 using a FIFO queue. If any word is disconnected, BFS completes with `visitedCount < numVertices`, indicating the graph is disconnected and prompting the generator to retry.

**Q4: Why is postorder traversal required for tree memory cleanup?**
> *Answer:* In C++, freeing a parent node before deleting its children results in dangling pointers or lost memory references (memory leak). Postorder visits `Left`, then `Right`, and finally `delete node`, guaranteeing that subtrees are safely cleaned first.
