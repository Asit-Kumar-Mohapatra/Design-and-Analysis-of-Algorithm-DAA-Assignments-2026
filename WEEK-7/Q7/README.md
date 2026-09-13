<div align="center">
  
# ✖️ Matrix Chain Multiplication (MCM)
*Solving exponential permutations with $O(n^3)$ Interval Dynamic Programming.*

---
</div>

## 📖 Problem Description
> You are given a sequence (chain) of $n$ matrices to be multiplied: $A_1 \times A_2 \times \dots \times A_n$. The dimensions of these matrices are provided in an array $p$ of size $n+1$, where matrix $A_i$ has dimensions $p[i-1] \times p[i]$.

Matrix multiplication is associative, meaning the final matrix product is exactly the same no matter how you parenthesize the chain. However, the **number of scalar multiplications** required to compute the final product varies wildly depending on the chosen parenthesization. The goal is to determine the optimal parenthesization that minimizes the total number of scalar multiplications.

<br>

## ⚙️ Code Explanation
The C code solves this classic optimization problem using **Interval Dynamic Programming (DP)**.
Attempting to check every possible full parenthesization is futile, as the number of combinations follows the Catalan numbers, scaling exponentially. 

### 💡 The Core Logic
The DP algorithm leverages **optimal substructure**. Any parenthesization of $A_i \dots A_j$ must have a final multiplication that splits the chain at some point $k$ (where $i \le k < j$).
The cost to merge the two resulting sub-products is $p[i-1] \times p[k] \times p[j]$. 

The recurrence relation is:
```text
m[i][j] = min { m[i][k] + m[k+1][j] + p[i-1] × p[k] × p[j] }  for i ≤ k < j
```
*(Base cases: $m[i][i] = 0$, as a single matrix requires no multiplications).*

The algorithm constructs the solution by iterating over increasing chain lengths $L$, from $L=2$ up to $n$. This guarantees that when calculating $m[i][j]$, the costs of all smaller constituent sub-chains are already known. An auxiliary table `s[i][j]` tracks the optimal split point $k$, allowing the optimal parenthesization string to be reconstructed recursively at the end.

### ⏱️ Complexity Profile
| Metric | Rating | Description |
| :--- | :---: | :--- |
| **Time Complexity** | $O(n^3)$ | There are $O(n^2)$ subproblem states $(i, j)$ to solve. For each state, we iterate through $O(n)$ possible split points $k$. |
| **Space Complexity** | $O(n^2)$ | Two tables of size $(n+1) \times (n+1)$ are required: the cost table `m` and the split point table `s`. |

> **Note:** The program verifies the DP solution against a brute-force recursive generator for small $n$ and cross-references textbook examples (like CLRS) to ensure absolute accuracy.

<br>

## 📊 Graph & Complexity Analysis
> 📈 **File Reference:** `mcm_complexity_analysis.png`

The visualization validates the $O(n^3)$ behavior of the dynamic programming approach.

The graph charts execution time against the number of matrices $n$. It visually confirms that the computational time scales cubically ($n^3$), maintaining a flat ratio when elapsed time is divided by $n^3$. It emphasizes that while cubic time limits the max manageable $n$ compared to linear algorithms, it is a spectacular improvement over the naive exponential $O(4^n/n^{1.5})$ Catalan growth, transforming an unsolvable combinatorics problem into a highly tractable one.

---
<div align="center">
  <i>Algorithm Design & Analysis - Week 7</i>
</div>
