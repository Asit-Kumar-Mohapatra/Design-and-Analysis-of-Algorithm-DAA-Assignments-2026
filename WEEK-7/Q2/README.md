<div align="center">
  
# 🥚 Super Egg Testing Experiment
*An optimized dynamic programming solution for critical floor identification.*

---
</div>

## 📖 Problem Description
> You are given $E$ identical eggs and a building with $F$ floors. You need to find the minimum number of droppings that **guarantees** (in the worst-case scenario) finding the highest floor from which an egg can be dropped without breaking.

If an egg breaks, you can no longer use it. If it survives, you can reuse it for another drop. The challenge is to minimize the number of drops while covering all possible worst-case scenarios, ensuring that you can always exactly determine the critical floor.

<br>

## ⚙️ Code Explanation
The provided C program implements a highly optimized **Dynamic Programming (DP)** approach to solve the problem efficiently.

### 💡 The Core Logic
A naive DP approach would define `dp(e, f)` as the minimum drops for $e$ eggs and $f$ floors, checking all intermediate floors, resulting in a time complexity of $O(E \times F^2)$. This is far too slow for large buildings (e.g., $F = 100,000$).

The optimized code **flips the question**:
**"With $e$ eggs and $d$ drops, what is the maximum number of floors $F$ I can fully resolve?"**

Let this be `maxFloors(e, d)`. If we drop an egg:
1. **It Breaks:** we have $e-1$ eggs and $d-1$ drops to explore the floors below.
2. **It Survives:** we have $e$ eggs and $d-1$ drops to explore the floors above.

This leads to an elegant recurrence relation:
```c
maxFloors(e, d) = maxFloors(e-1, d-1) + maxFloors(e, d-1) + 1
```

The algorithm simply increments the drop count $d = 1, 2, 3, \dots$ until `maxFloors(E, d) >= F`. The current $d$ is the guaranteed minimum number of drops required.

### ⏱️ Complexity Profile
| Metric | Rating | Description |
| :--- | :---: | :--- |
| **Time Complexity** | $O(E \times \log F)$ | Specifically $O(E \times d^*)$, where $d^*$ is the final answer. Since $d^*$ grows logarithmically with $F$, this allows the algorithm to handle hundreds of thousands of floors in milliseconds. |
| **Space Complexity** | $O(E)$ | The code maintains a single 1D array of size $E+1$, updated in-place from right to left to conserve memory. |

> **Note:** The program also features an explanation mode and a rigorous validation suite cross-checking against the naive $O(E \times F^2)$ DP for smaller test cases to guarantee absolute correctness.

<br>

## 📊 Graph & Complexity Analysis
> 📈 **File Reference:** `egg_drop_complexity_analysis.png`

The visualization plots the relationship between the number of floors $F$, the number of eggs $E$, and the required number of drops or execution time. 

The graph highlights the dramatic reduction in computational time from the naive $O(E \times F^2)$ to the optimized $O(E \times \log F)$ approach. As $F$ scales to massive numbers, the execution time remains minuscule. The growth of required drops relative to $F$ follows a sharp logarithmic curve when $E > 1$, mathematically proving the extreme efficiency of the `maxFloors` inverted formulation.

---
<div align="center">
  <i>Algorithm Design & Analysis - Week 7</i>
</div>
