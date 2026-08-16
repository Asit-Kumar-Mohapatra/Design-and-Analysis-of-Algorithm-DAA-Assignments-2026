#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Hybrid Threshold: Small subproblems use hardware-optimized loop multiplication
#define THRESHOLD 16

// Function Prototypes
bool isPowerOfTwo(int n);
void printMatrix(const int* matrix, int n);
void printExplanation(int n);

/**
 * OPTIMIZATION 1: Zero-Copy Matrix Addition with Restrict Pointers
 * 'restrict' tells the compiler pointers do not overlap, enabling SIMD vectorization.
 */
static inline void addMatrixStrided(const int* restrict A, int lda,
                                    const int* restrict B, int ldb,
                                    int* restrict C, int ldc, int n) {
    for (int i = 0; i < n; i++) {
        const int* aRow = A + i * lda;
        const int* bRow = B + i * ldb;
        int* cRow = C + i * ldc;
        for (int j = 0; j < n; j++) {
            cRow[j] = aRow[j] + bRow[j];
        }
    }
}

/**
 * Zero-Copy Matrix Subtraction with Restrict Pointers
 */
static inline void subMatrixStrided(const int* restrict A, int lda,
                                    const int* restrict B, int ldb,
                                    int* restrict C, int ldc, int n) {
    for (int i = 0; i < n; i++) {
        const int* aRow = A + i * lda;
        const int* bRow = B + i * ldb;
        int* cRow = C + i * ldc;
        for (int j = 0; j < n; j++) {
            cRow[j] = aRow[j] - bRow[j];
        }
    }
}

/**
 * OPTIMIZATION 2: Cache-Line & SIMD Optimized Base Case (i-k-j Loop Order)
 * Operates on strided 1D arrays without submatrix copying.
 */
void standardMultiplyStrided(const int* restrict A, int lda,
                            const int* restrict B, int ldb,
                            int* restrict C, int ldc, int n) {
    for (int i = 0; i < n; i++) {
        int* cRow = C + i * ldc;
        for (int j = 0; j < n; j++) {
            cRow[j] = 0;
        }
        for (int k = 0; k < n; k++) {
            int r = A[i * lda + k]; // Cached in CPU register
            const int* bRow = B + k * ldb;
            for (int j = 0; j < n; j++) {
                cRow[j] += r * bRow[j];
            }
        }
    }
}

/**
 * OPTIMIZATION 3: Zero-Allocation Recursive Strassen
 * Uses submatrix offsets instead of copying, and slices workspace from a pre-allocated buffer.
 */
void strassenRecursive(const int* restrict A, int lda,
                      const int* restrict B, int ldb,
                      int* restrict C, int ldc,
                      int n, int* restrict workspace) {
    // Base Case Cutoff
    if (n <= THRESHOLD) {
        standardMultiplyStrided(A, lda, B, ldb, C, ldc, n);
        return;
    }

    int k = n / 2;
    int kSquare = k * k;

    // Slice intermediate buffers from the pre-allocated workspace scratchpad
    int* M1 = workspace;
    int* M2 = M1 + kSquare;
    int* M3 = M2 + kSquare;
    int* M4 = M3 + kSquare;
    int* M5 = M4 + kSquare;
    int* M6 = M5 + kSquare;
    int* M7 = M6 + kSquare;
    int* t1 = M7 + kSquare;
    int* t2 = t1 + kSquare;
    int* nextWorkspace = t2 + kSquare; // Remaining scratchpad passed to deeper recursion

    // ZERO-COPY QUADRANT VIEWS: Offset calculations using matrix stride (lda, ldb)
    const int* A11 = A;                const int* A12 = A + k;
    const int* A21 = A + k * lda;      const int* A22 = A + k * lda + k;

    const int* B11 = B;                const int* B12 = B + k;
    const int* B21 = B + k * ldb;      const int* B22 = B + k * ldb + k;

    // Strassen's 7 Recursive Formulas
    addMatrixStrided(A11, lda, A22, lda, t1, k, k);
    addMatrixStrided(B11, ldb, B22, ldb, t2, k, k);
    strassenRecursive(t1, k, t2, k, M1, k, k, nextWorkspace);

    addMatrixStrided(A21, lda, A22, lda, t1, k, k);
    strassenRecursive(t1, k, B11, ldb, M2, k, k, nextWorkspace);

    subMatrixStrided(B12, ldb, B22, ldb, t2, k, k);
    strassenRecursive(A11, lda, t2, k, M3, k, k, nextWorkspace);

    subMatrixStrided(B21, ldb, B11, ldb, t2, k, k);
    strassenRecursive(A22, lda, t2, k, M4, k, k, nextWorkspace);

    addMatrixStrided(A11, lda, A12, lda, t1, k, k);
    strassenRecursive(t1, k, B22, ldb, M5, k, k, nextWorkspace);

    subMatrixStrided(A21, lda, A11, lda, t1, k, k);
    addMatrixStrided(B11, ldb, B12, ldb, t2, k, k);
    strassenRecursive(t1, k, t2, k, M6, k, k, nextWorkspace);

    subMatrixStrided(A12, lda, A22, lda, t1, k, k);
    addMatrixStrided(B21, ldb, B22, ldb, t2, k, k);
    strassenRecursive(t1, k, t2, k, M7, k, k, nextWorkspace);

    // Reconstruct C matrix quadrants directly using strided pointers
    int* C11 = C;                int* C12 = C + k;
    int* C21 = C + k * ldc;      int* C22 = C + k * ldc + k;

    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) {
            int idx = i * k + j;
            C11[i * ldc + j] = M1[idx] + M4[idx] - M5[idx] + M7[idx];
            C12[i * ldc + j] = M3[idx] + M5[idx];
            C21[i * ldc + j] = M2[idx] + M4[idx];
            C22[i * ldc + j] = M1[idx] - M2[idx] + M3[idx] + M6[idx];
        }
    }
}

/**
 * Top-level API Wrapper allocating the single scratchpad workspace
 */
void strassenMultiply(const int* A, const int* B, int* C, int n) {
    // Total scratchpad size formula for worst-case recursive workspace
    size_t workspaceSize = 0;
    for (int tempN = n / 2; tempN >= THRESHOLD; tempN /= 2) {
        workspaceSize += 9 * (size_t)tempN * tempN;
    }
    if (workspaceSize == 0) workspaceSize = 1; // Fallback for small n

    int* workspace = (int*)calloc(workspaceSize, sizeof(int));
    if (!workspace) {
        fprintf(stderr, "[Fatal Error]: Scratchpad allocation failed!\n");
        exit(EXIT_FAILURE);
    }

    strassenRecursive(A, n, B, n, C, n, n, workspace);

    free(workspace);
}

int main() {
    int n;

    printf("=====================================================\n");
    printf("  STRASSEN'S MATRIX MULTIPLICATION (HPC PRODUCTION)  \n");
    printf("=====================================================\n\n");

    printf("Enter matrix dimension n: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        fprintf(stderr, "\n[Error]: Invalid dimension! Must be a positive integer.\n");
        return EXIT_FAILURE;
    }

    // Validation Check: Must be a power of 2
    if (!isPowerOfTwo(n)) {
        printf("\n=====================================================\n");
        printf("[NOT POSSIBLE]: Strassen's multiplication cannot be performed!\n");
        printf("Reason: Dimension n = %d is odd or not a power of 2.\n", n);
        printf("Standard Divide & Conquer Strassen requires n = 2^k (e.g., 2, 4, 8, 16...)\n");
        printf("to split matrices into equal integer-sized quadrants.\n");
        printf("=====================================================\n\n");

        printExplanation(n);
        return EXIT_SUCCESS;
    }

    // Allocate 1D contiguous arrays for optimal memory layout
    int* A = (int*)calloc((size_t)n * n, sizeof(int));
    int* B = (int*)calloc((size_t)n * n, sizeof(int));
    int* C = (int*)calloc((size_t)n * n, sizeof(int));

    if (!A || !B || !C) {
        fprintf(stderr, "[Fatal Error]: Matrix allocation failed!\n");
        return EXIT_FAILURE;
    }

    printf("\nEnter elements for Matrix A (%dx%d):\n", n, n);
    for (int i = 0; i < n * n; i++) {
        if (scanf("%d", &A[i]) != 1) {
            fprintf(stderr, "\n[Error]: Invalid matrix element!\n");
            free(A); free(B); free(C);
            return EXIT_FAILURE;
        }
    }

    printf("\nEnter elements for Matrix B (%dx%d):\n", n, n);
    for (int i = 0; i < n * n; i++) {
        if (scanf("%d", &B[i]) != 1) {
            fprintf(stderr, "\n[Error]: Invalid matrix element!\n");
            free(A); free(B); free(C);
            return EXIT_FAILURE;
        }
    }

    // Execute Production Strassen Multiplication
    strassenMultiply(A, B, C, n);

    // Output Result Matrix
    printf("\n--- RESULT MATRIX C (A * B) ---\n");
    printMatrix(C, n);

    // Output Theoretical Concepts and Complexity Analysis
    printExplanation(n);

    // Free resources
    free(A);
    free(B);
    free(C);

    return EXIT_SUCCESS;
}

bool isPowerOfTwo(int n) {
    return (n > 0) && ((n & (n - 1)) == 0);
}

void printMatrix(const int* matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%d\t", matrix[i * n + j]);
        }
        printf("\n");
    }
    printf("\n");
}

void printExplanation(int n) {
    printf("=====================================================\n");
    printf("     ALGORITHM CONCEPT & COMPLEXITY EXPLANATION      \n");
    printf("=====================================================\n");
    printf("1. CONCEPT & WORKING:\n");
    printf("   - Standard Divide & Conquer splits n x n matrices into\n");
    printf("     4 sub-quadrants of size (n/2) x (n/2), requiring 8 recursive\n");
    printf("     multiplications and 4 additions.\n");
    printf("   - Strassen's method uses linear algebra formulas to compute\n");
    printf("     7 recursive multiplications (M1 to M7) instead of 8,\n");
    printf("     at the expense of 18 matrix addition/subtraction operations.\n\n");

    printf("2. RECURRENCE RELATION:\n");
    printf("   - Base Case n <= %d : T(n) = O(n^3) [SIMD-Vectorized i-k-j Multiplication]\n", THRESHOLD);
    printf("   - Recursive n > %d   : T(n) = 7 * T(n/2) + O(n^2)\n", THRESHOLD);
    printf("     where O(n^2) represents the 18 submatrix add/subtract steps.\n\n");

    printf("3. TIME COMPLEXITY DERIVATION:\n");
    printf("   - Solved via Master Theorem [T(n) = a*T(n/b) + f(n)]:\n");
    printf("     a = 7 (subproblems), b = 2 (division factor), f(n) = O(n^2)\n");
    printf("   - Critical Exponent: log_b(a) = log_2(7) ≈ 2.80738\n");
    printf("   - Since f(n) = O(n^2) < O(n^2.807), Case 1 applies:\n");
    printf("     Time Complexity = O(n^(log_2 7)) ≈ O(n^2.807)\n");
    printf("   - Standard Multiplication : O(n^3)\n");
    printf("   - Strassen's Improvement  : O(n^2.807) (Faster for large n)\n\n");

    printf("4. SPACE COMPLEXITY ANALYSIS:\n");
    printf("   - Call Stack Depth = O(log n)\n");
    printf("   - Dynamic Heap Memory = Pre-allocated O(n^2) scratchpad buffer\n");
    printf("   - Recursive Memory Allocations = 0 (Zero runtime heap locks)\n");
    printf("   - Overall Space Complexity = O(n^2)\n");
    printf("=====================================================\n");
}