# Q2 — Standard 2-Way Merge Sort vs. Modified 3-Way Merge Sort

## Problem Statement

Implement and empirically compare two divide-and-conquer sorting algorithms:

- **Standard 2-Way Merge Sort** — the classic algorithm that splits an array into **two equal halves**, sorts each recursively, and merges them.
- **Modified 3-Way Merge Sort** — a variant that splits an array into **three equal thirds**, sorts each recursively, and merges all three simultaneously.

The objective is to:
1. Derive the recurrence relations for both algorithms using the **Master Theorem**
2. Determine whether splitting into three parts reduces the overall asymptotic complexity
3. Validate the theoretical bounds against empirical timing data for `N = 10,000` to `N = 100,000`

---

## Theoretical Analysis

### 2-Way Merge Sort

The array is split into two halves. Each half is sorted recursively, then merged in linear time.

**Recurrence relation:**
```
T(n) = 2T(n/2) + Theta(n)
```

**Master Theorem application:**
- a = 2, b = 2, f(n) = Theta(n)
- Critical exponent: n^(log_b(a)) = n^(log_2(2)) = n^1 = Theta(n)
- f(n) matches n^(log_b(a)) exactly → **Case 2** of the Master Theorem

**Solution:**
```
T(n) = Theta(n * log_2(n))
```

**Recursion tree depth:** `log_2(n)` levels

---

### Modified 3-Way Merge Sort

The array is split into three equal thirds. Each third is sorted recursively, then all three sorted sub-arrays are merged in a single three-way merge pass.

**Recurrence relation:**
```
T(n) = 3T(n/3) + Theta(n)
```

**Master Theorem application:**
- a = 3, b = 3, f(n) = Theta(n)
- Critical exponent: n^(log_b(a)) = n^(log_3(3)) = n^1 = Theta(n)
- f(n) matches n^(log_b(a)) exactly → **Case 2** of the Master Theorem

**Solution:**
```
T(n) = Theta(n * log_3(n)) = Theta(n * log_2(n) / log_2(3)) = Theta(n log n)
```

**Recursion tree depth:** `log_3(n) ≈ 0.631 * log_2(n)` (approximately 36.9% shallower)

---

## Complexity Comparison

| Algorithm            | Recurrence              | Tree Height | Asymptotic Bound     |
|----------------------|-------------------------|-------------|----------------------|
| Standard 2-Way Merge | T(n) = 2T(n/2) + O(n)  | log_2(n)    | Theta(n log_2 n)     |
| Modified 3-Way Merge | T(n) = 3T(n/3) + O(n)  | log_3(n)    | Theta(n log_3 n)     |

> Both are **Theta(n log n)** — the 3-way variant does NOT improve the asymptotic class.

### Why Doesn't 3-Way Win?

Although the recursion tree is ~37% shallower:

- **2-Way merge** evaluates 2 candidate stream heads per step → **1 comparison per element**
- **3-Way merge** evaluates 3 candidate stream heads per step → **up to 2 comparisons per element**

The extra comparisons inside the 3-way merge loop increase the constant factor, largely cancelling the gain from fewer recursion levels. Both algorithms ultimately converge to **Theta(n log n)**, differing only in their leading constant.

---

## File Descriptions

### `q2_merge_sort_vs_modified_merge_sort.c` — Main Benchmark Program
**Language:** C | **Lines:** 218

This file implements both sorting algorithms and benchmarks them head-to-head.

#### `merge2()` — 2-Way Merge (lines 13–25)
Classic two-pointer merge: uses indices `i` (left half) and `j` (right half) to zip both halves into a temporary buffer, then copies back. Time: O(n).

#### `merge_sort_2way()` — Recursive 2-Way Sort (lines 27–34)
Splits at midpoint `m = l + (r - l) / 2`, recurses on `[l, m]` and `[m+1, r]`, then calls `merge2()`.

#### `merge3()` — 3-Way Merge (lines 39–73)
Three-pointer merge using indices `i` (first third), `j` (middle third), `k` (last third). The merge proceeds in three phases:
1. While all three pointers are valid: pick the minimum of `arr[i]`, `arr[j]`, `arr[k]`
2. While two pointers remain valid: standard 2-way merge of the remaining pair
3. Drain any single remaining pointer

#### `merge_sort_3way()` — Recursive 3-Way Sort (lines 75–94)
- Splits using: `len = (r - l + 1) / 3`, `m1 = l + len - 1`, `m2 = l + 2*len - 1`
- **Special base case:** when only 2 elements remain, swaps directly (avoids invalid sub-range with integer division)
- Recurses on `[l, m1]`, `[m1+1, m2]`, `[m2+1, r]`, then calls `merge3()`

#### `is_sorted()` — Correctness Checker (lines 99–103)
Verifies the output array is non-decreasingly ordered. Prevents plotting broken/silently wrong sort output.

#### `print_complexities()` — Complexity Summary Table (lines 108–117)
Prints a formatted table of recurrence relations, tree heights, and asymptotic bounds to the console.

#### `print_conclusion()` — Detailed Theoretical Report (lines 119–154)
Four-section analysis covering:
1. Master Theorem derivation for 3-way merge sort
2. Recursion depth vs. merge overhead trade-off
3. CPU cache and register pressure from maintaining 3 simultaneous read pointers
4. Interpretation of the benchmark CSV and graph

#### `main()` — Benchmark Driver (lines 159–218)
- Input range: `N = 10,000` to `N = 100,000`, step 10,000
- Repetitions: **50 per N** to reduce timing noise
- Input type: **Reverse-sorted array** (worst-case input for comparison-based sorts)
- Allocates separate arrays `arr1` and `arr2` (2-way and 3-way respectively) with a shared `temp` buffer
- Verifies correctness with `is_sorted()` after each sort
- Writes `N,Algorithm,Time` rows to CSV

**How to compile and run:**
```bash
gcc -O2 -o q2_merge_sort_vs_modified_merge_sort q2_merge_sort_vs_modified_merge_sort.c -lm
./q2_merge_sort_vs_modified_merge_sort
```

---

### `merge_sort_vs_modified_merge_sort.csv` — Benchmark Data
**Format:** CSV with 3 columns | **Rows:** 21 (10 N-values x 2 algorithms + header)

| Column      | Type   | Description                                                |
|-------------|--------|------------------------------------------------------------|
| `N`         | int    | Input array size (10,000 to 100,000, step 10,000)          |
| `Algorithm` | string | `2-Way Merge Sort` or `3-Way Merge Sort`                   |
| `Time`      | float  | Average wall-clock time per sort in **seconds** (over 50 reps) |

**Sample rows:**
```
N,Algorithm,Time
10000,2-Way Merge Sort,0.00080000
10000,3-Way Merge Sort,0.00052000
100000,2-Way Merge Sort,0.00682000
100000,3-Way Merge Sort,0.00524000
```

> **Observation from the data:** The 3-Way variant consistently shows lower absolute time (~25-35% faster in practice), but both curves grow at the same log-linear rate — confirming that the difference is a constant factor, not an asymptotic improvement.

---

### `detailed_merge_sort_analysis.png` — Analysis Graph
**Format:** PNG (dual-panel, 15×6 inches, 300 DPI)

This single image contains two side-by-side plots:

| Panel | Title                                  | What to Look For                              |
|-------|----------------------------------------|-----------------------------------------------|
| Left  | Absolute Execution Time Scaling        | Both curves curving upward together (log-linear) |
| Right | Normalized Efficiency: Time / (N log₂ N) | Both lines flatten to constants (validating Theta(n log n)) |

---

## How to Reproduce All Results

```bash
# Step 1: Compile and run the C benchmark (generates CSV)
gcc -O2 -o q2_merge_sort_vs_modified_merge_sort q2_merge_sort_vs_modified_merge_sort.c -lm
./q2_merge_sort_vs_modified_merge_sort
# Output: merge_sort_vs_modified_merge_sort.csv

# Step 2: Generate the analysis graph
python q2_merge_sort_vs_modified_merge_sort.py
# Output: merge_sort_vs_modified_merge_sort_graph_analysis.png
```

---

## Conclusion

| Criterion              | 2-Way Merge Sort                  | 3-Way Merge Sort                         |
|------------------------|-----------------------------------|------------------------------------------|
| Recurrence             | T(n) = 2T(n/2) + O(n)            | T(n) = 3T(n/3) + O(n)                   |
| Asymptotic Complexity  | Theta(n log n)                    | Theta(n log n)                           |
| Recursion Depth        | log_2(n)                          | log_3(n) — ~37% shallower               |
| Comparisons per merge  | 1 per element                     | Up to 2 per element                      |
| Practical Speed        | Baseline                          | ~25-35% faster in benchmarks (constant factor only) |
| Asymptotic Winner?     | Tie                               | Tie                                      |

The 3-way split **does not improve the Big-O class** — it remains Theta(n log n). The slight empirical speed advantage observed is due to the shallower recursion tree partially compensating for the heavier per-merge comparison overhead, but this is a **constant-factor effect only**.

---

## Directory Structure

```
Q2/
├── q2_merge_sort_vs_modified_merge_sort.c    # C source: 2-way and 3-way implementations + benchmark
├── q2_merge_sort_vs_modified_merge_sort.py   # Python: reads CSV, generates dual-panel graph
├── merge_sort_vs_modified_merge_sort.csv     # Benchmark data (21 rows)
├── detailed_merge_sort_analysis.png              # Dual-panel analysis graph
└── README.md                                 # This file
```
