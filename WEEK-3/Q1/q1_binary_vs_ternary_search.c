#include <stdio.h>
#include <stdlib.h>

/**
 * Performs Binary Search and counts key comparisons using 1-based indexing.
 */
int binarySearch(const int arr[], int size, int target, int *comparisons) {
    int low = 1, high = size;
    *comparisons = 0;

    while (low <= high) {
        int mid = low + (high - low) / 2;

        (*comparisons)++;
        if (arr[mid] == target) {
            return mid;
        }

        (*comparisons)++;
        if (arr[mid] > target) {
            high = mid - 1;
        } else {
            low = mid + 1;
        }
    }
    return -1; // Target not found
}

/**
 * Performs Ternary Search and counts key comparisons using 1-based indexing.
 */
int ternarySearch(const int arr[], int size, int target, int *comparisons) {
    int low = 1, high = size;
    *comparisons = 0;

    while (low <= high) {
        int mid1 = low + (high - low) / 3;
        int mid2 = high - (high - low) / 3;

        (*comparisons)++;
        if (arr[mid1] == target) {
            return mid1;
        }

        (*comparisons)++;
        if (arr[mid2] == target) {
            return mid2;
        }

        (*comparisons)++;
        if (target < arr[mid1]) {
            high = mid1 - 1;
        } else {
            (*comparisons)++;
            if (target > arr[mid2]) {
                low = mid2 + 1;
            } else {
                low = mid1 + 1;
                high = mid2 - 1;
            }
        }
    }
    return -1; // Target not found
}

/**
 * Outputs theoretical explanation and mathematical validation.
 */
void printExplanation(void) {
    printf("\n===========================================================\n");
    printf("        THEORETICAL PROOF & RECURRENCE ANALYSIS\n");
    printf("===========================================================\n\n");
    
    printf("1. Binary Search Analysis:\n");
    printf("   - Recurrence: T(n) = T(n/2) + 2 comparisons (worst case)\n");
    printf("   - Height of Decision Tree: log2(n)\n");
    printf("   - Total Comparisons: C_binary(n) = 2 * log2(n)\n\n");

    printf("2. Ternary Search Analysis:\n");
    printf("   - Recurrence: T(n) = T(n/3) + 4 comparisons (worst case)\n");
    printf("   - Height of Decision Tree: log3(n) = log2(n) / log2(3)\n");
    printf("   - Total Comparisons: C_ternary(n) = 4 * log3(n)\n");
    printf("                                     = 4 * (log2(n) / 1.585)\n");
    printf("                                     ~ 2.524 * log2(n)\n\n");

    printf("3. Conclusion:\n");
    printf("   - Although both are O(log n), Ternary Search makes ~26%% MORE\n");
    printf("     comparisons in the worst case due to a larger constant factor.\n");
    printf("   - Hence, Binary Search is more efficient for sorted arrays.\n");
    printf("===========================================================\n");
}

int main(void) {
    int n;
    
    printf("Enter the number of elements in the array: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Error: Please enter a valid positive integer for the array size.\n");
        return 1;
    }

    // Allocate n + 1 integers to allow 1-based indexing (ignoring index 0)
    int *arr = (int *)malloc((n + 1) * sizeof(int));
    if (!arr) {
        printf("Error: Memory allocation failed!\n");
        return 1;
    }

    printf("Enter %d sorted integers (separated by spaces or newlines):\n", n);
    // Loop starts from 1 and goes up to n
    for (int i = 1; i <= n; i++) {
        scanf("%d", &arr[i]);
        
        // Safety check to ensure the user inputs sorted data
        if (i > 1 && arr[i] < arr[i - 1]) {
            printf("\n[!] WARNING: You entered a value smaller than the previous one.\n");
            printf("    Binary and Ternary search algorithms require a SORTED array.\n");
            printf("    The search results may be inaccurate.\n\n");
        }
    }

    int target;
    printf("Enter the target element to search for: ");
    scanf("%d", &target);

    // Initialize comparison counters
    int bin_comp = 0, tern_comp = 0;

    // Execute searches
    int bin_index  = binarySearch(arr, n, target, &bin_comp);
    int tern_index = ternarySearch(arr, n, target, &tern_comp);

    // Print empirical results
    printf("\n===========================================================\n");
    printf("                 EMPIRICAL TEST RESULTS\n");
    printf("===========================================================\n");
    printf("Array Size (n):              %d\n", n);
    printf("Target Element:              %d\n", target);

    if (bin_index != -1) {
        // Output will naturally print the 1-based index
        printf("Binary Search  : FOUND at index %d\n", bin_index);
    } else {
        printf("Binary Search  : NOT FOUND\n");
    }
    if (tern_index != -1) {
        printf("Ternary Search : FOUND at index %d\n", tern_index);
    } else {
        printf("Ternary Search : NOT FOUND\n");
    }
    
    printf("-----------------------------------------------------------\n");
    printf("Binary Search Comparisons:   %d\n", bin_comp);
    printf("Ternary Search Comparisons:  %d\n", tern_comp);

    // Print theoretical validation
    printExplanation();

    // Clean up
    free(arr);
    return 0;
}