# Q3 — Min & Max Finder

## Explanation of Question
Given an unsorted array of **N integers**, the objective is to find both the **minimum** and **maximum** elements simultaneously using a Divide and Conquer strategy. 

While finding just the max or just the min is trivial, finding both in the most optimized way is a classic algorithm design problem. The goal is to minimize the absolute total number of key comparisons to exactly `⌈3N/2⌉ - 2`, which is mathematically proven to be the theoretical minimum for this problem.

**The Naive Approach (Two Passes):**
- Pass 1: Scan all N elements for the maximum → requires `N−1` comparisons.
- Pass 2: Scan all N elements for the minimum → requires `N−1` comparisons.
- **Total:** `2N − 2` comparisons.

**The Divide & Conquer Approach (Pair Peeling):**
Instead of scanning everything twice, we process elements in pairs. 
1. We take two elements and compare them to each other (1 comparison).
2. The larger of the two is compared against the global maximum (1 comparison).
3. The smaller of the two is compared against the global minimum (1 comparison).
- **Total:** 3 comparisons for every 2 elements, yielding the `3N/2` ratio!

## Algorithmic & Mathematical Analysis
### Recurrence Relation
Let `T(N)` be the number of comparisons to find the min and max in an array of size `N`. Using the pair-peeling method:
- **Base Case 1:** `T(1) = 0` (1 element is both min and max, 0 comps)
- **Base Case 2:** `T(2) = 1` (Compare the two elements against each other, 1 comp)
- **Recursive Step:** `T(N) = T(N-2) + 3` (Extract a pair, 1 internal comp + 2 global merge comps).

**Solving the Recurrence by unrolling:**
`T(N) = T(N-2) + 3`
`T(N) = 3 * (N/2 - 1) + 1 = 3N/2 - 2`

Accounting for odd `N`, the exact formula is `⌈3N/2⌉ - 2`.
For example, if `N = 1000`:
- Naive approach takes `2(1000) - 2 = 1998` comparisons.
- D&C approach takes `1.5(1000) - 2 = 1498` comparisons.
This translates to a direct **25% reduction** in comparison operations!

### Information-Theoretic Bound
To establish a global max, every element except the max must "lose" a comparison. To establish a min, every element except the min must "win" a comparison. That is `N-1` wins and `N-1` losses needed. By comparing elements in pairs first, a single comparison yields both a "win" and a "loss", doubling the information gained per comparison and achieving the theoretical lower bound.

## C Code Explanation
### `getMinMaxDNC()` Function
```c
void getMinMaxDNC(int *arr, int low, int high, int *min, int *max, long long *cmp) {
    int n = high - low + 1;

    // Base Case 1
    if (n == 1) { *min = *max = arr[low]; return; }

    // Base Case 2
    if (n == 2) {            
        (*cmp)++;
        if (arr[low] > arr[high]) { *max = arr[low];  *min = arr[high]; }
        else                      { *max = arr[high]; *min = arr[low];  }
        return;
    }

    // Peel the front pair — 1 comparison internally
    int pA = arr[low], pB = arr[low+1], pMax, pMin;
    (*cmp)++;
    if (pA > pB) { pMax = pA; pMin = pB; } else { pMax = pB; pMin = pA; }

    // Recurse on remaining N-2 elements
    int rMin, rMax;
    getMinMaxDNC(arr, low+2, high, &rMin, &rMax, cmp);

    // Merge step — 2 comparisons
    (*cmp)++; *max = (pMax > rMax) ? pMax : rMax;  // merge max
    (*cmp)++; *min = (pMin < rMin) ? pMin : rMin;  // merge min
}
```
*Notice how the recursion aggressively reduces the array by 2 elements at a time, completely avoiding the standard `T(N/2)` tree in favor of a linear `T(N-2)` tail-recursive-style descent.*

## Graph Analysis
The graphical analysis plots the absolute comparison count against `N` for both approaches:
- Both algorithms exhibit `O(N)` linear growth, meaning they appear as straight lines on the graph.
- The **D&C line** runs strictly below the **Naive line**, with a visibly shallower slope.
- The gap between the lines represents the 25% computational savings. While asymptotically both are `O(N)`, this graph perfectly demonstrates how Big-O notation can hide massive constant-factor performance differences that are crucial in HPC and embedded systems.
