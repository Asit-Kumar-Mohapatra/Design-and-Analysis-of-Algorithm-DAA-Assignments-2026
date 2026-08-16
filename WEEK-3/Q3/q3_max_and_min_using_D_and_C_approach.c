#include <stdio.h>
#include <stdlib.h>

// Function Prototypes
void getMinMaxDNC(const int arr[], int low, int high, int *min, int *max, long long *comparisons);
void printAnalysis(int n, long long actualComparisons);

int main(void) {
    int n;

    printf("==================================================\n");
    printf("     DIVIDE & CONQUER: MAX & MIN FINDER IN C      \n");
    printf("==================================================\n\n");

    // 1. Dynamic Array Size Input with Validation
    printf("Enter the number of elements (n > 0): ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("\n[Error]: Invalid array size! Please enter a positive integer.\n");
        return 1;
    }

    // 2. Heap Allocation
    int *arr = (int *)malloc((size_t)n * sizeof(int));
    if (arr == NULL) {
        printf("\n[Error]: Memory allocation failed!\n");
        return 1;
    }

    // 3. Dynamic Array Elements Input
    printf("Enter %d element(s) separated by spaces:\n", n);
    for (int i = 0; i < n; i++) {
        if (scanf("%d", &arr[i]) != 1) {
            printf("\n[Error]: Invalid input at index %d!\n", i);
            free(arr);
            return 1;
        }
    }

    // 4. Algorithm Execution
    int min, max;
    long long comparisons = 0;

    getMinMaxDNC(arr, 0, n - 1, &min, &max, &comparisons);

    // 5. Results Display
    printf("\n--------------------------------------------------\n");
    printf("                   RESULTS                        \n");
    printf("--------------------------------------------------\n");
    printf(" Array Size (n)       : %d\n", n);
    printf(" Minimum Element      : %d\n", min);
    printf(" Maximum Element      : %d\n", max);

    // 6. Output Theoretical Analysis & Validation
    printAnalysis(n, comparisons);

    // Free dynamically allocated memory
    free(arr);
    arr = NULL;

    return 0;
}

void getMinMaxDNC(const int arr[], int low, int high, int *min, int *max, long long *comparisons) {
    int n = high - low + 1;

    // Base Case 1: single element
    if (n == 1) {
        *min = arr[low];
        *max = arr[low];
        return;
    }

    // Base Case 2: exactly two elements
    if (n == 2) {
        (*comparisons)++;
        if (arr[low] > arr[high]) {
            *max = arr[low];
            *min = arr[high];
        } else {
            *max = arr[high];
            *min = arr[low];
        }
        return;
    }

    // Divide Step: peel off the first pair; recurse on the rest
    int pairA = arr[low];
    int pairB = arr[low + 1];
    int pairMin, pairMax;

    (*comparisons)++;
    if (pairA > pairB) {
        pairMax = pairA;
        pairMin = pairB;
    } else {
        pairMax = pairB;
        pairMin = pairA;
    }

    int restMin, restMax;
    getMinMaxDNC(arr, low + 2, high, &restMin, &restMax, comparisons);

    // Combine Step (2 comparisons)
    (*comparisons)++;
    *max = (pairMax > restMax) ? pairMax : restMax;
    (*comparisons)++;
    *min = (pairMin < restMin) ? pairMin : restMin;
}

/**
 * Output theoretical recurrence relation, time, space, and bound status.
 */
void printAnalysis(int n, long long actualComparisons) {
    printf("\n--------------------------------------------------\n");
    printf("             COMPLEXITY & VALIDATION              \n");
    printf("--------------------------------------------------\n");
    printf(" Recurrence Relation  : T(n) = T(n-2) + 3   [1 to order the pair,\n");
    printf("                                              2 to merge into global]\n");
    printf(" Time Complexity      : O(n)\n");
    printf(" Space Complexity     : O(n/2) recursion depth -- see note below\n");
    printf(" Actual Comparisons   : %lld\n", actualComparisons);

    // Tight theoretical bound: ceil(3n/2) - 2
    long long theoreticalBound = (3LL * n + 1) / 2 - 2;
    if (theoreticalBound < 0) theoreticalBound = 0;

    printf(" Theoretical Bound    : %lld  (ceil(3n/2) - 2)\n", theoreticalBound);

    if (actualComparisons <= theoreticalBound) {
        printf(" Status               : SUCCESS (matches optimal ceil(3n/2)-2 bound)\n");
    } else {
        printf(" Status               : FAILED (exceeded theoretical bound)\n");
    }
    printf("--------------------------------------------------\n");
    printf(" Note: this pair-peeling recursion has O(n/2) recursion depth\n");
    printf(" (it is 'tail-recursive' on the remaining elements). If O(log n)\n");
    printf(" space is required instead, convert the tail recursion to a\n");
    printf(" simple iterative loop over pairs -- the comparison count and\n");
    printf(" bound proof are unchanged either way.\n");
    printf("==================================================\n");
}
