/*
 * ============================================================
 *  HEAP SORT ON N RANDOMLY GENERATED ELEMENTS STORED IN A FILE
 * ============================================================
 *
 *  Steps:
 *    1. Ask the user to enter N.
 *    2. Generate N random integers and store them in "input.csv".
 *    3. Read the N elements back from the file into an array.
 *    4. Sort the array using an optimized (iterative, in-place) Heap Sort.
 *    5. Write the sorted output to "sorted_output.csv" and print a sample.
 *
 *  Compile : gcc -O2 -o heap_sort heap_sort.c -lm
 *  Run     : ./heap_sort
 *            (the program will prompt: "Enter the number of elements (N): ")
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ---------------------------------------------------------------
 * heapify (iterative version)
 * ---------------------------------------------------------------
 * Sifts the element at index 'root' down into its correct position
 * in a max-heap of size 'n'. Using an iterative loop instead of
 * recursion avoids function-call overhead and stack usage,
 * which matters for large N — this is the key optimization.
 * ---------------------------------------------------------------
 */
void heapify(int arr[], int n, int root) {
    while (1) {
        int largest = root;
        int left    = 2 * root + 1;
        int right   = 2 * root + 2;

        if (left < n && arr[left] > arr[largest])
            largest = left;

        if (right < n && arr[right] > arr[largest])
            largest = right;

        if (largest == root)
            break;                      /* heap property restored, stop */

        /* swap */
        int temp = arr[root];
        arr[root] = arr[largest];
        arr[largest] = temp;

        root = largest;                 /* continue sifting down */
    }
}

/* ---------------------------------------------------------------
 * heapSort
 * ---------------------------------------------------------------
 * Phase 1 (Build Heap): Convert arr[0..n-1] into a max-heap.
 *                        Start from the last internal node
 *                        (n/2 - 1) and heapify upward -> O(n).
 * Phase 2 (Extraction) : Repeatedly swap the root (maximum) with
 *                        the last element of the heap, shrink the
 *                        heap by one, and heapify the root.
 *                        This is done n-1 times -> O(n log n).
 * ---------------------------------------------------------------
 */
void heapSort(int arr[], int n) {
    /* Phase 1: Build max heap - O(n) */
    for (int i = n / 2 - 1; i >= 0; i--)
        heapify(arr, n, i);

    /* Phase 2: Extract elements one by one - O(n log n) */
    for (int i = n - 1; i > 0; i--) {
        int temp = arr[0];
        arr[0] = arr[i];
        arr[i] = temp;

        heapify(arr, i, 0);   /* heapify reduced heap of size i */
    }
}

/* ---------------------------------------------------------------
 * generateInputFile
 * ---------------------------------------------------------------
 * Generates N random integers (range 0 to 99999) and writes them
 * to a CSV file, one value per line (single-column CSV).
 * ---------------------------------------------------------------
 */
void generateInputFile(const char *filename, int n) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Error creating input file");
        exit(EXIT_FAILURE);
    }

    fprintf(fp, "Random_Numbers\n");   /* CSV header row */

    srand((unsigned int) time(NULL));
    for (int i = 0; i < n; i++)
        fprintf(fp, "%d\n", rand() % 100000);

    fclose(fp);
}

/* ---------------------------------------------------------------
 * readInputFile
 * ---------------------------------------------------------------
 * Reads N integers from the given CSV file into a pre-allocated
 * array, skipping the header row.
 * ---------------------------------------------------------------
 */
void readInputFile(const char *filename, int arr[], int n) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    char header[64];
    if (fgets(header, sizeof(header), fp) == NULL) {
        fprintf(stderr, "Error: input file is empty or missing header\n");
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n; i++) {
        if (fscanf(fp, "%d", &arr[i]) != 1) {
            fprintf(stderr, "Error: file has fewer than %d elements\n", n);
            fclose(fp);
            exit(EXIT_FAILURE);
        }
    }

    fclose(fp);
}

/* ---------------------------------------------------------------
 * writeOutputFile
 * ---------------------------------------------------------------
 * Writes the sorted array to a CSV file, one value per line
 * (single-column CSV, with a header row).
 * ---------------------------------------------------------------
 */
void writeOutputFile(const char *filename, int arr[], int n) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Error creating output file");
        exit(EXIT_FAILURE);
    }

    fprintf(fp, "Sorted_Numbers\n");   /* CSV header row */

    for (int i = 0; i < n; i++)
        fprintf(fp, "%d\n", arr[i]);

    fclose(fp);
}

/* ---------------------------------------------------------------
 * printSample
 * ---------------------------------------------------------------
 * Prints up to 'count' elements from the array for a quick check
 * (printing all N elements for large N is not useful/practical).
 * ---------------------------------------------------------------
 */
void printSample(const char *label, int arr[], int n, int count) {
    int limit = (n < count) ? n : count;
    printf("%s (first %d of %d elements): ", label, limit, n);
    for (int i = 0; i < limit; i++)
        printf("%d ", arr[i]);
    printf("%s\n", (n > limit) ? "..." : "");
}

/* =================================================================
 * COMPLEXITY ANALYSIS SECTION
 * =================================================================
 * The functions below are instrumented copies of heapify/heapSort
 * that COUNT comparisons and swaps instead of just running fast.
 * They are used only by analyzeComplexity() so the real sort above
 * stays at zero overhead.
 * ================================================================= */

void heapifyCounted(int arr[], int n, int root, long *comparisons, long *swaps) {
    while (1) {
        int largest = root;
        int left    = 2 * root + 1;
        int right   = 2 * root + 2;

        if (left < n)  { (*comparisons)++; if (arr[left]  > arr[largest]) largest = left;  }
        if (right < n) { (*comparisons)++; if (arr[right] > arr[largest]) largest = right; }

        if (largest == root)
            break;

        int temp = arr[root];
        arr[root] = arr[largest];
        arr[largest] = temp;
        (*swaps)++;

        root = largest;
    }
}

void heapSortCounted(int arr[], int n, long *comparisons, long *swaps) {
    for (int i = n / 2 - 1; i >= 0; i--)
        heapifyCounted(arr, n, i, comparisons, swaps);

    for (int i = n - 1; i > 0; i--) {
        int temp = arr[0];
        arr[0] = arr[i];
        arr[i] = temp;
        (*swaps)++;
        heapifyCounted(arr, i, 0, comparisons, swaps);
    }
}

/* ---------------------------------------------------------------
 * log2i - integer-friendly base-2 logarithm using standard library
 * ---------------------------------------------------------------
 */
#include <math.h>
double log2i(int n) { return (n <= 1) ? 0.0 : log((double) n) / log(2.0); }

/* ---------------------------------------------------------------
 * analyzeComplexity
 * ---------------------------------------------------------------
 * Empirically demonstrates the O(n log n) time complexity of
 * Heap Sort by running it on a range of increasing input sizes
 * and comparing:
 *   - actual comparisons made        (measured)
 *   - theoretical n*log2(n)          (predicted growth curve)
 *   - actual wall-clock time         (measured)
 *   - ratio = comparisons / (n*log2(n))
 *
 * WHY THIS PROVES O(n log n):
 *   Build-heap phase   : n/2 heapify calls, each doing O(log n)
 *                        work in the worst case -> the phase as a
 *                        whole is a tight O(n) (proven via the
 *                        sum-of-heights argument), not O(n log n).
 *   Extraction phase   : (n-1) extract-max operations, each
 *                        followed by a heapify of O(log n) ->
 *                        this phase is O(n log n) and DOMINATES
 *                        the total running time.
 *   Overall            : O(n) + O(n log n) = O(n log n).
 *
 * If the algorithm is truly O(n log n), the ratio
 * (comparisons / (n*log2(n))) should stay roughly CONSTANT as n
 * grows -- it does not increase toward n^2 territory, and it does
 * not shrink toward O(n) territory. That stability is the proof.
 * ---------------------------------------------------------------
 */
void analyzeComplexity(void) {
    int sizes[] = {1000, 2000, 4000, 8000, 16000, 32000, 64000};
    int numSizes = sizeof(sizes) / sizeof(sizes[0]);

    printf("\n================ COMPLEXITY ANALYSIS (Heap Sort) ================\n");
    printf("%-10s %-15s %-18s %-10s %-12s\n",
           "N", "Comparisons", "N*log2(N)", "Ratio", "Time(s)");
    printf("-------------------------------------------------------------------\n");

    srand(42); /* fixed seed so sizes are comparable across a run */

    for (int s = 0; s < numSizes; s++) {
        int n = sizes[s];
        int *arr = (int *) malloc(n * sizeof(int));
        for (int i = 0; i < n; i++) arr[i] = rand() % 1000000;

        long comparisons = 0, swaps = 0;
        clock_t start = clock();
        heapSortCounted(arr, n, &comparisons, &swaps);
        clock_t end = clock();

        double theoretical = n * log2i(n);
        double ratio = (theoretical > 0) ? (comparisons / theoretical) : 0.0;
        double timeTaken = (double)(end - start) / CLOCKS_PER_SEC;

        printf("%-10d %-15ld %-18.1f %-10.3f %-12.6f\n",
               n, comparisons, theoretical, ratio, timeTaken);

        free(arr);
    }

    printf("-------------------------------------------------------------------\n");
    printf("Observation: the Ratio column stays roughly constant as N grows.\n");
    printf("This confirms comparisons scale as N*log2(N), i.e. Heap Sort runs\n");
    printf("in O(N log N) time in practice, matching the theoretical analysis:\n");
    printf("  Build-heap phase : O(N)\n");
    printf("  Extraction phase : O(N log N)  <-- dominant term\n");
    printf("  Overall          : O(N log N) for best, average, and worst case.\n");
    printf("  Space complexity : O(1) auxiliary (in-place sort).\n");
    printf("===================================================================\n");
}

int main(void) {
    int n;

    printf("Enter the number of elements (N): ");
    while (scanf("%d", &n) != 1 || n <= 0) {
        /* clear bad input from stdin so we don't loop forever on garbage input */
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }
        printf("Invalid input. Please enter a positive integer for N: ");
    }

    const char *inputFile  = "input.csv";
    const char *outputFile = "sorted_output.csv";

    printf("Generating %d random elements into '%s'...\n", n, inputFile);
    generateInputFile(inputFile, n);

    /* Single dynamic allocation sized exactly to N -> O(n) space, no resizing overhead */
    int *arr = (int *) malloc(n * sizeof(int));
    if (!arr) {
        fprintf(stderr, "Memory allocation failed.\n");
        return EXIT_FAILURE;
    }

    printf("Reading %d elements from '%s'...\n", n, inputFile);
    readInputFile(inputFile, arr, n);

    printSample("Before sorting", arr, n, 20);

    clock_t start = clock();
    heapSort(arr, n);
    clock_t end = clock();

    printSample("After sorting ", arr, n, 20);

    printf("Writing sorted elements to '%s'...\n", outputFile);
    writeOutputFile(outputFile, arr, n);

    double timeTaken = (double)(end - start) / CLOCKS_PER_SEC;
    printf("\nHeap Sort completed for N = %d elements.\n", n);
    printf("Time taken: %f seconds\n", timeTaken);

    free(arr);

    /* Run empirical complexity analysis to demonstrate O(n log n) growth */
    analyzeComplexity();

    return 0;
}