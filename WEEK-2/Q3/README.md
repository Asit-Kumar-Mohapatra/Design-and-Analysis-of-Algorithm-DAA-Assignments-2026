# Q3 — Merging k Sorted Arrays: Sequential vs. Divide-and-Conquer

## Problem Statement

Given **k sorted arrays**, each containing **n elements**, merge them into a single sorted array of **k*n elements**.

Analyze and compare two distinct merging strategies:

- **Method 1 — Sequential (Repeated) Merge:** Merge arrays one at a time, left to right, accumulating a growing partial result
- **Method 2 — Pairwise (Divide-and-Conquer) Merge:** Pair up arrays in each round, halving the count of arrays per round until one sorted array remains

The objectives are to:
1. Derive the worst-case time complexity for both methods
2. Implement both methods in C with correctness validation against a reference sort
3. Benchmark empirically over varying k (n fixed) and varying n (k fixed)
4. Confirm that the growth shapes match theoretical predictions

---

## Theoretical Analysis

### Method 1 — Sequential Merge: O(n * k^2)

**How it works:**
```
merge(A1, A2)        -> R2   (size 2n)
merge(R2, A3)        -> R3   (size 3n)
merge(R3, A4)        -> R4   (size 4n)
...
merge(R(k-1), Ak)    -> Rk   (size k*n)
```

A merge of two sorted arrays of lengths p and q takes O(p + q) time (classic linear zip merge).

**Cost analysis:**
- Step i (for i = 2 to k): merging a partial result of size (i-1)*n with array Ai of size n → cost O(i*n)
- Total cost = sum_{i=2}^{k} O(i*n) = O(n * (2 + 3 + ... + k)) = O(n * k*(k+1)/2 - 1) = **O(n * k^2)**

This degenerates similarly to insertion sort over arrays: each newly merged array must be merged against a partial result that keeps growing.

### Method 2 — Pairwise / Divide-and-Conquer Merge: O(n * k * log k)

**How it works:**
```
Round 1: pair up k arrays of size n     -> k/2 merges, each O(2n)   -> total O(k*n)
Round 2: pair up k/2 arrays of size 2n  -> k/4 merges, each O(4n)   -> total O(k*n)
Round 3: pair up k/4 arrays of size 4n  -> k/8 merges, each O(8n)   -> total O(k*n)
...
```

Every round touches each of the k*n elements exactly once: there are `log_2(k)` rounds, and each round costs O(k*n).

**Total cost = O(k*n) * log_2(k) = O(n * k * log k)**

This is the same asymptotic advantage merge sort has over insertion sort — divide-and-conquer prevents the growing partial-result problem.

### Comparison Table

| Method                | Strategy             | Worst-Case Time Complexity |
|-----------------------|----------------------|----------------------------|
| Method 1 (Sequential) | Left-to-right merge  | O(n * k^2)                 |
| Method 2 (Pairwise)   | Divide and conquer   | O(n * k * log k)           |

Since `log k` grows far slower than `k`, Method 2 is **asymptotically and empirically superior** for large k. For `k = 2`, both methods are equivalent (one merge either way).

---

## File Descriptions

### `q3_merging_k_sorted_arrays.c` — Main Implementation and Benchmark
**Language:** C | **Lines:** 415

This is the complete implementation, validation, and benchmarking program.

#### Portable High-Resolution Timer (lines 116–136)
To achieve sub-microsecond timing accuracy (especially on Windows where `clock()` has ~10-16ms effective resolution), the program uses:
- **Windows:** `QueryPerformanceCounter` / `QueryPerformanceFrequency` (sub-microsecond)
- **POSIX/Linux:** `clock_gettime(CLOCK_MONOTONIC, ...)` (nanosecond precision)

This replaces `clock()` to prevent flat/stepped timing plateaus at small k values.

#### `merge_two()` — Core Merge Primitive (lines 142–154)
Classic two-pointer merge of two sorted int arrays `a[0..na)` and `b[0..nb)` into a freshly `malloc`'d output array of size `na + nb`. Time: O(na + nb).

#### `method1_sequential_merge()` — Method 1 (lines 161–175)
- Starts with a copy of `arrays[0]` (size n)
- Iterates from i = 1 to k-1: merges the growing `result` (size i*n) with `arrays[i]` (size n), frees the old result, updates result
- Returns a `malloc`'d array of size k*n

#### `method2_pairwise_merge()` — Method 2 (lines 183–226)
Bottom-up iterative divide-and-conquer implementation using a working list of (pointer, length) pairs:
- In each round, pairs `cur[i]` with `cur[i+1]` and merges them via `merge_two()`
- An odd leftover array (when count is odd) is carried forward unchanged to the next round
- Continues until only one array remains (the fully merged result)

#### `generate_arrays()` — Test Data Generation (lines 241–251)
Generates k sorted arrays of n random integers each:
- Each element: `rand() % 1000000` (range 0 to 999,999)
- Each array individually sorted with `qsort()`

#### `validate()` — Correctness Checker (lines 263–305)
Runs **200 random trials**, each with:
- k in [2, 16] (random)
- n in [1, 50] (random)

For each trial, checks that:
- Both method outputs are **sorted**
- Both method outputs have the correct length (k*n)
- Both method outputs are **identical** to each other
- Both method outputs match a **reference qsort** of the concatenated input

Output: `ALL 200 TRIALS PASSED` if everything is correct.

#### Adaptive Timing Loop (lines 321–345)
Instead of a fixed repeat count, each timing call keeps re-running the merge until the **cumulative elapsed time exceeds 50 ms** (`MIN_BENCH_TIME_SEC = 0.05`), then divides by the repeat count. This self-adjusting approach ensures:
- Very fast configurations (small k, small n) get enough repetitions for a stable non-zero measurement
- Slow configurations (large k, large n) don't spin unnecessarily

#### `main()` — Benchmark Driver (lines 347–415)
Runs two benchmark sweeps and writes results to `merging_k_sorted_arrays.csv`:

**Benchmark 1 — Vary k (n = 400 fixed):**
k values: `{2, 4, 8, 16, 32, 64, 96, 128, 160, 192, 224, 256}`
Isolates k-dependence to confirm O(k^2) vs O(k log k) growth

**Benchmark 2 — Vary n (k = 64 fixed):**
n values: `{50, 100, 200, 400, 800, 1600, 3200, 6400}`
Isolates n-dependence — both methods scale linearly in n; slopes differ by ~log(k) factor

Also prints a theoretical conclusion to the console.

**How to compile and run:**
```bash
gcc -O2 -o q3_merging_k_sorted_arrays q3_merging_k_sorted_arrays.c
./q3_merging_k_sorted_arrays
```

---

### `q3_merging_k_sorted_arrays.py` — Graph Generation Script
**Language:** Python 3 | **Lines:** 273
**Dependencies:** `pandas`, `numpy`, `matplotlib`

Reads `merging_k_sorted_arrays.csv` and produces **3 PNG graphs** with full theoretical overlay curves.

**Fallback behavior:** If the CSV is not found, the script attempts to:
1. Compile and run the C program automatically via `gcc` + subprocess
2. If compilation fails, fall back to a pure-Python simulation of both algorithms for standalone plotting

#### `plot_vs_k()` — Time vs. Number of Arrays (lines 160–191)
Uses the `vary_k` series. Plots measured timings for both methods AND overlays **least-squares-fit theoretical curves**:
- Method 1 theoretical: `C1 * n * k^2`
- Method 2 theoretical: `C2 * n * k * log2(k)`

The constants C1, C2 are fitted to the data (absorbing hardware/malloc overhead) so only the **growth shape** is validated, not absolute values.

#### `plot_vs_n()` — Time vs. Elements per Array (lines 194–214)
Uses the `vary_n` series. Plots raw measured timings for both methods on a linear scale.
- Both methods scale linearly in n (for fixed k), as each element is touched a bounded number of times per round
- The slope ratio between the two methods reflects the ~log(k) factor difference

#### `plot_loglog()` — Log-Log View (lines 217–243)
Uses the `vary_k` series on a log-log scale. Includes a **slope-2 reference line** anchored at the smallest k.
- Method 1's slope ≈ 2 on the log-log plot confirms O(k^2) growth
- Method 2 stays visibly sub-quadratic (slope < 2), confirming O(k log k)

**How to run:**
```bash
python q3_merging_k_sorted_arrays.py
```
> Must be run from the Q3/ directory, or ensure `merging_k_sorted_arrays.csv` is in the working directory.

---

### `merging_k_sorted_arrays.csv` — Benchmark Data
**Format:** CSV with 5 columns | **Rows:** 21 (12 vary_k rows + 8 vary_n rows + header)

| Column            | Type   | Description                                                |
|-------------------|--------|------------------------------------------------------------|
| `series`          | string | `vary_k` (n fixed, k swept) or `vary_n` (k fixed, n swept)|
| `k`               | int    | Number of arrays being merged                              |
| `n`               | int    | Number of elements per array                               |
| `method1_time_ms` | float  | Avg time for Method 1 (sequential) in **milliseconds**     |
| `method2_time_ms` | float  | Avg time for Method 2 (pairwise) in **milliseconds**       |

**Sample rows (vary_k series, n = 400):**
```
series,k,n,method1_time_ms,method2_time_ms
vary_k,2,400,0.003515,0.003889
vary_k,64,400,4.307808,1.626513
vary_k,256,400,59.745100,9.745317
```

**Sample rows (vary_n series, k = 64):**
```
vary_n,64,50,0.447409,0.184777
vary_n,64,6400,67.336100,22.175167
```

> **Key observation:** At k = 256, Method 1 takes ~59.7 ms vs Method 2's ~9.7 ms — a **6x speedup** — directly demonstrating the O(k^2) vs O(k log k) gap widening with k.

---

### Graph Files (3 PNG) — Visual Validation

| File                                      | Series   | X-Axis | Key Visual Feature                                      |
|-------------------------------------------|----------|--------|---------------------------------------------------------|
| `merging_k_sorted_arrays_vs_k.png`        | vary_k   | k      | Method 1 curves up steeply (quadratic); Method 2 curves gently (sub-linear in k); theoretical fits overlay |
| `merging_k_sorted_arrays_vs_n.png`        | vary_n   | n      | Both methods grow linearly in n; Method 1 line is steeper by ~log(k) factor |
| `merging_k_sorted_arrays_loglog.png`      | vary_k   | log(k) | Method 1 slope ≈ 2 confirms O(k^2); Method 2 slope < 2 confirms O(k log k); slope-2 reference guide shown |

---

## How to Reproduce All Results

```bash
# Step 1: Compile and run the C benchmark
gcc -O2 -o q3_merging_k_sorted_arrays q3_merging_k_sorted_arrays.c
./q3_merging_k_sorted_arrays
# Output: merging_k_sorted_arrays.csv (+ validation and console report)

# Step 2: Generate the 3 analysis graphs
python q3_merging_k_sorted_arrays.py
# Output: merging_k_sorted_arrays_vs_k.png
#         merging_k_sorted_arrays_vs_n.png
#         merging_k_sorted_arrays_loglog.png
```

---

## Conclusion

| Criterion               | Method 1 (Sequential)       | Method 2 (Pairwise / D&C)          |
|-------------------------|-----------------------------|------------------------------------|
| Strategy                | Left-to-right repeated merge| Pair-merge in log_2(k) rounds      |
| Worst-Case Time         | O(n * k^2)                  | O(n * k * log k)                   |
| Growth in k (n fixed)   | Quadratic (slope 2 in log-log)| Sub-quadratic (slope < 2 in log-log)|
| Growth in n (k fixed)   | Linear                      | Linear                             |
| Speedup at k = 256      | 59.7 ms                     | 9.7 ms (~6x faster)                |
| Asymptotic Winner?      | No                          | Yes — O(n k log k) beats O(n k^2) |

Method 2 is **decisively superior** for large k. The pairwise divide-and-conquer structure prevents the "growing partial result" penalty of sequential merging, giving it the same fundamental advantage that merge sort has over insertion sort.

---

## Directory Structure

```
Q3/
├── q3_merging_k_sorted_arrays.c      # C source: both methods, validation, adaptive benchmark
├── q3_merging_k_sorted_arrays.py     # Python: reads CSV, generates 3 graphs with theory overlays
├── merging_k_sorted_arrays.csv       # Benchmark data (21 rows, vary_k and vary_n series)
├── merging_k_sorted_arrays_vs_k.png  # Graph: time vs k (with theoretical fit curves)
├── merging_k_sorted_arrays_vs_n.png  # Graph: time vs n (linear scaling confirmation)
├── merging_k_sorted_arrays_loglog.png# Graph: log-log view confirming k^2 vs k log k slopes
└── README.md                         # This file
```
