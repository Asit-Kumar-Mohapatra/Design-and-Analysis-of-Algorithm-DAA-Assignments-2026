<div align="center">

<h1>
  <img src="https://img.shields.io/badge/Week%206-DAA%20Lab-E91E63?style=for-the-badge&logo=c&logoColor=white"/>
</h1>

<h2>🔄 Q4 — Sorting via the Reversal Procedure</h2>

<p>
  <img src="https://img.shields.io/badge/Language-C-00599C?style=flat-square&logo=c"/>
  <img src="https://img.shields.io/badge/Algorithm%20A-O(n)%20Reversals-E91E63?style=flat-square"/>
  <img src="https://img.shields.io/badge/Algorithm%20B-O(n%20log²%20n)%20Cost-9C27B0?style=flat-square"/>
  <img src="https://img.shields.io/badge/Primitive-reverse(p%2C%20i%2C%20j)-FF9800?style=flat-square"/>
</p>

</div>

---

## 📌 Overview

This question addresses a specialized sorting problem: given a **permutation** of {1, 2, ..., n}, sort it using **only the reversal operation**:

```
reverse(p, i, j)  →  reverses the sub-sequence  p[i], p[i+1], ..., p[j]
```

The problem is answered in **two parts:**

| | Algorithm | Optimizes | Bound |
|---|----------|-----------|-------|
| **Part 1** | Selection Sort by Reversal | Number of `reverse()` calls | ≤ n−1 = **O(n) reversals** |
| **Part 2** | Merge Sort + Rotation-Reversal | Total cost (sum of all lengths) | **O(n log² n) total cost** |

Both algorithms operate on a `Perm` struct that tracks every `reverse()` call, its length, and cumulative cost — enabling **empirical verification** of the theoretical complexity claims.

---

## 🎮 Program Modes

<table>
<thead>
  <tr>
    <th style="background:#E91E63;color:white;">Mode</th>
    <th style="background:#E91E63;color:white;">Trigger</th>
    <th style="background:#E91E63;color:white;">Description</th>
  </tr>
</thead>
<tbody>
  <tr>
    <td><b>[1] Custom Permutation</b></td>
    <td>Menu option 1</td>
    <td>Enter your own permutation. Both algorithms run step-by-step with a full printed trace: every <code>reverse(p, i, j)</code> call, its cost, and the array state before/after.</td>
  </tr>
  <tr>
    <td><b>[2] Automated Validation</b></td>
    <td>Menu option 2</td>
    <td>Runs 30,000 randomly generated permutations. Verifies correctness and checks Algorithm A never exceeds n−1 reversals; tracks Algorithm B''s cost/bound ratio.</td>
  </tr>
  <tr>
    <td><b>[3] Theoretical Analysis</b></td>
    <td>Menu option 3</td>
    <td>Prints a complete self-contained proof of correctness and cost bounds for both algorithms.</td>
  </tr>
  <tr>
    <td><b>Non-interactive CI</b></td>
    <td><code>./reversal_sort -test</code></td>
    <td>Jumps straight to the 30,000-case validation suite with no menu — useful for scripts and CI pipelines.</td>
  </tr>
</tbody>
</table>

---

## ⚡ Part 1 — Algorithm A: O(n) Reversals

### 🔴 Selection Sort by Reversal

**Core Idea:** Place elements one at a time into their correct positions using at most one reversal per element.

**Loop Invariant:** Before iteration `i`, positions `1..i−1` already contain values `1..i−1` in sorted order.

**Step for value `i`:**

```
1. Find index j such that p[j] = i
2. If j == i  →  element already in place, skip
3. Else       →  call reverse(p, i, j)
               •  This moves value i from position j to position i
               •  Positions < i are NEVER touched (since j ≥ i)
               •  Invariant is preserved for all previously placed elements
```

After n−1 iterations, positions 1..n−1 are correct; position n is forced correct by elimination.

**Reversal Count Proof:**
- Each iteration performs **at most one** `reverse()` call.
- There are exactly n−1 iterations.
- Total reversals ≤ **n − 1 = O(n)**. ✅

### ⚠️ Why Total Cost Can Be O(n²)

A single reversal can span up to n elements. For the worst-case input (the fully reversed permutation `[n, n-1, ..., 1]`), the reversals have lengths n, n−1, ..., 2:

```
Total cost = n + (n−1) + (n−2) + ... + 2 = n(n+1)/2 − 1 = O(n²)
```

This is exactly why Part 2 demands a cost-bounded algorithm.

### 🔧 Implementation Detail

A `pos[]` auxiliary array maintains `pos[v] = current index of value v`, enabling O(1) lookup of where value `i` is located instead of a linear scan each time.

---

## ⚡ Part 2 — Algorithm B: O(n log² n) Total Cost

### 🟣 Merge Sort + Rotation-Reversal

**Core Idea:** Standard top-down merge sort, where the merge step uses **in-place rotations**, and every rotation is implemented by exactly **3 reversal calls** (the classic reversal rotation trick).

---

### 🔑 The 3-Reversal Rotation Trick

To move block B = [mid, hi) in front of block A = [lo, mid):

```
rotate_left(lo, mid, hi):
    reverse(lo,   mid−1)    // reverse block A  →  rev(A) B
    reverse(mid,  hi−1)     // reverse block B  →  rev(A) rev(B)
    reverse(lo,   hi−1)     // reverse whole    →  B A   ✅
```

**Algebraic proof:** `reverse( rev(A) ∥ rev(B) ) = B A`
This is a standard identity — reversing a concatenation of two already-reversed blocks un-reverses each and swaps their order.

**Cost of one rotation:** `(mid−lo) + (hi−mid) + (hi−lo) = 2 × (hi−lo) = O(length)`.

---

### 🔑 In-Place Merge — Rotate-Merge Algorithm

Given two sorted adjacent blocks A = [lo, mid) and B = [mid, hi), merge proceeds by divide and conquer:

```
merge_inplace(lo, mid, hi):
    if p[mid−1] <= p[mid]:  return   // already joined — base case + infinite-recursion guard

    if |A| >= |B|:
        mid1 = median index of A
        val  = p[mid1]
        mid2 = first index in B where B[k] >= val   (binary search, O(log n))
        rotate_left(mid1, mid, mid2)                 // place val at its final position
        mid'' = mid1 + (mid2 − mid)                  // val now sits here
        recurse on [lo, mid1) + [lo, mid'')          // left sub-problem
        recurse on [mid'', mid2) + [mid2, hi)        // right sub-problem
    else:
        (symmetric, using right half''s median)
```

> **Critical:** The boundary check `p[mid-1] <= p[mid]` is essential for both performance and correctness — without it, a degenerate case would reproduce the exact same recursive call, causing **infinite recursion**.

---

### 📊 Cost Analysis

**Cost of one `merge_inplace(lo, mid, hi)` call:**

| Property | Value |
|----------|-------|
| Work per level | O(n1 + n2) — O(1) rotations of total length O(range) |
| Recursion depth | O(log(min(n1, n2) + 1)) — smaller side always binary-searched |
| **Total** | **O((n1 + n2) × log(min(n1, n2) + 1))** |

**Summing over all merge-sort levels:**

<table>
<thead>
  <tr>
    <th style="background:#9C27B0;color:white;">Depth d</th>
    <th style="background:#9C27B0;color:white;">Merges</th>
    <th style="background:#9C27B0;color:white;">Each size</th>
    <th style="background:#9C27B0;color:white;">Cost per level</th>
  </tr>
</thead>
<tbody>
  <tr><td>0 (root)</td><td>1</td><td>n</td><td>O(n log n)</td></tr>
  <tr><td>1</td><td>2</td><td>n/2</td><td>O(n log n)</td></tr>
  <tr><td>2</td><td>4</td><td>n/4</td><td>O(n log n)</td></tr>
  <tr><td>⋮</td><td>⋮</td><td>⋮</td><td>⋮</td></tr>
  <tr><td>log n</td><td>n/2</td><td>2</td><td>O(n)</td></tr>
</tbody>
</table>

**Total:**
```
O(log n) levels  ×  O(n log n) per level  =  O(n log² n)   ✅
```

---

## 📐 Algorithm Comparison

<table>
<thead>
  <tr>
    <th style="background:#E91E63;color:white;">Property</th>
    <th style="background:#E91E63;color:white;">Algorithm A</th>
    <th style="background:#E91E63;color:white;">Algorithm B</th>
  </tr>
</thead>
<tbody>
  <tr><td><b>Strategy</b></td><td>Selection sort by reversal</td><td>Merge sort + rotation-reversals</td></tr>
  <tr><td><b>Reversal count</b></td><td>≤ n−1 = <b>O(n)</b></td><td>O(n log n) calls</td></tr>
  <tr><td><b>Total cost (sum of lengths)</b></td><td>O(n²) worst case</td><td><b>O(n log² n)</b></td></tr>
  <tr><td><b>What it optimizes</b></td><td>Number of <code>reverse()</code> calls</td><td>Total work performed</td></tr>
</tbody>
</table>

---

## 🧪 Validation Suite

The automated harness runs **30,000 random permutations** (n up to 4,000) using a fixed xorshift64 seed for full reproducibility.

**Checks for Algorithm A:**
- ✅ Result equals `[1, 2, 3, ..., n]`
- ✅ `reversal_count ≤ n−1` on every single test case

**Checks for Algorithm B:**
- ✅ Result equals `[1, 2, 3, ..., n]`
- ✅ Tracks `total_cost / (n × log₂(n)²)` — confirms ratio stays **bounded** (does not grow with n), validating O(n log² n)

---

## 📂 Files in This Folder

| File | Description |
|------|-------------|
| `q4_sorting_via_reversal_procedure.c` | Main C source — both algorithms, trace mode, validation suite, theory explanation |
| `q4_sorting_via_reversal_procedure.exe` | Compiled executable |
| `reversal_sort_complexity.png` | Plot comparing O(n) reversal count (Algo A) vs O(n log² n) cost (Algo B) |

---

## 🛠️ How to Compile and Run

```bash
# Compile
gcc -O2 -Wall -o reversal_sort q4_sorting_via_reversal_procedure.c -lm

# Interactive menu
./reversal_sort

# Non-interactive: validation suite only (for CI)
./reversal_sort -test
```

---

## 🔑 Key Design Decisions

| Decision | Reason |
|----------|--------|
| `Perm` struct with `reversal_count` and `total_cost` | Enables empirical verification of O(n) and O(n log² n) claims without touching algorithm logic |
| `verbose_log` flag + `verbose_tag` string | Trace mode prints every call with its reason and cost; purely observational, zero effect on results |
| Boundary check `p[mid-1] <= p[mid]` | Both a performance optimization (skip trivially merged boundary) and a termination guarantee (prevents infinite recursion) |
| xorshift64 RNG with fixed seed | Fast, no-library RNG producing fully reproducible test runs |
| Half-open range convention `[lo, hi)` | Standard for merge sort; eliminates off-by-one errors |
| 1-indexed permutation array `p[1..n]` | Matches the mathematical definition of a permutation of {1, ..., n}; `p[0]` is intentionally unused |

---

<div align="center">
<sub>📘 DAA Lab · Week 6 · Q4 · Sorting via the Reversal Procedure</sub>
</div>
