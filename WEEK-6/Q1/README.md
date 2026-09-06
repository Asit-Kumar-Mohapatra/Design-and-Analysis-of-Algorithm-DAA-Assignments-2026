<div align="center">

<h1>
  <img src="https://img.shields.io/badge/Week%206-DAA%20Lab-6C63FF?style=for-the-badge&logo=c&logoColor=white"/>
</h1>

<h2 style="color:#6C63FF;">🧮 Q1 — 1D Array Operations & Their Worst-Case Optimal Complexities</h2>

<p>
  <img src="https://img.shields.io/badge/Language-C-00599C?style=flat-square&logo=c"/>
  <img src="https://img.shields.io/badge/Paradigm-Comparison--Based-FF6B6B?style=flat-square"/>
  <img src="https://img.shields.io/badge/Operations-9-4CAF50?style=flat-square"/>
  <img src="https://img.shields.io/badge/Complexity-Θ(n)%20to%20Θ(n%20log%20n)-FFA500?style=flat-square"/>
</p>

</div>

---

## 📌 Overview

This question explores a comprehensive set of **nine fundamental operations** performed on a one-dimensional integer array of size `n`. For each operation, the program:

1. 🔎 Establishes the **theoretical lower bound** — why no algorithm can do better.
2. ⚙️ Identifies the **algorithm** used to achieve that bound.
3. ✅ Justifies **why the algorithm is optimal** — it meets the lower bound exactly.
4. 📊 Computes and **displays the result** on the user-supplied input array.

> **Data Structure Choice:** A plain C `int arr[]` in contiguous memory, providing **O(1) random access** and **O(1) in-place swapping** — a linked list cannot offer these for the operations below.

---

## ⚡ Operations Covered

<table>
<thead>
  <tr>
    <th style="background:#6C63FF;color:white;">#</th>
    <th style="background:#6C63FF;color:white;">Operation</th>
    <th style="background:#6C63FF;color:white;">Worst-Case Time</th>
    <th style="background:#6C63FF;color:white;">Algorithm Used</th>
  </tr>
</thead>
<tbody>
  <tr><td><b>i</b></td><td>Maximum Element</td><td><code>Θ(n)</code></td><td>Linear scan, running max</td></tr>
  <tr><td><b>ii</b></td><td>First &amp; Second Largest</td><td><code>Θ(n)</code></td><td>Single pass, two trackers</td></tr>
  <tr><td><b>iii</b></td><td>Mean</td><td><code>Θ(n)</code></td><td>Single accumulation pass</td></tr>
  <tr><td><b>iv</b></td><td>Median</td><td><code>Θ(n)</code></td><td>Median-of-Medians (BFPRT) SELECT</td></tr>
  <tr><td><b>v</b></td><td>Standard Deviation</td><td><code>Θ(n)</code></td><td>Two sequential linear passes</td></tr>
  <tr><td><b>vi</b></td><td>Mode</td><td><code>Θ(n log n)</code></td><td>Sort + linear run-length scan</td></tr>
  <tr><td><b>vii</b></td><td>Remove Duplicates</td><td><code>Θ(n log n)</code></td><td>Sort + in-place compaction</td></tr>
  <tr><td><b>viii</b></td><td>Reverse Array</td><td><code>Θ(n)</code></td><td>Two-pointer in-place swap</td></tr>
  <tr><td><b>ix</b></td><td>Partition Around Pivot</td><td><code>Θ(n)</code></td><td>Single sweep with slot pointer</td></tr>
</tbody>
</table>

---

## 🔬 Detailed Analysis

### 🟣 (i) Maximum Element — `Θ(n)`

| | |
|---|---|
| **Lower Bound** | Every element must be examined — an unexamined element could be the true max → **Ω(n)** |
| **Algorithm** | Single linear scan keeping a running maximum; exactly n−1 comparisons |
| **Optimal?** | ✅ Yes — O(n) scan meets Ω(n). No comparison-based algorithm can do better. |

---

### 🟣 (ii) First and Second Largest — `Θ(n)`

| | |
|---|---|
| **Lower Bound** | An unseen element could beat "second" → **Ω(n)** |
| **Algorithm** | Single pass maintaining `(first, second)` with at most 2 comparisons per element |
| **Optimal?** | ✅ Yes. A tournament method cuts the comparison *count* to `n + ⌈log₂n⌉ − 2`, but this only changes the constant — the Θ(n) class is unchanged |
| **Note** | Duplicates count separately: for `[9, 9, 7]`, the answer is `9` and `9` |

---

### 🟣 (iii) Mean — `Θ(n)`

| | |
|---|---|
| **Lower Bound** | The sum depends on every element — omitting any one changes the result → **Ω(n)** |
| **Algorithm** | Single accumulation pass using `long long` to prevent overflow, then divide by n |
| **Optimal?** | ✅ Yes — O(n) meets Ω(n) |

---

### 🟠 (iv) Median — `Θ(n)` *(most sophisticated operation)*

| | |
|---|---|
| **Lower Bound** | Every element must be read → **Ω(n)** |
| **Naive (rejected)** | Sorting first → Θ(n log n) — does **not** meet Ω(n), therefore suboptimal |
| **Algorithm** | **Deterministic Median-of-Medians (BFPRT) SELECT** |
| **Optimal?** | ✅ Yes — guaranteed O(n) worst case, unlike quickselect which is O(n²) worst case |

**How BFPRT works:**

```
1. Divide array into groups of 5
2. Find median of each group via insertion sort  →  O(1) per group
3. Recursively find median of those group-medians
4. Use median-of-medians as pivot → guarantees ≤ 7n/10 elements on each side
5. Partition and recurse on the relevant side
```

**Recurrence:** `T(n) = T(⌈n/5⌉) + T(≤7n/10) + O(n)` → solves to **O(n)** guaranteed.

> Operates on a **private copy** of the array — the caller's original order is never disturbed.

---

### 🟣 (v) Standard Deviation — `Θ(n)`

| | |
|---|---|
| **Lower Bound** | Mean must be fully computed before any deviation can be found → two Ω(n) passes unavoidable |
| **Algorithm** | Pass 1: compute mean. Pass 2: compute `Σ(xᵢ − mean)²`. Take square root. |
| **Optimal?** | ✅ Yes — two sequential O(n) passes still compose to Θ(n) |

---

### 🟠 (vi) Mode — `Θ(n log n)`

| | |
|---|---|
| **Lower Bound** | Unbounded integer range → counting sort unsafe. Comparison-based sorting lower bound → **Ω(n log n)** |
| **Algorithm** | `qsort` → Θ(n log n), then one O(n) linear scan tracking the longest consecutive run |
| **Optimal?** | ✅ Yes for worst case. Hash table gives Θ(n) *average* but degrades to O(n²) under adversarial collisions |

---

### 🟠 (vii) Remove Duplicates — `Θ(n log n)`

| | |
|---|---|
| **Lower Bound** | Same unbounded-range argument → **Ω(n log n)** for a safe worst-case method |
| **Algorithm** | Sort → bring equal elements adjacent. One forward compaction pass keeps only first of each run. In-place, O(1) extra space. |
| **Optimal?** | ✅ Yes — Θ(n log n) + Θ(n) = Θ(n log n) |

---

### 🟣 (viii) Reverse Array — `Θ(n)`

| | |
|---|---|
| **Lower Bound** | Every element must move to its mirrored slot → **Ω(n)** |
| **Algorithm** | Two-pointer in-place swap: n/2 swaps, O(1) extra space |
| **Optimal?** | ✅ Yes — both time (O(n) = Ω(n)) and space (O(1)) are optimal |

---

### 🟣 (ix) Partition Around Random Pivot — `Θ(n)`

| | |
|---|---|
| **Lower Bound** | Every element must be classified as ≥ pivot or < pivot → **Ω(n)** |
| **Algorithm** | Single sweep with a "next slot for ≥ pivot" pointer. After the call: `[ ≥pivot | <pivot ]` |
| **Pivot selection** | Chosen randomly from the array's own values at runtime (seeded with `time.h`) |
| **Optimal?** | ✅ Yes — same cost class as a single quicksort partition step |

---

## 📂 Files in This Folder

| File | Description |
|------|-------------|
| `q1_1D_array_operations_and_their_complexities.c` | Main C source — all nine operations with inline complexity explanations |
| `q1_1D_array_operations_and_their_complexities.exe` | Compiled executable |
| `complexity_analysis.png` | Empirical timing graph: measured runtimes vs. theoretical curves |

---

## 🛠️ How to Compile and Run

```bash
# Compile
gcc -O2 -Wall -std=c11 q1_1D_array_operations_and_their_complexities.c -o q1 -lm

# Run interactively
./q1
# → Enter n, then n space-separated integers
```

---

## 🔑 Key Design Decisions

| Decision | Reason |
|----------|--------|
| `long long` accumulator for mean/stddev | Prevents integer overflow for large arrays |
| Private copy for median | BFPRT rearranges the array; a clone protects caller's data |
| Private copy for mode/dedup | `qsort` is in-place; clone avoids corrupting original |
| `time.h`-seeded `rand()` for pivot | Ensures genuinely random pivot at runtime |

---

<div align="center">
<sub>📘 DAA Lab · Week 6 · Q1 · 1D Array Operations</sub>
</div>
