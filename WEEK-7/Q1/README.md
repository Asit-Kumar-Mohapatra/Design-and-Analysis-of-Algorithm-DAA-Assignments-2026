<div align="center">
  
# 🪙 Invert the Coin-Triangle
*A fully optimized $O(1)$ mathematical solver.*

---
</div>

## 📖 Problem Description
> An equilateral triangle of side $n$ is built from closely packed coins. The total number of coins is given by the triangular number formula $T(n) = \frac{n(n+1)}{2}$, arranged in $n$ rows with 1 coin in the first row, 2 in the second, and so on.

On each move, you can slide **exactly one** coin to a new empty lattice position on the plane. The objective is to find the minimum number of moves required to turn the triangle completely upside down.

This classic puzzle demands not just a simulation but an optimal strategy. Rather than shifting all coins, the optimal strategy keeps as many coins as possible in their current positions by shifting the bounds of the "inverted" triangle down over the original one. 

<br>

## ⚙️ Code Explanation
The C implementation provides a highly optimized **$O(1)$ mathematical solver** to determine the minimum number of moves. 

### 💡 The Core Logic
The core idea is to find the **maximum possible overlap** between the original upright triangle and the target inverted triangle. Every coin within the overlap region is already in its final position and does not need to be moved. The coins outside this overlap must be picked up and moved exactly once into the remaining empty target spots. 

Therefore, the minimum number of moves is:
```text
minimum_moves(n) = Total Coins - Maximum Overlap
```

By sliding the inverted triangle down by $k = \lfloor(n-1)/3\rfloor$ rows, the overlap is maximized. Using algebraic simplifications for the sum of the overlap, the problem reduces to a compact closed-form expression:
```c
minimum_moves(n) = floor(n * (n + 1) / 6)
```

### ⏱️ Complexity Profile
| Metric | Rating | Description |
| :--- | :---: | :--- |
| **Time Complexity** | $O(1)$ | Evaluates the closed-form mathematical expression, executing in constant time regardless of how astronomically large $n$ becomes. |
| **Space Complexity** | $O(1)$ | Only a few primitive variables are allocated for the computation. |

> **Note:** The code also includes an extensive validation suite that compares the $O(1)$ closed-form formula against an $O(n)$ row-by-row simulation loop and precomputed known ground-truth values from OEIS A007997.

<br>

## 📊 Graph & Complexity Analysis
> 📈 **File Reference:** `coin_triangle_complexity_analysis.png`

The accompanying graph visualizes the extreme efficiency and performance of the solver.

It demonstrates the profound advantage of the $O(1)$ mathematical approach compared to traditional looping or simulation methods. While an iterative simulation's execution time grows linearly (or worse) with $n$, the $O(1)$ approach remains **completely flat**, showing **zero degradation in performance** even for inputs up to $10^{12}$. The graph plotting $n$ versus computational time highlights this flawless constant-time behavior perfectly.

---
<div align="center">
  <i>Algorithm Design & Analysis - Week 7</i>
</div>
