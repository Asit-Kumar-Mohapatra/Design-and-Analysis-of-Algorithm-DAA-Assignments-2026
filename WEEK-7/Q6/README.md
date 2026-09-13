<div align="center">
  
# ⏳ The Best Time to be Alive
*An elegant $O(n \log n)$ Sweep-Line Algorithm for interval overlap counting.*

---
</div>

## 📖 Problem Description
> You are given a historical index of prominent scientists containing their birth and death years. The input data is sorted alphabetically by the scientist's name, meaning the years are provided in no particular chronological order.

Your task is to find the specific year in which the maximum number of these scientists were alive simultaneously. 

**Tie-breaking rule:** If Scientist A dies in the exact same year that Scientist B is born, A's death is considered to happen *before* B's birth. Therefore, A and B are not considered alive at the same time during that shared year.

<br>

## ⚙️ Code Explanation
The C code uses an elegant **Sweep-Line Algorithm** (Interval Overlap Counting) to solve the problem efficiently, disregarding the unhelpful alphabetical order entirely.

### 💡 The Core Logic
The approach works as follows:
1. **Event Transformation:** Every lifespan is converted into two discrete timestamped events:
   - `(birth_year, +1)` indicating one more person is alive.
   - `(death_year, -1)` indicating one fewer person is alive.
2. **Chronological Sorting:** All $2n$ events are sorted by year in ascending order. 
   - *Crucially, the tie-breaking rule is naturally satisfied:* since deaths are represented as `-1` and births as `+1`, a standard ascending sort by `(year, value)` intrinsically places deaths before births in any tied year. No special-case logic is needed.
3. **Sweeping:** The sorted event array is iterated linearly from left to right. A running total `alive` is updated by the event delta (`+1` or `-1`). 
   - The maximum value of `alive` seen during this sweep and the year it occurred are recorded.

### ⏱️ Complexity Profile
| Metric | Rating | Description |
| :--- | :---: | :--- |
| **Time Complexity** | $O(n \log n)$ | Converting lifespans to events and the final sweep are $O(n)$, but sorting the $2n$ events dictates the $O(n \log n)$ time bound. This is asymptotically optimal, as finding maximum overlap of unordered intervals fundamentally requires sorting. |
| **Space Complexity** | $O(n)$ | Memory is allocated for $2n$ discrete event structures. |

> **Note:** The code features a brute-force reference implementation $O(n^2)$ that strictly checks overlap year-by-year. The validation suite confirms the sweep-line algorithm produces identical results while successfully maintaining its optimal $O(n \log n)$ scaling behavior under heavy loads.

<br>

## 📊 Graph & Complexity Analysis
> 📈 **File Reference:** `complexity_analysis.png`

The accompanying graph showcases the performance curve of the $O(n \log n)$ sweep-line algorithm.

It highlights how execution time scales as the number of scientists $n$ grows into the millions. The curve remains tightly bounded, demonstrating that the time taken relative to $n \log n$ is virtually a flat horizontal line. This visual proof confirms that the algorithm operates at the strict theoretical lower bound for interval sorting problems, entirely avoiding the quadratic explosion of naive counting methods.

---
<div align="center">
  <i>Algorithm Design & Analysis - Week 7</i>
</div>
