#include <stdio.h>
#include <stdlib.h>

// Helper Function Prototypes
static inline void swap(int* a, int* b);
void selectionSort(int* A, int n);
void printArray(const int* A, int n);
void printExplanation(int n);

static inline void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void selectionSort(int* A, int n) {
    if (!A || n < 2) return;

    for (int i = 0; i < n - 1; i++) {
        int minIndex = i;

        // Find minimum element in unsorted slice A[i ... n-1]
        for (int j = i + 1; j < n; j++) {
            if (A[j] < A[minIndex]) {
                minIndex = j;
            }
        }

        // Swap only when needed to minimize unnecessary writes
        if (minIndex != i) {
            swap(&A[i], &A[minIndex]);
        }
    }
}

int main(void) {
    int n;

    printf("=====================================================\n");
    printf("   SELECTION SORT WITH LOOP INVARIANT VALIDATION    \n");
    printf("=====================================================\n\n");

    // 1. Validated User Input for Array Size
    printf("Enter number of elements (n): ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        fprintf(stderr, "\n[Error]: Invalid input! Size must be a positive integer.\n");
        return EXIT_FAILURE;
    }

    // 2. Dynamic Heap Allocation
    int* A = (int*)malloc((size_t)n * sizeof(int));
    if (!A) {
        fprintf(stderr, "\n[Fatal Error]: Heap allocation failed!\n");
        return EXIT_FAILURE;
    }

    // 3. User Input for Array Elements
    printf("\nEnter %d elements (separated by spaces or newlines):\n", n);
    for (int i = 0; i < n; i++) {
        printf(" Element [%d]: ", i);
        if (scanf("%d", &A[i]) != 1) {
            fprintf(stderr, "\n[Error]: Invalid element entered!\n");
            free(A);
            return EXIT_FAILURE;
        }
    }

    // Output Unsorted Array
    printf("\n-----------------------------------------------------\n");
    printf("UNSORTED ARRAY: ");
    printArray(A, n);

    // Execute Selection Sort
    selectionSort(A, n);

    // Output Sorted Array
    printf("SORTED ARRAY   : ");
    printArray(A, n);
    printf("-----------------------------------------------------\n");

    // Output Formal Proofs
    printExplanation(n);

    // Clean up
    free(A);
    return EXIT_SUCCESS;
}

void printArray(const int* A, int n) {
    printf("[ ");
    for (int i = 0; i < n; i++) {
        printf("%d ", A[i]);
    }
    printf("]\n");
}

void printExplanation(int n) {
    (void)n; /* n available for future dynamic analysis if needed */
    printf("\n=====================================================\n");
    printf("     ALGORITHM CONCEPT & COMPLEXITY EXPLANATION      \n");
    printf("=====================================================\n");
    printf("1. LOOP INVARIANT:\n");
    printf("   - Statement: At the start of outer loop iteration i,\n");
    printf("     A[0 ... i-1] contains the i smallest elements in sorted order.\n");
    printf("   - Initialization: For i = 0, A[0 ... -1] is empty -> Trivially true.\n");
    printf("   - Maintenance: Inner loop finds minimum in A[i ... n-1] and swaps it\n");
    printf("     into A[i], preserving sorted order for A[0 ... i].\n");
    printf("   - Termination: Stops at i = n - 1. A[0 ... n-2] is sorted, leaving\n");
    printf("     A[n-1] as the maximum element -> Entire array is sorted.\n\n");

    printf("2. WHY RUN FOR ONLY (n - 1) ELEMENTS?\n");
    printf("   - Once (n - 1) elements are placed in their final positions,\n");
    printf("     the last element is automatically >= all previous elements.\n");
    printf("     Running an n-th iteration would redundantly compare A[n-1] with itself.\n\n");

    printf("3. COMPLEXITY ANALYSIS:\n");
    printf("   - Total Comparisons: Sum_{i=1}^{n-1} (n - i) = n(n - 1) / 2\n");
    printf("   - Worst-Case Time  : Theta(n^2)\n");
    printf("   - Best-Case Time   : Theta(n^2) (Inner loop scans fully regardless of order)\n");
    printf("   - Auxiliary Space  : O(1) (In-place sort)\n");
    printf("=====================================================\n");
}
