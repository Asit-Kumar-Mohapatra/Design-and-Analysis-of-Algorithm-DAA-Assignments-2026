/* ============================================================================
 *  Partition Point Search (0s followed by 1s)
 *  - Implements an optimized Binary Search to find the transition point.
 *  - Takes manual array input from the user.
 *  - Handles edge cases (all 0s, all 1s) gracefully.
 * ==========================================================================*/
#include <stdio.h>
#include <stdlib.h>

/* ===== Core Algorithm ===== */
/* Using 'const' ensures the array cannot be modified during the search */
static int locate_transition_point(const int *arr, int size)
{
    int left_bound = 0;
    int right_bound = size - 1;
    int transition_idx = -1;

    /* Binary Search: O(log n) */
    while (left_bound <= right_bound) {
        /* Prevents potential integer overflow compared to (left+right)/2 */
        int mid_point = left_bound + (right_bound - left_bound) / 2;

        if (arr[mid_point] == 1) {
            transition_idx = mid_point;
            right_bound = mid_point - 1; /* Continue searching left for earlier 1s */
        } else {
            left_bound = mid_point + 1;  /* The transition must be on the right */
        }
    }

    return transition_idx;
}

/* ===== Utility ===== */
static void display_array(const int *arr, int size)
{
    printf(" [ ");
    for (int i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("]\n");
}

/* ===== Reporting ===== */
static void print_theoretical_metrics(void)
{
    printf("\n====================================================\n");
    printf(" ALGORITHM METRICS\n");
    printf("====================================================\n");
    printf(" Time Complexity  : O(log n) - Binary Search halves the search space.\n");
    printf(" Space Complexity : O(1)     - No extra memory allocation required.\n");
    printf("====================================================\n\n");
}

/* ===== MAIN ============================================================== */
int main(void)
{
    int n;
    int *data_array;

    printf("====================================================\n");
    printf(" PARTITION POINT FINDER (0 -> 1 Transition)\n");
    printf("====================================================\n");

    printf("Enter total number of elements: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Invalid input. Exiting.\n");
        return 1;
    }

    /* Dynamically allocate memory based on user input */
    data_array = (int *)malloc(n * sizeof(int));
    if (data_array == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        return 1;
    }

    /* Manual Input Section */
    printf("Enter %d elements (0s strictly followed by 1s, separated by spaces):\n> ", n);
    for (int i = 0; i < n; i++) {
        scanf("%d", &data_array[i]);
    }

    printf("\n--- INPUT DATA ---\n");
    display_array(data_array, n);

    /* Execute Algorithm */
    int partition_index = locate_transition_point(data_array, n);

    /* Process Results */
    printf("\n--- SEARCH RESULTS ---\n");
    if (partition_index == -1) {
        printf(" Status : No transition found.\n");
        printf(" Detail : The array consists entirely of 0s (or contains no 1s).\n");
    } 
    else if (partition_index == 0) {
        printf(" Status : Transition at Index [0].\n");
        printf(" Detail : The array consists entirely of 1s.\n");
    } 
    else {
        printf(" Status : Transition found successfully!\n");
        printf(" Detail : The array transitions from 0 to 1 between index [%d] and [%d].\n", 
               partition_index - 1, partition_index);
    }

    print_theoretical_metrics();

    /* Clean up */
    free(data_array);

    return 0;
}