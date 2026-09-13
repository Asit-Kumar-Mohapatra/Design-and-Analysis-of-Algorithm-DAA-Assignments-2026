<div align="center">
  
# 🎛️ Security Switches
*An elegant $O(1)$ closed-form solver for the Chinese Rings configuration.*

---
</div>

## 📖 Problem Description
> You are faced with a security panel containing $n$ switches, all initially in the ON position. You want to turn every switch OFF. 

However, the switches are interlocked with the following rules:
1. The rightmost switch (switch $n$) may be toggled freely at any time.
2. Any other switch $i$ (where $i < n$) may be toggled **if and only if** switch $i+1$ is ON, and every switch to the right of $i+1$ is completely OFF.

You must determine the **absolute minimum number of moves** (toggles) required to transition the entire panel from the all-ON state to the all-OFF state.

<br>

## ⚙️ Code Explanation
The C code derives and implements a mathematically closed-form solution to achieve flawless $O(1)$ performance, avoiding any recursive or graph-searching overhead.

### 💡 The Core Logic
This mechanical configuration is isomorphic to the classic **"Chinese Rings" (Baguenaudier)** puzzle. The state transitions form a reflected binary Gray code path.

By analyzing the state transitions, freeing switch $n-1$ requires recursively solving the $n-2$ sub-puzzle twice, yielding the linear recurrence:
```text
M(n) = M(n-1) + 2M(n-2) + 1    (with M(1)=1, M(2)=2)
```

Solving the characteristic equation $x^2 - x - 2 = 0$ provides the roots $2$ and $-1$. Fitting the base cases provides the exact closed-form solution:
$$M(n) = \frac{2^{n+1}}{3} - \frac{(-1)^n}{6} - \frac{1}{2}$$

Because $2^{n+1} \pmod 3$ cycles elegantly, the alternating and constant terms can be subsumed by integer floor division, resulting in the astonishingly simple integer formula:
```c
M(n) = floor( 2^(n+1) / 3 )
```

### ⏱️ Complexity Profile
| Metric | Rating | Description |
| :--- | :---: | :--- |
| **Time Complexity** | $O(1)$ | The result is obtained with a single bit-shift and division operation. |
| **Space Complexity** | $O(1)$ | No arrays, recursion, or dynamic memory are needed. |

> **Hardware Push:** To push the hardware to its absolute limits, the code utilizes `uint64_t` for $n \le 62$, and the GCC/Clang compiler extension `unsigned __int128` to calculate exact results up to $n = 126$ in constant time. 

<br>

## 📊 Graph & Complexity Analysis
> 📈 **File Reference:** `security_switches_complexity_analysis.png`

The graph contrasts the closed-form $O(1)$ computation time against the $O(2^n)$ state space it represents.

The graph visually asserts that while the number of physical moves required to solve the puzzle explodes exponentially as $2^n$, the algorithmic time to *calculate* that number remains totally flat at $O(1)$. It highlights the profound power of solving recurrence relations algebraically rather than iterating through states via Breadth-First Search (BFS) or recursive simulation.

---
<div align="center">
  <i>Algorithm Design & Analysis - Week 7</i>
</div>
