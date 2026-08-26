# Q4: Heap Sort

This folder contains an optimized C implementation of **Heap Sort** designed to sort N randomly generated elements stored in a file.

## Detailed Description
Heap Sort is a comparison-based sorting algorithm that operates by first organizing the data into a binary max-heap structure and then repeatedly extracting the largest element to build the sorted array from the back. It offers a strong, guaranteed worst-case time complexity of **O(N log N)** and operates entirely in-place, meaning it requires only O(1) auxiliary space beyond the input array.

This specific implementation includes a major optimization: an **iterative heapify** function. The traditional recursive approach to sifting elements down the heap incurs function-call overhead and consumes recursion stack space proportional to the height of the tree (O(log N)). By replacing the recursion with an iterative `while` loop, this program avoids potential stack overflows and significantly reduces execution time, which is especially critical when processing very large values of N.

The program workflow is as follows:
1. It prompts the user for the number of elements (N).
2. It generates N random integers and stores them in a single-column CSV file (`input.csv`).
3. It reads the elements back from the file into an array in memory.
4. It performs Phase 1 (Build Heap) in O(N) time, iterating backward from the last internal node.
5. It performs Phase 2 (Extraction) in O(N log N) time to fully sort the array.
6. It writes the final sorted output back to disk into `sorted_output.csv`.

## Files
- `q4_heap_sort.c`: The C source code implementing the iterative, in-place Heap Sort.
- Data files: `input.csv` (the randomly generated input) and `sorted_output.csv` (the final sorted output).
