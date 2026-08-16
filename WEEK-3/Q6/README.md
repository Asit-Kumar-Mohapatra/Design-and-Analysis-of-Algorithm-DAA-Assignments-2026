# Q6 — Selection Sort & Loop Invariants

## Explanation of Question
The goal of this assignment is to implement the **Selection Sort** algorithm in C and, more importantly, formally prove its mathematical correctness using **Loop Invariants**. 

**Algorithm Concept:** Selection Sort divides the array into two logical parts: a sorted left half and an unsorted right half. In each pass, it scans the entire unsorted right half, finds the absolute minimum element, and swaps it with the leftmost unsorted element, thereby expanding the sorted half by one.

**Loop Invariant Concept:** A loop invariant is a logical condition or property that is rigorously proven to be:
1. True *before* the loop starts (**Initialization**).
2. True *after every iteration* of the loop (**Maintenance**).
3. Leading to a proof of correctness when the loop terminates (**Termination**).

## Algorithmic & Mathematical Analysis
### The Formal Loop Invariant Proof
**The Invariant:** "At the start of iteration `i` of the outer loop, the subarray `arr[0...i-1]` contains the `i` smallest elements of the entire array, sorted in ascending order."

1. **Initialization (Before `i=0`):** 
   The subarray `arr[0...-1]` is empty. An empty array trivially contains the 0 smallest elements in sorted order. The invariant is vacuously true.
2. **Maintenance (During iteration `i`):** 
   By the invariant assumption, `arr[0...i-1]` holds the `i` smallest elements sorted. The inner loop scans `arr[i...n-1]` to find its minimum element. This element must be the `i+1`-th smallest element overall. It is then swapped with `arr[i]`. Now, `arr[0...i]` contains the `i+1` smallest elements, in sorted order. The invariant is preserved for the start of iteration `i+1`.
3. **Termination (When `i=n-1`):** 
   The outer loop terminates after `n-1` iterations. According to the invariant, `arr[0...n-2]` now contains the `n-1` smallest elements in sorted order. Since all other `n-1` elements are smaller and correctly placed, the final single element at `arr[n-1]` must be the absolute largest element, naturally resting in its correct final position. Thus, the entire array `arr[0...n-1]` is perfectly sorted. Correctness is proven.

### Complexity Analysis
- **Time Complexity:** The outer loop runs `n-1` times. The inner loop runs `n-1-i` times. Summing this yields `(n-1) + (n-2) + ... + 1 = n(n-1)/2` exact comparisons. Since the inner loop always executes fully regardless of input order, the **Best, Worst, and Average cases are exactly identical at `Θ(n^2)`**.
- **Space Complexity:** `O(1)`. The array is sorted purely in-place, requiring only a single `temp` variable for the swap operation. No extra memory is allocated.

## C Code Explanation
```c
void selectionSort(int *arr, int n) {
    // Outer loop boundary: i runs up to n-2. 
    // n-1 passes are enough, last element falls into place automatically.
    for (int i = 0; i < n - 1; i++) {
        
        int min_idx = i; // Assume current boundary is the minimum
        
        // Inner loop: Scans the remaining unsorted portion
        for (int j = i + 1; j < n; j++) {
            if (arr[j] < arr[min_idx]) {
                min_idx = j; // Update tracker when new minimum is found
            }
        }
        
        // Swap the found minimum with the first unsorted element
        if (min_idx != i) {
            int temp = arr[i];
            arr[i] = arr[min_idx];
            arr[min_idx] = temp;
        }
    }
}
```
*Note the optimization `if (min_idx != i)`: This prevents redundant memory write operations when the element is already precisely where it needs to be, which is a common scenario in partially sorted data.*

## Graph Analysis
The `Selection_Sort_Complexity_Analysis.png` graph plots the number of comparisons against the input size `n`.
- Because Selection Sort never terminates early (unlike Bubble Sort or Insertion Sort), it always makes exactly `n(n-1)/2` comparisons. 
- As a result, if you were to plot the Best Case, Worst Case, and Average Case curves, they would all completely overlap as a single, identical parabolic line.
- This perfectly visualizes the `Θ(n^2)` tight bound, demonstrating that as `N` doubles, the processing time consistently quadruples, showcasing poor scalability for large datasets but high predictability for small, real-time systems.
