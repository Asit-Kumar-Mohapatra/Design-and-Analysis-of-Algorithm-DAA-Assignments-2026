<div align="center">
  
# 🗼 Reve's Puzzle (4-Peg Tower of Hanoi)
*Sub-exponential dynamic programming with the Frame-Stewart Algorithm.*

---
</div>

## 📖 Problem Description
> Reve's Puzzle is a generalization of the classic Tower of Hanoi. The objective is to move $n$ disks of varying sizes from a starting peg to a destination peg, but instead of the standard 3 pegs, you have **4 pegs** at your disposal. 

The standard Hanoi rule applies strictly: **you can never place a larger disk on top of a smaller one at any time.**

The specific challenge is to find the minimum number of moves to solve the puzzle for $n$ disks (famously taking exactly 33 moves for $n=8$) and to algorithmically generalize this generation for any $n$.

<br>

## ⚙️ Code Explanation
The C code uses the **Frame-Stewart Algorithm** alongside dynamic programming (DP) to find the absolute minimum number of moves and physically generate the required sequence.

### 💡 The Core Logic
The Frame-Stewart principle for moving $n$ disks using 4 pegs is:
1. Pick an **optimal split point** $k$ (where $1 \le k < n$).
2. Move the top $k$ disks to a spare peg, utilizing all 4 pegs. This requires $M(k)$ moves recursively.
3. Move the remaining $(n-k)$ disks to the destination peg. Since one spare peg is occupied by the smaller disks, you can only use 3 pegs for this step. Moving $x$ disks on 3 pegs takes exactly $2^x - 1$ moves.
4. Move the $k$ disks from the spare peg onto the destination peg, again utilizing all 4 pegs. This takes another $M(k)$ moves.

The optimal recurrence relation is thus:
```text
M(n) = min { 2 × M(k) + 2^(n-k) - 1 }  for 1 ≤ k < n
```
*(Base cases: $M(0) = 0$, $M(1) = 1$)*

To supercharge the DP, the code leverages the mathematical fact that the optimal split $k$ always satisfies $(n-k) = O(\sqrt{2n})$. Because of this, it only needs to check a small window of the last 63 values of $k$ instead of iterating all the way from 1. 

### ⏱️ Complexity Profile
| Metric | Rating | Description |
| :--- | :---: | :--- |
| **Time Complexity** | $O(n)$ | To build the DP table, we only check a constant bounded range (max 63 candidates) of previous $k$ values for each $n$. |
| **Space Complexity** | $O(n)$ | For storing the `dp` and `bestK` lookup tables. |
| **Generation Complexity** | $\Theta(2^{\sqrt{2n}})$ | Generating the actual move list takes $O(M(n))$ time and space. |

> **Note:** The program includes a peg simulation engine that structurally validates the legality of every generated move live, guaranteeing that no larger disk is ever placed on a smaller one.

<br>

## 📊 Graph & Complexity Analysis
> 📈 **File Reference:** `reves_puzzle_analysis.png`

The graph illustrates the sub-exponential growth rate of the Frame-Stewart algorithm.

Unlike the standard 3-peg Tower of Hanoi, which scales purely exponentially as $O(2^n)$, Reve's Puzzle scales at $O(2^{\sqrt{2n}})$. The graph visually plots the number of moves against the number of disks $n$, showing a curve that bends noticeably slower than a strict exponential curve. It confirms that the introduction of a 4th peg provides a profound mathematical advantage, reducing the problem's complexity class entirely.

---
<div align="center">
  <i>Algorithm Design & Analysis - Week 7</i>
</div>
