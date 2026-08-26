# Q2: Finding the K-th Smallest Element

This folder contains a C implementation for finding the K-th smallest element in an unsorted array using the **Median of Medians** selection algorithm.

## Detailed Description
The task of finding the K-th smallest (or largest) element in a dataset is a classic selection problem. While simply sorting the array and picking the K-th element takes O(N log N) time, this implementation achieves a guaranteed linear worst-case time complexity of **O(N)**. 

It accomplishes this by using the deterministic Median of Medians approach to select a high-quality pivot. The array is conceptually divided into chunks of 5 elements. The median of each chunk is found, and then the median of those medians is recursively determined. This final value is used to partition the array. Because the pivot is guaranteed to be greater than at least 30% of the elements and less than at least 30% of the elements, the search space is significantly reduced in every recursive step. The algorithm asks the user for the total number of elements N, takes the array elements as input, and finally requests the value of K (where 1 <= K <= N) to locate and return the exact K-th smallest item efficiently.

## Files
- `q2_finding_kth_smallest_element.c`: The C source code implementing the linear time selection algorithm to find the K-th smallest element.
