# Design and Analysis of Algorithms (DAA) — Assignments 2026

| | |
|---|---|
| **Student Name** | Asit Kumar Mohapatra |
| **Registration ID** | B525017 |
| **Branch** | CE (Computer Engineering) |
| **Course** | Design and Analysis of Algorithms |
| **Language** | C (C99) |

---

## Repository Structure

```
.
├── README.md
└── WEEK-1/
    ├── Assignment-1 questions.pdf
    ├── q1_put_them_in_order.c
    ├── q2_fair_vs_biased_coin.c
    ├── q3_performance_analysis_of_bubble_sort.c
    ├── q4_towers_of_hanoi.c
    ├── q5_find_the_partition_point.c
    ├── q6_element_uniqueness.c
    ├── fair_vs_biased_coin.csv / .png
    ├── performance_analysis_of_bubble_sort.csv / .png
    └── towers_of_hanoi.csv / .png
```

Each program prints a formatted report to the terminal, draws an ASCII graph where a
plot is meaningful, and — for the three simulation questions — writes a `.csv` dataset
next to the executable. The `.png` files are the plots generated from those datasets.

---

## WEEK-1

### Q1 — Put Them in Order

Arranges twelve given functions in increasing order of asymptotic growth.

The functions are stored in a table along with their growth group, dominant power, and
leading coefficient. A **merge sort** driven by a custom comparator orders them purely
on theoretical bounds, using the coefficient as a strict tie-breaker so that functions
sharing a Big-O class (for example `100n² + 6n` and `n² − 324`) still resolve to a
definite position.

Output has three parts: the ordering on one line with strict `<` inequalities, the
actual computed values at `n = 10⁸`, and an ASCII bar chart at `n = 10¹⁰⁰`. Values
beyond the range of a `double` are reported as `INF` rather than overflowing, and the
chart uses a log-of-log scale so that `1/n` through `3^n` fit on the same axis.

Ordering produced:

```
1/n < log2(n) < 12*sqrt(n) < 50*n^0.5 < n^0.51 < (2^32)*n
    < n*log2(n) < n^2-324 < 100*n^2+6*n < 2*n^3 < n^(log2 n) < 3^n
```

### Q2 — Fair vs Biased Coin

Simulates coin tosses to demonstrate the **Law of Large Numbers**.

Phase 1 flips a fair coin in batches that scale 10× each step (10 → 1,000,000) and
tabulates the observed probability against the absolute error from 0.5 — the error
shrinks as the sample grows. Phase 2 runs five coins at 50%, 60%, 70%, 90%, and 30%
bias over a user-supplied number of flips, then exports every row to
`fair_vs_biased_coin.csv`. Phase 3 renders the observed probabilities as an ASCII
histogram.

Complexity: **O(k·n)** time for `k` coins and `n` flips, **O(k)** space.

### Q3 — Performance Analysis of Bubble Sort

Compares two Bubble Sort variants on the *same* randomized dataset:

1. **Optimized** — stops early once a pass completes with no swaps.
2. **Naive** — always runs all `n − 1` passes.

Both receive a clone of one base array, so the comparison counts are directly
comparable. The program reports the operations saved, draws a scaled ASCII bar graph,
writes `performance_analysis_of_bubble_sort.csv`, and prints a complexity table.

| Metric | Early Stop | Full Pass |
|---|---|---|
| Best case | O(n) | O(n²) |
| Average case | O(n²) | O(n²) |
| Worst case | O(n²) | O(n²) |
| Space | O(1) | O(1) |

On random input the early-stop variant rarely triggers its shortcut, so the gain shows
up mainly on already-sorted or nearly-sorted data — which is exactly what the best-case
row records.

### Q4 — Towers of Hanoi

Solves the puzzle recursively and analyses its growth.

A step-by-step trace is printed for 3 discs, then the solver runs silently for
`n = 1…25`, recording the simulated move count beside the closed-form value `2ⁿ − 1`.
The two columns match for every `n`, which validates the recursion against the formula.
The dataset is exported to `towers_of_hanoi.csv`.

The recursion counter is threaded through as a pointer rather than a global, so each run
starts clean. Bar length in the plot is set to `n` itself, which makes the chart a
log-scale view — the straight line it produces is the signature of exponential data.

Conclusion drawn from the plot:
- **Time O(2ⁿ)** — every additional disc doubles the work.
- **Practically infeasible** beyond roughly `n > 64`.
- **Space O(n)** — recursion only ever goes `n` frames deep.

### Q5 — Find the Partition Point

Given an array of 0s strictly followed by 1s, finds the index where the transition
happens using **binary search**.

When the midpoint holds a 1 the index is recorded as a candidate and the search moves
left, hunting for an earlier 1; when it holds a 0 the transition must lie to the right.
The midpoint is computed as `left + (right − left) / 2` to avoid integer overflow. Both
degenerate inputs are handled explicitly — an all-0s array reports no transition, and an
all-1s array reports the transition at index 0.

Complexity: **O(log n)** time, **O(1)** space.

### Q6 — Element Uniqueness

Checks whether an array contains duplicates by brute-force pairwise comparison, and
counts the comparisons actually performed.

The nested loop compares each element against every later element and returns as soon as
a match is found, so the comparison counter doubles as a measure of how early the
duplicate appeared.

| Case | Time |
|---|---|
| Best (duplicate found immediately) | O(1) |
| Worst (all unique) | O(n²) |
| Space | O(1) |

The program notes that sorting first or using a hash map would reduce the time cost,
at the expense of the O(1) space this version keeps.

---

## Building and Running

All programs are single-file and standard C. Q1 and Q2 use `math.h`, so link the math
library with `-lm`.

```bash
cd WEEK-1

# Q1 and Q2 need -lm
gcc -Wall -o q1 q1_put_them_in_order.c -lm && ./q1
gcc -Wall -o q2 q2_fair_vs_biased_coin.c -lm && ./q2

# Q3-Q6
gcc -Wall -o q3 q3_performance_analysis_of_bubble_sort.c && ./q3
gcc -Wall -o q4 q4_towers_of_hanoi.c && ./q4
gcc -Wall -o q5 q5_find_the_partition_point.c && ./q5
gcc -Wall -o q6 q6_element_uniqueness.c && ./q6
```

### Inputs Expected

| Program | Input |
|---|---|
| Q1 | none |
| Q2 | number of flips for the comparison matrix (defaults to 10000 on bad input) |
| Q3 | number of elements, 2–10000 (defaults to 100 on bad input) |
| Q4 | none |
| Q5 | element count, then that many 0s and 1s — 0s first |
| Q6 | element count, then that many integers |

CSV files are written to the current working directory, so run the programs from inside
`WEEK-1` to keep the output beside the existing datasets.

---

## Notes

- Q2 and Q3 seed `rand()` from `time(NULL)`, so their numbers differ between runs; the
  committed CSV and PNG files capture one representative run.
- Q3, Q5, and Q6 allocate their arrays with `malloc` and free them before exit.
- Q1's ASCII chart uses a Unicode block character; the other programs use plain `#` to
  stay readable on terminals without UTF-8.

