# Q4 — Strassen's Matrix Multiplication

## Explanation of Question
The standard algorithm to multiply two N×N matrices involves 3 nested loops and requires `O(N^3)` operations. Even using a standard Divide and Conquer approach (splitting matrices into four `N/2 × N/2` quadrants) still yields 8 recursive multiplications, which mathematically simplifies back to `O(N^3)`.

Volker Strassen's breakthrough in 1969 was realizing that by strategically adding and subtracting the sub-matrices, the 8 necessary multiplications could be compressed into **just 7 recursive multiplications**. 

The task is to implement this complex algorithm in C, enforcing that N is a power of 2 (to allow perfect successive halving), and to prove via runtime analysis and the Master Theorem that this optimization fundamentally changes the asymptotic complexity.

## Algorithmic & Mathematical Analysis
### Master Theorem Proof
**Standard D&C Multiplication:**
- Recursion: `T(n) = 8·T(n/2) + O(n^2)`
- Master Theorem: `a=8, b=2`. Critical exponent `log₂(8) = 3`.
- Result: `O(n^3)` (No better than the naive loop!).

**Strassen's Multiplication:**
- Recursion: `T(n) = 7·T(n/2) + O(n^2)`
- Master Theorem: `a=7, b=2`. Critical exponent `log₂(7) ≈ 2.807`.
- Result: `O(n^2.807)`.

By saving just 1 multiplication at the expense of 18 matrix additions, the savings compound exponentially across deep recursion trees. For `N=1024`, standard multiplication takes ~1 billion operations, while Strassen's takes ~134 million — nearly **7.5x faster** theoretically.

### Space Complexity
Strassen's algorithm is notorious for being memory-heavy. At each recursive step, we must allocate space for 7 intermediate matrices (M1 to M7) and several addition/subtraction buffers. The recurrence for space is `S(n) = 7·S(n/2) + O(n^2)`, which tightly bounds to `O(N^2)`.

## C Code Explanation
Implementing Strassen in C requires intense memory management to be performant. Our implementation utilizes three key High-Performance Computing (HPC) optimizations:

1. **Hybrid Threshold:**
   When matrices get very small (e.g., `N <= 16`), the overhead of recursive function calls and memory allocation far outweighs the multiplication savings. The code falls back to a highly cache-optimized standard multiplication loop for the base cases.
   
2. **Zero-Copy Strided Views:**
   Instead of physically copying the quadrants into new arrays, we use pointer arithmetic and "strides" to view the existing matrix as 4 sub-matrices.
   ```c
   const int* A11 = A;
   const int* A12 = A + k;
   const int* A21 = A + k * lda;  // lda is the 'stride'
   const int* A22 = A + k * lda + k;
   ```

3. **Pre-allocated Workspace Buffer:**
   Dynamic memory allocation (`malloc()`) inside deep recursion causes massive heap fragmentation and latency. We pre-compute the total memory required across all levels and allocate it once at the start.
   ```c
   size_t workspaceSize = 0;
   for (tempN = n/2; tempN >= THRESHOLD; tempN /= 2)
       workspaceSize += 9 * tempN * tempN; // 7 Mi + 2 temp buffers
   int* workspace = (int*)calloc(workspaceSize, sizeof(int));
   ```

## Graph Analysis
The generated graph `strassen_matrix_multiplication_graph.png` plots **operation count vs matrix size N**.
- **The Crossover Point:** For very small `N` (typically < 64), the Standard O(N^3) algorithm actually sits *lower* on the graph. Strassen's matrix addition overhead dominates.
- **The Inflection:** Around `N=64` or `N=128`, the lines cross. 
- **The Divergence:** From `N=256` onward, the `O(N^3)` curve shoots aggressively upwards, while the `O(N^2.807)` Strassen curve rises much more gently. The gap between them visually confirms the immense power of sub-cubic algorithms at scale.
