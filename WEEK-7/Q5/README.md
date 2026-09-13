<div align="center">
  
# 🎯 Hitting a Moving Target
*An optimized $O(n)$ deterministic sweep strategy.*

---
</div>

## 📖 Problem Description
> You are given $n$ hiding spots arranged in a straight line, numbered $1$ to $n$. A target is hiding in one of the spots. You can shoot exactly one spot per turn. However, the target is moving: between any two consecutive shots, the target **must** move to an adjacent spot. 

You never observe the target. The objective is to design a fixed, deterministic sequence of shots that is guaranteed to hit the target eventually, regardless of its starting position or adversarial movement strategy.

<br>

## ⚙️ Code Explanation
The C code implements the optimal deterministic sweep strategy based on a parity argument, guaranteeing a hit in linear time.

### 💡 The Core Logic
Because the target moves by exactly $\pm 1$ spot every turn on the path graph, its position flips between odd and even parity on every single move. If you know its starting parity, you know its parity at every future turn.

The algorithm uses a **converging sweep** strategy to trap the target:
1. **Sweep Right:** Shoot spots $2, 3, 4, \dots, n-1$. 
   If the target started on an even spot, the difference between the target's position and the shot position never increases. It is bounded, and since the path ends at $n$, the target is forced to turn back. Once it turns, the difference strictly shrinks by 2 each turn, guaranteeing a hit before the sweep ends.
2. **Sweep Left:** Shoot spots $n-1, n-2, \dots, 2$. 
   This is the mirror image. It guarantees a hit on any target that started on an odd spot.

By concatenating the two sweeps: `2, 3, 4, ..., n-1, n-1, n-2, ..., 3, 2`
The total number of shots required is $(n-2) + (n-2) = 2n - 4$ for $n \ge 3$. 

### ⏱️ Complexity Profile
| Metric | Rating | Description |
| :--- | :---: | :--- |
| **Time Complexity** | $O(n)$ | The sequence length is linearly proportional to $n$, taking $2n - 4$ operations to generate. |
| **Space Complexity** | $O(n)$ | The sequence array requires contiguous memory proportional to $n$. |

> **Verification Engine:** The program doesn't just print the sequence; it includes an exhaustive dynamic programming verification engine. This engine mathematically validates the $O(n)$ strategy by analyzing every possible starting spot and every possible adversarial evasion path in $O(n^2)$ time, proving absolutely that evasion is impossible.

<br>

## 📊 Graph & Complexity Analysis
> 📈 **File Reference:** `time_complexity_analysis.png`

The graph illustrates the sequence length scaling linearly as $O(n)$ compared to the number of spots $n$.

It effectively visualizes why a simple linear sweep operates perfectly and predictably. Even though the number of possible evasion paths the target could theoretically take branches exponentially over time, the deterministic linear sweep strategy collapses that possibility space entirely within just $2n-4$ fixed steps, as represented by the linear growth on the graph.

---
<div align="center">
  <i>Algorithm Design & Analysis - Week 7</i>
</div>
