#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Function Prototypes
bool isPowerOfTwo(int n);
void printMatrix(const int* matrix, int n);
void printExplanation(int n);
void multiplySpecialRec(const int* A, int lda,
                       const int* B, int ldb,
                       int* C, int ldc,
                       int n, int* workspace);

/**
 * Checks if a dimension n is a power of 2 (n = 2^k).
 */
bool isPowerOfTwo(int n) {
    return (n > 0) && ((n & (n - 1)) == 0);
}

/**
 * Core $O(n^2)$ Divide-and-Conquer Recursive Algorithm
 * Uses zero dynamic heap allocations during recursion by leveraging a pre-allocated workspace.
 */
void multiplySpecialRec(const int* A, int lda,
                       const int* B, int ldb,
                       int* C, int ldc,
                       int n, int* workspace) {
    // Base Case: 1x1 Matrix
    if (n == 1) {
        C[0] = A[0] * B[0];
        return;
    }

    int k = n / 2;
    int kSquare = k * k;

    // Slice workspace buffers for intermediate computations
    int* S_A1 = workspace;                  // (A1 + A2)
    int* S_B1 = S_A1 + kSquare;            // (B1 + B2)
    int* P1   = S_B1 + kSquare;            // (A1 + A2)(B1 + B2)
    int* S_A2 = P1   + kSquare;            // (A1 - A2)
    int* S_B2 = S_A2 + kSquare;            // (B1 - B2)
    int* P2   = S_B2 + kSquare;            // (A1 - A2)(B1 - B2)
    int* nextWorkspace = P2 + kSquare;     // Remaining scratchpad passed deeper

    // Zero-copy submatrix pointer views
    const int* A1 = A;             // Top-Left block
    const int* A2 = A + k;         // Top-Right block
    const int* B1 = B;             // Top-Left block
    const int* B2 = B + k;         // Top-Right block

    // 1. Compute S_A1 = A1 + A2  and  S_B1 = B1 + B2
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) {
            S_A1[i * k + j] = A1[i * lda + j] + A2[i * lda + j];
            S_B1[i * k + j] = B1[i * ldb + j] + B2[i * ldb + j];
        }
    }

    // 2. Recursive Call 1: P1 = (A1 + A2)(B1 + B2)
    multiplySpecialRec(S_A1, k, S_B1, k, P1, k, k, nextWorkspace);

    // 3. Compute S_A2 = A1 - A2  and  S_B2 = B1 - B2
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) {
            S_A2[i * k + j] = A1[i * lda + j] - A2[i * lda + j];
            S_B2[i * k + j] = B1[i * ldb + j] - B2[i * ldb + j];
        }
    }

    // 4. Recursive Call 2: P2 = (A1 - A2)(B1 - B2)
    multiplySpecialRec(S_A2, k, S_B2, k, P2, k, k, nextWorkspace);

    // 5. Reconstruct C1 = (P1 + P2) / 2  and  C2 = (P1 - P2) / 2
    // Place results into C11, C12, C21, C22 quadrants directly
    int* C11 = C;                int* C12 = C + k;
    int* C21 = C + k * ldc;      int* C22 = C + k * ldc + k;

    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) {
            int idx = i * k + j;

            /*
             * Division by 2 is always exact (no integer truncation error).
             * Proof by algebraic expansion:
             *   P1 + P2 = (A1+A2)(B1+B2) + (A1-A2)(B1-B2)
             *           = 2*A1*B1 + 2*A2*B2  ->  always even
             *   P1 - P2 = (A1+A2)(B1+B2) - (A1-A2)(B1-B2)
             *           = 2*A1*B2 + 2*A2*B1  ->  always even
             * Therefore integer division by 2 produces the exact result.
             */
            int c1 = (P1[idx] + P2[idx]) / 2;  /* C1 = A1*B1 + A2*B2 */
            int c2 = (P1[idx] - P2[idx]) / 2;  /* C2 = A1*B2 + A2*B1 */

            C11[i * ldc + j] = c1; // C1
            C12[i * ldc + j] = c2; // C2
            C21[i * ldc + j] = c2; // C2
            C22[i * ldc + j] = c1; // C1
        }
    }
}

/**
 * Wrapper function for Matrix Multiplication
 */
void multiplySpecialPattern(const int* A, const int* B, int* C, int n) {
    /*
     * Workspace size derivation:
     * At each recursion level with block size m, 6 buffers of size (m/2)^2 are consumed:
     *   S_A1, S_B1, P1, S_A2, S_B2, P2  ->  6 * (m/2)^2 integers
     * Summing over all levels (geometric series):
     *   Level 1: 6*(n/2)^2 = 6n^2/4
     *   Level 2: 6*(n/4)^2 = 6n^2/16
     *   ...  Total = 6n^2/4 * 1/(1-1/4) = 2n^2  (exact theoretical minimum)
     *
     * 2*n*n is the exact minimum with ZERO safety margin. Any off-by-one in
     * pointer arithmetic causes silent heap corruption. We use 3*n*n as a
     * safe upper bound (50% extra headroom) to prevent undefined behaviour.
     */
    size_t workspaceSize = 3 * (size_t)n * n;
    int* workspace = (int*)calloc(workspaceSize, sizeof(int));
    if (!workspace) {
        fprintf(stderr, "[Fatal Error]: Dynamic memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }

    multiplySpecialRec(A, n, B, n, C, n, n, workspace);

    free(workspace);
}

int main() {
    int n;

    printf("=====================================================\n");
    printf("   SPECIAL PATTERN MATRIX MULTIPLICATION (D&C O(n^2)) \n");
    printf("=====================================================\n\n");

    printf("Enter matrix dimension n: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        fprintf(stderr, "\n[Error]: Invalid dimension! Must be a positive integer.\n");
        return EXIT_FAILURE;
    }

    // 1. Validation Check: Ensure n is a power of 2 (n = 2^k)
    if (!isPowerOfTwo(n)) {
        printf("\n=====================================================\n");
        printf("[NOT POSSIBLE]: Special D&C Matrix Multiplication cannot be performed!\n");
        printf("Reason: Dimension n = %d is odd or not a power of 2 (2^k).\n", n);
        printf("The recursive structure requires n = 2^k to split equal blocks.\n");
        printf("=====================================================\n\n");

        // Print mathematical concept and complexity analysis regardless
        printExplanation(n);
        return EXIT_SUCCESS;
    }

    // Allocate 1D contiguous matrix arrays
    int* A = (int*)calloc((size_t)n * n, sizeof(int));
    int* B = (int*)calloc((size_t)n * n, sizeof(int));
    int* C = (int*)calloc((size_t)n * n, sizeof(int));

    if (!A || !B || !C) {
        fprintf(stderr, "[Fatal Error]: Memory allocation failed!\n");
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

    // Execute Divide & Conquer Multiplication
    multiplySpecialPattern(A, B, C, n);

    // Output Result Matrix
    printf("\n--- RESULT MATRIX C (A * B) ---\n");
    printMatrix(C, n);

    // Output Theoretical Concepts and Complexity Analysis
    printExplanation(n);

    // Clean up
    free(A);
    free(B);
    free(C);

    return EXIT_SUCCESS;
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
    printf("1. MATHEMATICAL FORMULATION & CONCEPT:\n");
    printf("   - Given special-pattern matrices A and B of the form:\n");
    printf("     M = [M1  M2]\n");
    printf("         [M2  M1]\n");
    printf("   - Standard block multiplication yields:\n");
    printf("     C1 = A1*B1 + A2*B2\n");
    printf("     C2 = A1*B2 + A2*B1\n");
    printf("   - Instead of computing 4 recursive multiplications, we define:\n");
    printf("     P1 = (A1 + A2) * (B1 + B2)\n");
    printf("     P2 = (A1 - A2) * (B1 - B2)\n");
    printf("   - Reconstructing C:\n");
    printf("     C1 = (P1 + P2) / 2\n");
    printf("     C2 = (P1 - P2) / 2\n\n");

    printf("2. RECURRENCE RELATION:\n");
    printf("   - Base Case  (n = 1) : T(1) = O(1)\n");
    printf("   - Recursive  (n > 1) : T(n) = 2 * T(n/2) + O(n^2)\n");
    printf("     where 2*T(n/2) represents the 2 recursive subproblem calls\n");
    printf("     and O(n^2) accounts for matrix additions/subtractions.\n\n");

    printf("3. TIME COMPLEXITY DERIVATION (MASTER THEOREM):\n");
    printf("   - General Recurrence: T(n) = a*T(n/b) + f(n)\n");
    printf("   - Parameters: a = 2, b = 2, f(n) = O(n^2)\n");
    printf("   - Critical Exponent: log_b(a) = log_2(2) = 1\n");
    printf("   - Compare f(n) with n^(log_b a):\n");
    printf("     f(n) = O(n^2) = Omega(n^(1 + 1))  [Case 3 of Master Theorem]\n");
    printf("   - Regularity Condition check: 2 * (n/2)^2 = n^2 / 2 <= c * n^2 (holds for c = 1/2 < 1)\n");
    printf("   - Final Time Complexity = Theta(f(n)) = O(n^2)\n\n");

    printf("4. SPACE COMPLEXITY ANALYSIS:\n");
    printf("   - Call Stack Depth = O(log n)\n");
    printf("   - Auxiliary Memory = Pre-allocated scratchpad buffer of size O(n^2)\n");
    printf("   - Overall Space Complexity = O(n^2)\n");
    printf("=====================================================\n");
}