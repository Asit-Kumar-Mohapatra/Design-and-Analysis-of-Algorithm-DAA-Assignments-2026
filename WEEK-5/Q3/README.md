# Q3: Quick Sort

This folder contains a highly optimized C implementation of the **Quick Sort** algorithm, designed to sort a large collection of N randomly generated elements stored in a file.

## Detailed Description
Standard Quick Sort algorithms can suffer from high overhead on small arrays, deep recursion stacks, and O(N²) worst-case performance on sorted or reverse-sorted data. This implementation focuses on real-world performance tuning and applies several advanced optimizations to mitigate these issues and achieve a robust **O(N log N)** time complexity.

Key optimizations applied in this implementation:
1. **Median-of-Three Pivot Selection**: Instead of picking the first or last element, it samples the first, middle, and last elements and uses their median as the pivot. This avoids the O(N²) worst-case on already sorted or organ-pipe inputs.
2. **Insertion Sort Cutoff**: Quick Sort has high constant overhead for tiny sub-arrays. This implementation switches to Insertion Sort when the partition size drops below a threshold (e.g., 10 elements), which is significantly faster for small datasets.
3. **Tail-Call Elimination**: It converts one of the recursive calls into an explicit `while` loop and always recurses on the smaller half. This strictly bounds the maximum recursion stack depth to O(log N) instead of a potential O(N).
4. **Hoare Partition Scheme**: Utilizes Hoare's two-pointer partition method, which performs approximately three times fewer swaps on average compared to the standard Lomuto scheme.
5. **Fast PRNG**: The standard `rand()` function is replaced by a fast `xorshift32` Pseudo-Random Number Generator, which provides better distribution and speed when generating the vast amounts of test data.
6. **Buffered File I/O**: Instead of slow, element-by-element `fprintf`/`fscanf` calls, it uses a large 1MB text buffer to drastically reduce the number of system calls when reading from and writing to the CSV files.

## Files
- `q3_quick_sort.c`: The C source code implementing the highly optimized Quick Sort algorithm.
- Data files: `random_data.csv` (the randomly generated input) and `sorted_data.csv` (the final sorted output).
