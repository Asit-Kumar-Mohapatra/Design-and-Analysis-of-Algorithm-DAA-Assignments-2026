# Q1 — Dictionary Operations: Asymptotic Analysis Across Data Structures

## Problem Statement

Implement and empirically benchmark **seven fundamental dictionary operations** — **Search, Insert, Delete, Max, Min, Predecessor, and Successor** — across **six different data structures**:

| # | Data Structure         | Ordering  | Link Type |
|---|------------------------|-----------|-----------|
| 1 | Unsorted Array         | Unsorted  | Array     |
| 2 | Sorted Array           | Sorted    | Array     |
| 3 | Singly Linked List     | Unsorted  | Singly    |
| 4 | Singly Linked List     | Sorted    | Singly    |
| 5 | Doubly Linked List     | Unsorted  | Doubly    |
| 6 | Doubly Linked List     | Sorted    | Doubly    |

The goal is to validate theoretical **Big-O worst-case complexities** against measured wall-clock timing data across input sizes `N = 2000` to `N = 20000` (step 2000), averaged over **1000 repetitions** per configuration.

---

## Theoretical Complexity Table

| Data Structure   | Search     | Insert | Delete | Max    | Min    | Predecessor | Successor | Space  |
|------------------|------------|--------|--------|--------|--------|-------------|-----------|--------|
| Unsorted Array   | O(n)       | O(1)   | O(1)   | O(n)   | O(n)   | O(n)        | O(n)      | O(n)   |
| Sorted Array     | O(log n)   | O(n)   | O(n)   | O(1)   | O(1)   | O(1)        | O(1)      | O(n)   |
| Singly Unsorted  | O(n)       | O(1)   | O(n)   | O(n)   | O(n)   | O(n)        | O(n)      | O(n)   |
| Singly Sorted    | O(n)       | O(n)   | O(n)   | O(1)*  | O(1)   | O(n)        | O(1)      | O(n)   |
| Doubly Unsorted  | O(n)       | O(1)   | O(1)   | O(n)   | O(n)   | O(n)        | O(n)      | O(n)   |
| Doubly Sorted    | O(n)       | O(n)   | O(1)   | O(1)   | O(1)   | O(1)        | O(1)      | O(n)   |

> * Singly Sorted Max is O(1) only when a **tail pointer** is maintained; otherwise O(n).

---

## Key Algorithmic Insights

### Search
- **Unsorted structures** (Array, Singly/Doubly Linked): require a full linear scan — O(n)
- **Sorted Linked Lists**: still O(n) — sorted order cannot enable binary search because linked structures lack O(1) random access
- **Sorted Array**: achieves **O(log n)** via binary search (contiguous memory enables O(1) midpoint calculation)

### Insert
- **Unsorted Array & Unsorted Linked Lists**: O(1) — append to end or prepend to head
- **Sorted Array**: O(n) — inserting a new minimum forces every element to shift right
- **Sorted Linked Lists**: O(n) — must traverse to find the correct insertion position

### Delete (given a pointer/index)
- **Unsorted Array**: O(1) — swap-with-last trick (arr[idx] = arr[--size])
- **Doubly Linked Lists** (both variants): O(1) — node->prev and node->next allow immediate pointer bypass
- **Sorted Array**: O(n) — all subsequent elements must shift left
- **Singly Linked Lists**: O(n) — lack of a prev pointer forces a linear scan from head to find the predecessor node

### Max / Min
- **Unsorted structures**: O(n) full scan required
- **Sorted structures**: O(1) — Min at head/index 0, Max at tail/index n-1

### Predecessor / Successor
- **Unsorted structures**: O(n) scan to find the largest element smaller (predecessor) or smallest element larger (successor) than the target
- **Sorted Array**: O(1) via arr[idx-1] and arr[idx+1]
- **Doubly Sorted**: O(1) via node->prev and node->next
- **Singly Sorted**: Successor is O(1) (node->next), but Predecessor is O(n) (no backward pointer)

---

## File Descriptions

### `q1_dictionary_operations.c` — Main Benchmark Program
**Language:** C | **Lines:** 408

This is the primary implementation file. It contains:

- **Data structure definitions** (typedef struct) for all 6 structures:
  - `ArrayDS` — backing array with size and capacity
  - `SNode` / `SinglyUnsorted` / `SinglySorted` — singly linked node and list wrappers
  - `DNode` / `DoublyUnsorted` / `DoublySorted` — doubly linked node and list wrappers

- **42 operation implementations** (7 operations x 6 structures), using the naming convention `<DS_Prefix>_<Operation>`:
  - `UA_*` → Unsorted Array
  - `SA_*` → Sorted Array
  - `SU_*` → Singly Unsorted
  - `SS_*` → Singly Sorted
  - `DU_*` → Doubly Unsorted
  - `DS_*` → Doubly Sorted

- **`print_complexities()`** — prints the theoretical worst-case complexity table to the console

- **`print_conclusion()`** — prints a detailed 7-point analysis covering:
  - Why binary search is impossible on linked lists
  - The swap-with-last deletion trick for unsorted arrays
  - Cache locality advantages of contiguous arrays
  - How empirical data validates Big-O theory

- **`main()` benchmark driver** — iterates over `N = 2000..20000` in steps of 2000. For each N:
  1. Populates all 6 data structures with N pre-sorted even integers
  2. Uses the `BENCHMARK` macro to time each of the 42 operations averaged over 1000 repetitions
  3. Forces worst-case scenarios:
     - Search: key `-1` (guaranteed absent)
     - Delete (Singly Unsorted): re-inserts at **tail** via `SU_InsertTail()` to keep the target node at the far end, exposing the true O(n) delete cost
     - Insert (Sorted Array): inserts a new minimum to force maximum element shifts
  4. Writes all results to `dictionary_operations.csv`

**How to compile and run:**
```bash
gcc -O2 -o q1_dictionary_operations q1_dictionary_operations.c
./q1_dictionary_operations
```

---

### `q1_dictionary_operations_graphs.py` — Graph Generation Script
**Language:** Python 3 | **Lines:** 58
**Dependencies:** `pandas`, `matplotlib`

Reads `dictionary_operations.csv` and generates **7 separate PNG graphs**, one per operation. Each graph plots execution time (seconds) vs. input size N for all 6 data structures simultaneously, using distinct colors and markers:

| Data Structure  | Color          | Marker |
|-----------------|----------------|--------|
| Unsorted Array  | Red (#e74c3c)  | o      |
| Sorted Array    | Blue (#3498db) | s      |
| Singly Unsorted | Green (#2ecc71)| ^      |
| Singly Sorted   | Orange (#f39c12)| d     |
| Doubly Unsorted | Purple (#9b59b6)| v     |
| Doubly Sorted   | Teal (#1abc9c) | P      |

**How to run:**
```bash
python q1_dictionary_operations_graphs.py
```
> Must be run from the Q1/ directory, or ensure `dictionary_operations.csv` is in the working directory.

---

### `dictionary_operations.csv` — Benchmark Data
**Format:** CSV with 4 columns | **Rows:** 420 (10 values of N x 6 structures x 7 operations)

| Column      | Type   | Description                                               |
|-------------|--------|-----------------------------------------------------------|
| `N`         | int    | Number of elements in the data structure (2000 to 20000)  |
| `Structure` | string | One of the 6 data structure names                         |
| `Operation` | string | Search / Insert / Delete / Max / Min / Predecessor / Successor |
| `Time`      | float  | Average wall-clock time per call in seconds (over 1000 reps) |

**Sample rows:**
```
N,Structure,Operation,Time
2000,Unsorted Array,Search,0.00000400
2000,Sorted Array,Search,0.00000000
2000,Singly Unsorted,Delete,0.00001200
```

---

### Graph Files (7 PNG) — Visual Validation

All 7 graphs are saved in the `Q1/` directory. Each graph title follows the pattern:
`Asymptotic Growth Analysis: <Operation>() Operation`

| File                    | Operation    | Expected Visual Shape                                                 |
|-------------------------|--------------|-----------------------------------------------------------------------|
| `graph_search.png`      | Search       | Sorted Array hugs near-zero (O(log n)); all others grow linearly (O(n)) |
| `graph_insert.png`      | Insert       | Sorted structures grow linearly (O(n)); unsorted structures are flat (O(1)) |
| `graph_delete.png`      | Delete       | Unsorted Array and Doubly Linked flat (O(1)); others grow linearly (O(n)) |
| `graph_max.png`         | Max          | Sorted structures flat (O(1)); unsorted structures grow linearly (O(n)) |
| `graph_min.png`         | Min          | Sorted structures flat (O(1)); unsorted structures grow linearly (O(n)) |
| `graph_predecessor.png` | Predecessor  | Sorted Array and Doubly Sorted flat (O(1)); others linear (O(n))      |
| `graph_successor.png`   | Successor    | Sorted Array, Singly Sorted (tail ptr) and Doubly Sorted flat (O(1)); others linear |

---

## How to Reproduce All Results

```bash
# Step 1: Compile and run the C benchmark
gcc -O2 -o q1_dictionary_operations q1_dictionary_operations.c
./q1_dictionary_operations
# Output: dictionary_operations.csv

# Step 2: Generate the 7 graphs
python q1_dictionary_operations_graphs.py
# Output: graph_search.png, graph_insert.png, graph_delete.png,
#         graph_max.png, graph_min.png, graph_predecessor.png, graph_successor.png
```

---

## Directory Structure

```
Q1/
├── q1_dictionary_operations.c          # C source: 42 operations + benchmark driver
├── q1_dictionary_operations_graphs.py  # Python: reads CSV, outputs 7 graphs
├── dictionary_operations.csv           # Benchmark data (420 rows)
├── graph_search.png                    # Graph: Search operation
├── graph_insert.png                    # Graph: Insert operation
├── graph_delete.png                    # Graph: Delete operation
├── graph_max.png                       # Graph: Max operation
├── graph_min.png                       # Graph: Min operation
├── graph_predecessor.png               # Graph: Predecessor operation
├── graph_successor.png                 # Graph: Successor operation
└── README.md                           # This file
```
