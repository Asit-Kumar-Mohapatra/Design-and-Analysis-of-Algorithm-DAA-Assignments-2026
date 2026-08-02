/* ============================================================================
 *  Element Uniqueness Verification (Brute Force)
 *  - Checks if an array contains any duplicate elements.
 *  - Uses an exhaustive pairwise comparison strategy O(n^2).
 *  - Takes manual array input from the user via dynamic memory allocation.
 * ==========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* ===== Core Algorithm ===== */
/* Using 'const' ensures the array isn't modified. Returns true if unique. */
static bool verify_uniqueness(const int *arr, int size, long long *comparisons_made)
{
    *comparisons_made = 0;

    for (int i = 0; i < size - 1; i++) {
        for (int j = i + 1; j < size; j++) {
            (*comparisons_made)++;
            
            if (arr[i] == arr[j]) {
                return false; /* Duplicate found, early exit */
            }
        }
    }
    return true; /* Exhausted all pairs, all elements are strictly unique */
}

/* ===== Utility ===== */
static void display_array_data(const int *arr, int size)
{
    printf(" [ ");
    for (int i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("]\n");
}

/* ===== Reporting ===== */
static void print_analytical_summary(void)
{
    printf("\n====================================================\n");
    printf(" THEORETICAL COMPLEXITY ANALYSIS\n");
    printf("====================================================\n");
    printf(" Best Case Time   : O(1)   - Duplicate found immediately.\n");
    printf(" Worst Case Time  : O(n^2) - No duplicates; checks every pair.\n");
    printf(" Space Complexity : O(1)   - Evaluated in-place.\n");
    printf("----------------------------------------------------\n");
    printf(" Conclusion: While this brute-force method is highly\n");
    printf(" space-efficient, the nested loop structure makes it\n");
    printf(" inherently slow for large datasets. Using a hash map\n");
    printf(" or sorting the array first would improve time efficiency.\n");
    printf("====================================================\n\n");
}

/* ===== MAIN ============================================================== */
int main(void)
{
    int element_count;
    int *data_stream;

    printf("====================================================\n");
    printf(" ELEMENT UNIQUENESS VERIFIER\n");
    printf("====================================================\n");

    printf("Enter the total number of elements to process: ");
    if (scanf("%d", &element_count) != 1 || element_count <= 0) {
        fprintf(stderr, "Invalid input. Execution terminated.\n");
        return 1;
    }

    /* Dynamic allocation replaces static arrays for safe memory management */
    data_stream = (int *)malloc(element_count * sizeof(int));
    if (data_stream == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        return 1;
    }

    /* Manual Input Section */
    printf("Enter %d integers (separated by spaces):\n> ", element_count);
    for (int i = 0; i < element_count; i++) {
        scanf("%d", &data_stream[i]);
    }

    printf("\n--- INPUT DATA ---\n");
    display_array_data(data_stream, element_count);

    /* Execute Algorithm */
    long long ops_count = 0;
    bool is_unique = verify_uniqueness(data_stream, element_count, &ops_count);

    /* Process Results */
    printf("\n--- VERIFICATION RESULTS ---\n");
    if (is_unique) {
        printf(" Status      : ALL ELEMENTS ARE UNIQUE.\n");
    } else {
        printf(" Status      : DUPLICATES DETECTED.\n");
    }
    printf(" Comparisons : %lld operation(s) performed.\n", ops_count);

    print_analytical_summary();

    /* Clean up */
    free(data_stream);

    return 0;
}