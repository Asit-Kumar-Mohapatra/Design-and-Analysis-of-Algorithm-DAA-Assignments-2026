/* ============================================================================
 *  Performance Analysis of Bubble Sort
 *  - Compares an early-terminating Bubble Sort vs a Full-Pass Bubble Sort.
 *  - Evaluates efficiency via comparison counts on randomized sequences.
 *  - Exports results to 'performance_analysis_of_bubble_sort.csv'.
 *  - Displays theoretical Time and Space Complexities.
 * ==========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define MAX_ARRAY_SIZE 10000
#define GRAPH_MAX_WIDTH 50

/* ===== Utility Functions ===== */
static void swap_elements(int *a, int *b) 
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

static void fill_random_data(int *arr, int size) 
{
    for (int i = 0; i < size; i++) {
        arr[i] = rand() % 1000; /* Random integers between 0 and 999 */
    }
}

static void clone_array(const int *source, int *destination, int size) 
{
    for (int i = 0; i < size; i++) {
        destination[i] = source[i];
    }
}

/* ===== Sorting Algorithms ===== */

/* (i) Bubble sort that terminates if the array is sorted before (n-1)th pass */
static long long bubble_sort_optimized(int *arr, int n) 
{
    long long comparisons = 0;
    bool is_swapped;
    
    for (int pass = 0; pass < n - 1; pass++) {
        is_swapped = false;
        for (int j = 0; j < n - pass - 1; j++) {
            comparisons++;
            if (arr[j] > arr[j + 1]) {
                swap_elements(&arr[j], &arr[j + 1]);
                is_swapped = true;
            }
        }
        /* Early termination condition */
        if (!is_swapped) {
            break; 
        }
    }
    return comparisons;
}

/* (ii) Bubble sort that always completes the (n-1)th pass */
static long long bubble_sort_naive(int *arr, int n) 
{
    long long comparisons = 0;
    
    for (int pass = 0; pass < n - 1; pass++) {
        for (int j = 0; j < n - pass - 1; j++) {
            comparisons++;
            if (arr[j] > arr[j + 1]) {
                swap_elements(&arr[j], &arr[j + 1]);
            }
        }
    }
    return comparisons;
}

/* ===== Reporting and Visualization ===== */
static void export_to_csv(long long opt_comps, long long naive_comps, int n) 
{
    const char *filename = "performance_analysis_of_bubble_sort.csv";
    FILE *fp = fopen(filename, "w");
    
    if (fp == NULL) {
        fprintf(stderr, "[!] Error: Unable to create CSV file.\n");
        return;
    }

    fprintf(fp, "Data_Size,Algorithm,Comparisons\n");
    fprintf(fp, "%d,Optimized (Early Stop),%lld\n", n, opt_comps);
    fprintf(fp, "%d,Naive (Full Pass),%lld\n", n, naive_comps);

    fclose(fp);
    printf("[*] Data successfully exported to '%s'\n", filename);
}

static void draw_comparison_graph(long long opt_comps, long long naive_comps) 
{
    printf("\n--- ASCII PERFORMANCE GRAPH (Comparison Count) ---\n");
    
    /* Determine the scale dynamically based on the larger value */
    long long max_comps = (naive_comps > opt_comps) ? naive_comps : opt_comps;
    if (max_comps == 0) max_comps = 1; /* Prevent division by zero */

    int opt_bars = (int)((double)opt_comps / max_comps * GRAPH_MAX_WIDTH);
    int naive_bars = (int)((double)naive_comps / max_comps * GRAPH_MAX_WIDTH);

    /* Render Optimized Bar */
    printf("%-20s | ", "Early Termination");
    for (int i = 0; i < opt_bars; i++) putchar('#');
    printf(" (%lld)\n", opt_comps);

    /* Render Naive Bar */
    printf("%-20s | ", "Full Pass");
    for (int i = 0; i < naive_bars; i++) putchar('#');
    printf(" (%lld)\n\n", naive_comps);
}

static void display_theoretical_complexities(void)
{
    printf("====================================================\n");
    printf(" THEORETICAL COMPLEXITY ANALYSIS\n");
    printf("====================================================\n");
    printf("%-20s | %-15s | %-15s\n", "Metric", "Early Stop", "Full Pass");
    printf("---------------------+-----------------+-----------------\n");
    printf("%-20s | %-15s | %-15s\n", "Best Case Time", "O(n)", "O(n^2)");
    printf("%-20s | %-15s | %-15s\n", "Average Case Time", "O(n^2)", "O(n^2)");
    printf("%-20s | %-15s | %-15s\n", "Worst Case Time", "O(n^2)", "O(n^2)");
    printf("%-20s | %-15s | %-15s\n", "Space Complexity", "O(1)", "O(1)");
    printf("----------------------------------------------------\n\n");
}

/* ===== MAIN ============================================================== */
int main(void) 
{
    int data_size;
    int *base_array, *test_arr_1, *test_arr_2;

    srand((unsigned int)time(NULL));

    printf("====================================================\n");
    printf("    BUBBLE SORT: OPTIMIZED VS NAIVE COMPARISON\n");
    printf("====================================================\n");

    printf("Enter number of elements to simulate (Max %d): ", MAX_ARRAY_SIZE);
    if (scanf("%d", &data_size) != 1 || data_size < 2 || data_size > MAX_ARRAY_SIZE) {
        data_size = 100; /* Safe fallback */
        printf("Invalid input. Defaulting to n = %d\n", data_size);
    }

    /* Dynamically allocate memory for cleaner execution on larger data sets */
    base_array = (int*)malloc(data_size * sizeof(int));
    test_arr_1 = (int*)malloc(data_size * sizeof(int));
    test_arr_2 = (int*)malloc(data_size * sizeof(int));

    if (!base_array || !test_arr_1 || !test_arr_2) {
        fprintf(stderr, "Memory allocation failed!\n");
        return 1;
    }

    fill_random_data(base_array, data_size);
    
    /* Ensure both algorithms sort the exact same dataset */
    clone_array(base_array, test_arr_1, data_size);
    clone_array(base_array, test_arr_2, data_size);

    /* Execute sorts and capture comparison metrics */
    long long optimized_comparisons = bubble_sort_optimized(test_arr_1, data_size);
    long long naive_comparisons     = bubble_sort_naive(test_arr_2, data_size);

    /* Output Results */
    printf("\n[RESULTS] Array Size: %d\n", data_size);
    printf("----------------------------------------------------\n");
    printf("%-30s : %lld comparisons\n", "1. Optimized (Early Stop)", optimized_comparisons);
    printf("%-30s : %lld comparisons\n", "2. Naive (Full Pass)", naive_comparisons);
    printf("----------------------------------------------------\n");
    printf("Operations Saved: %lld\n", naive_comparisons - optimized_comparisons);

    draw_comparison_graph(optimized_comparisons, naive_comparisons);
    export_to_csv(optimized_comparisons, naive_comparisons, data_size);
    
    printf("\n");
    display_theoretical_complexities();

    /* Cleanup */
    free(base_array);
    free(test_arr_1);
    free(test_arr_2);

    return 0;
}