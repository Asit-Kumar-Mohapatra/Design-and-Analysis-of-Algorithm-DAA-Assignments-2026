# Q2 — Defective Coin Search

## Explanation of Question
You are given **N coins** that all appear identical, but exactly one coin is defective — it is slightly lighter than all the genuine coins. You are provided with a physical balance scale that can only tell you *which side is heavier* (or if the two sides are perfectly balanced). 

The goal is to find the defective coin using at most `⌈log₂(N)⌉` weighing operations, implementing this in C using a Divide and Conquer approach.

**The Strategy:**
A balance scale is an excellent real-world implementation of ternary (3-way) branching logic. When you weigh two equal groups of coins:
1. **Left is lighter:** The defective coin must be in the left group.
2. **Right is lighter:** The defective coin must be in the right group.
3. **Balanced:** The defective coin is in neither group (meaning it must be among the coins left off the scale).

By continuously dividing the search space into equal halves (setting aside an extra coin if the total count is odd), each weighing guarantees that we eliminate at least 50% of the remaining suspects.

## Algorithmic & Mathematical Analysis
### Physical Weighings Complexity
At each step, we perform exactly **1 weighing operation** and divide the problem space in half.
- **Recurrence Relation:** `W(n) = W(n/2) + 1`
- Applying the **Master Theorem** (where `a=1, b=2, f(n)=1`): `log_b(a) = log_2(1) = 0`. Since `f(n) = O(n^0)`, we fall into Case 2.
- **Total Physical Weighings:** `W(n) = Θ(log₂ n)`.

**Information-Theoretic Lower Bound:**
Every weighing yields 3 possible outcomes. After `k` weighings, we can distinguish between `3^k` states. To find 1 coin out of `N`, we need `3^k >= N`, meaning mathematically we only need `⌈log₃(N)⌉` weighings if we divide into perfect thirds. However, dividing into halves yields a `⌈log₂(N)⌉` bound which is achieved by this specific binary implementation.

### CPU Computational Time
While the physical weighings are `O(log n)`, our programmatic simulation of the scale (`weighScale()`) requires iterating over the array to sum the weights.
- Each `weighScale()` call takes `O(n)` CPU time (iterating over `n/2` elements).
- **Recurrence Relation:** `T(n) = T(n/2) + O(n)`
- **CPU Time Complexity:** `O(n log n)`
- **Space Complexity:** `O(1)` as it is implemented iteratively without recursion stacks.

## C Code Explanation
### `weighScale()` — The Scale Simulator
This function acts as the physical scale. It sums the weights of two subarrays and returns `-1` (left lighter), `1` (right lighter), or `0` (balanced).
```c
int weighScale(int *coins, int lStart, int lEnd, int rStart, int rEnd) {
    long long weightLeft = 0, weightRight = 0;
    for (int i = lStart; i < lEnd; i++) weightLeft  += coins[i];
    for (int i = rStart; i < rEnd; i++) weightRight += coins[i];
    if (weightLeft  < weightRight) return -1;  
    if (weightRight < weightLeft)  return  1;  
    return 0;                                   
}
```

### `findDefectiveCoin()` — Core Divide & Conquer Logic
```c
int start = 0, end = n;
bool defectGuaranteed = false;

while ((end - start) > 1) {
    int count = end - start;
    int mid   = count / 2;
    // Weigh the two equal halves
    int result = weighScale(coins, start, start+mid, start+mid, start+mid*2);

    if (result == -1) { end = start + mid; defectGuaranteed = true; } // Left lighter
    else if (result == 1) { start += mid;  defectGuaranteed = true; } // Right lighter
    else {
        // Balanced. If even, no defective coin exists here.
        if (count % 2 == 0) return -1;   
        else {
            // Odd count: check the one set-aside coin
            int extraIdx = end - 1;
            if (weighScale(coins, extraIdx, extraIdx+1, start, start+1) == -1)
                return extraIdx;
            return -1;
        }
    }
}
if ((end - start) == 1 && defectGuaranteed) return start;
return -1;
```
*Note the odd/even handling: If we have an odd number of coins, we divide `N-1` coins into two halves and leave 1 coin off the scale. If the scale balances, that leftover coin is our prime suspect.*

## Graph Analysis
A graphical analysis of this algorithm plots the number of physical weighings against `N`. 
- The resulting curve perfectly maps to a base-2 logarithmic curve (`y = log₂(x)`).
- The CPU simulation time would plot as an `O(N log N)` curve, demonstrating the divergence between the theoretical physical operations (weighing) and the programmatic simulation overhead (array summing).
