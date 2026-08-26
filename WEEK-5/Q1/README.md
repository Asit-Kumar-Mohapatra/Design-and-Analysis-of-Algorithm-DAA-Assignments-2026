# Q1: Finding the Median

This folder contains a C implementation for finding the median of an unsorted array using the **Median of Medians** approach. 

## Detailed Description
Finding the median or any order statistic using standard Quickselect has an average time complexity of O(N) but can degrade to O(N²) in the worst case if bad pivots are consistently chosen. To mitigate this, this implementation uses the deterministic Median of Medians algorithm.

The algorithm works by dividing the input array into smaller groups of 5 elements each. It sorts each of these small groups using a simple Insertion Sort to find their respective medians. It then recursively finds the median of these medians to use as the pivot for the partitioning step. By doing so, it guarantees that the chosen pivot is relatively close to the true median, ensuring a reasonably balanced partition at every step. This strict balancing provides a guaranteed worst-case time complexity of **O(N)**, making it asymptotically optimal since any correct algorithm must inspect every element at least once (an Ω(N) lower bound). The extra space complexity is bounded at O(log N) due to the recursive stack depth.

## Files
- `q1_finding_median.c`: The C source code implementing the deterministic O(N) median finding algorithm.
