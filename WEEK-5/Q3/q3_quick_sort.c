/*
 * =====================================================================
 *  QUICK SORT OF N RANDOM ELEMENTS STORED IN A FILE
 * =====================================================================
 *
 *  Optimizations applied:
 *  1. Median-of-three pivot selection      -> avoids O(N^2) on sorted/
 *                                              reverse-sorted/organ-pipe
 *                                              inputs, gives good pivots.
 *  2. Insertion sort cutoff for small       -> Quicksort has high constant
 *     sub-arrays (< 10 elements)               overhead for tiny arrays;
 *                                                insertion sort wins there.
 *  3. Tail-call elimination via explicit    -> Converts one recursive call
 *     iteration (recurse on smaller half,       into a loop, guaranteeing
 *     loop on larger half)                      O(log N) stack depth
 *                                                worst case instead of O(N).
 *  4. Explicit stack size bound             -> log2(N) depth is provable,
 *                                              so no dynamic stack needed.
 *  5. Iterative partition (Hoare scheme)    -> Hoare partition does ~3x
 *                                              fewer swaps than Lomuto on
 *                                              average.
 *  6. Buffered file I/O (fread/fwrite in    -> avoids N separate fscanf/
 *     bulk, not element-by-element)            fprintf syscalls; O(N) I/O
 *                                                instead of O(N) with huge
 *                                                constant.
 *  7. In-place sort, O(1) extra memory      -> no auxiliary array needed
 *     (besides an O(log N) explicit stack)     (unlike mergesort).
 *  8. rand() replaced by a fast xorshift    -> much faster & better
 *     PRNG for generating test data            distribution than libc rand()
 *
 *  Time complexity : O(N log N) average, O(N log N) worst case guaranteed
 *                     (median-of-three + smaller-half recursion prevents
 *                      the classic O(N^2) worst case).
 *  Space complexity: O(log N)  (explicit stack, not O(N) recursion).
 *
 *  Compile : gcc -O3 -march=native -o quicksort_file quicksort_file.c
 *  Run     : ./quicksort_file <N> [random|sorted|reverse|duplicates]
 *            (input type defaults to 'random' if omitted)
 *
 *  Timing  : Sort time uses the best timer available per platform:
 *              - Windows/MinGW: QueryPerformanceCounter (high-res
 *                wall-clock timer). POSIX's clock_gettime()/
 *                CLOCK_PROCESS_CPUTIME_ID do NOT exist on MinGW, so we
 *                branch at compile time with #ifdef _WIN32.
 *              - Linux/macOS: clock_gettime(CLOCK_PROCESS_CPUTIME_ID,
 *                ...), nanosecond resolution, counts only this
 *                process's actual CPU time.
 *            Both are far more precise than clock()/CLOCKS_PER_SEC.
 * =====================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define INSERTION_THRESHOLD 10      /* switch to insertion sort below this size */
#define DATA_FILE           "random_data.csv"
#define SORTED_FILE         "sorted_data.csv"

/* Output buffer chunk size for fast buffered CSV writing (avoids one
 * fprintf() call per element, which would dominate runtime for large N). */
#define IO_BUFFER_CAP        (1 << 20)   /* 1 MB text buffer */

/* ---------------------------------------------------------------------
 * Fast xorshift32 PRNG - much cheaper than libc rand() and good enough
 * quality for generating test data.
 * ------------------------------------------------------------------- */
static uint32_t xstate = 0;

static inline uint32_t xorshift32(void)
{
    uint32_t x = xstate;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return xstate = x;
}

/* ---------------------------------------------------------------------
 * high_res_seconds - returns elapsed time in seconds at the highest
 * resolution the platform offers (see timing note in the file header).
 * ------------------------------------------------------------------- */
#ifdef _WIN32

static inline double high_res_seconds(void)
{
    static LARGE_INTEGER freq;
    static int freq_init = 0;
    LARGE_INTEGER counter;

    if (!freq_init) {
        QueryPerformanceFrequency(&freq);
        freq_init = 1;
    }
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)freq.QuadPart;
}

#else /* POSIX: Linux, macOS, etc. */

static inline double high_res_seconds(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

#endif


/* ---------------------------------------------------------------------
 * swap - inline pointer-free swap using a temporary (compiler will
 * turn this into a couple of register moves at -O2/-O3).
 * ------------------------------------------------------------------- */
static inline void swap(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

/* ---------------------------------------------------------------------
 * median_of_three - selects a good pivot by looking at low, mid, high
 * and moving the median value to the 'low' position of the sub-array.
 * This alone defeats the classic already-sorted / reverse-sorted worst
 * case that plagues naive quicksort implementations.
 * ------------------------------------------------------------------- */
static inline void median_of_three(int arr[], int low, int high)
{
    int mid = low + ((high - low) >> 1);

    if (arr[mid] < arr[low])  swap(&arr[mid], &arr[low]);
    if (arr[high] < arr[low]) swap(&arr[high], &arr[low]);
    if (arr[high] < arr[mid]) swap(&arr[high], &arr[mid]);

    /* place pivot (the median) at low+1, keep low/high as sentinels */
    swap(&arr[mid], &arr[low]);
}

/* ---------------------------------------------------------------------
 * insertion_sort - used for small sub-arrays where its low overhead
 * beats quicksort's recursive/partition overhead.
 * ------------------------------------------------------------------- */
static inline void insertion_sort(int arr[], int low, int high)
{
    for (int i = low + 1; i <= high; i++) {
        int key = arr[i];
        int j = i - 1;
        while (j >= low && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

/* ---------------------------------------------------------------------
 * hoare_partition - classic Hoare partition scheme (fewer swaps than
 * Lomuto, roughly N/6 swaps on average vs N/2).
 * Pivot value is arr[low] (already placed there by median_of_three).
 * ------------------------------------------------------------------- */
static inline int hoare_partition(int arr[], int low, int high)
{
    int pivot = arr[low];
    int i = low - 1;
    int j = high + 1;

    for (;;) {
        do { i++; } while (arr[i] < pivot);
        do { j--; } while (arr[j] > pivot);
        if (i >= j) return j;
        swap(&arr[i], &arr[j]);
    }
}

/* ---------------------------------------------------------------------
 * quicksort - iterative, explicit-stack quicksort.
 *   - always recurses (pushes) on the SMALLER partition and loops on
 *     the LARGER partition -> guarantees stack depth O(log N).
 *   - falls back to insertion sort below INSERTION_THRESHOLD.
 *   - uses median-of-three pivoting.
 * ------------------------------------------------------------------- */
void quicksort(int arr[], int n)
{
    if (n < 2) return;

    /* Explicit stack: depth is provably O(log2 N) because we always
     * recurse into the smaller half, so ceil(log2(n))+2 is more than
     * enough (64 covers n up to 2^62). */
    int stack_low[64], stack_high[64];
    int top = -1;

    stack_low[++top]  = 0;
    stack_high[top]   = n - 1;

    while (top >= 0) {
        int low  = stack_low[top];
        int high = stack_high[top--];

        while (low < high) {
            if (high - low + 1 < INSERTION_THRESHOLD) {
                insertion_sort(arr, low, high);
                break;
            }

            median_of_three(arr, low, high);
            int p = hoare_partition(arr, low, high);

            int left_size  = p - low + 1;
            int right_size = high - p;

            if (left_size < right_size) {
                /* push larger (right) half, loop on smaller (left) half */
                stack_low[++top]  = p + 1;
                stack_high[top]   = high;
                high = p;
            } else {
                /* push larger (left) half, loop on smaller (right) half */
                stack_low[++top]  = low;
                stack_high[top]   = p;
                low = p + 1;
            }
        }
    }
}

/* ---------------------------------------------------------------------
 * fill_array - populates arr[0..n-1] according to the requested
 * distribution, used to empirically stress-test whether the
 * median-of-three optimization actually holds up on adversarial
 * inputs (not just average-case random data):
 *
 *   "random"      - uniform random values (default, average case)
 *   "sorted"      - already ascending -> classic naive-quicksort
 *                    O(N^2) trigger (first/last element as pivot)
 *   "reverse"     - descending -> same classic worst-case trigger
 *   "duplicates"  - only 10 distinct values repeated -> stresses
 *                    partitioning quality with many equal keys
 * ------------------------------------------------------------------- */
static void fill_array(int arr[], int n, const char *type)
{
    xstate = (uint32_t)time(NULL) ^ 0x9E3779B9u;
    if (xstate == 0) xstate = 1;

    if (strcmp(type, "sorted") == 0) {
        for (int i = 0; i < n; i++) arr[i] = i;
    } else if (strcmp(type, "reverse") == 0) {
        for (int i = 0; i < n; i++) arr[i] = n - i;
    } else if (strcmp(type, "duplicates") == 0) {
        for (int i = 0; i < n; i++) arr[i] = (int)(xorshift32() % 10);
    } else { /* "random" (default) */
        for (int i = 0; i < n; i++) arr[i] = (int)(xorshift32() % 1000000);
    }
}

/* ---------------------------------------------------------------------
 * write_csv - fast buffered CSV writer: one value per line, with a
 * header row ("index,value") so the file is easy to read in Excel /
 * Notepad / a text editor and easy to eyeball for sortedness.
 *
 * Uses a manual int-to-string routine + a large manual write buffer
 * instead of calling fprintf() once per element - fprintf() has to
 * parse the format string every call, which is the dominant cost when
 * N is large. This keeps CSV writing at O(N) with a low constant.
 * ------------------------------------------------------------------- */
static void write_csv(const char *filename, int arr[], int n)
{
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Could not open %s for writing\n", filename);
        exit(EXIT_FAILURE);
    }

    char *buf = (char *)malloc(IO_BUFFER_CAP);
    if (!buf) {
        fprintf(stderr, "Memory allocation failed for I/O buffer\n");
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    size_t len = 0;
    char tmp[16];

    len += (size_t)sprintf(buf + len, "index,value\n");

    for (int i = 0; i < n; i++) {
        /* flush buffer if it's getting close to full */
        if (len > IO_BUFFER_CAP - 64) {
            fwrite(buf, 1, len, fp);
            len = 0;
        }

        /* manual itoa for index */
        int t = sprintf(tmp, "%d", i);
        memcpy(buf + len, tmp, (size_t)t);
        len += (size_t)t;
        buf[len++] = ',';

        /* manual itoa for value */
        t = sprintf(tmp, "%d", arr[i]);
        memcpy(buf + len, tmp, (size_t)t);
        len += (size_t)t;
        buf[len++] = '\n';
    }

    if (len > 0) fwrite(buf, 1, len, fp);

    free(buf);
    fclose(fp);
}

/* ---------------------------------------------------------------------
 * generate_and_store - creates N integers of the requested
 * distribution, writes them to DATA_FILE as CSV (human-readable), and
 * returns them in a heap-allocated array for sorting in memory.
 * ------------------------------------------------------------------- */
int *generate_and_store(int n, const char *type)
{
    int *arr = (int *)malloc((size_t)n * sizeof(int));
    if (!arr) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    fill_array(arr, n, type);
    write_csv(DATA_FILE, arr, n);

    return arr;
}

/* ---------------------------------------------------------------------
 * store_sorted - writes the sorted array back to disk as CSV.
 * ------------------------------------------------------------------- */
void store_sorted(int arr[], int n)
{
    write_csv(SORTED_FILE, arr, n);
}

/* ---------------------------------------------------------------------
 * main
 * ------------------------------------------------------------------- */
int main(int argc, char *argv[])
{
    int n;
    char type[16] = "random";

    if (argc >= 2) {
        n = atoi(argv[1]);
        if (argc >= 3) {
            strncpy(type, argv[2], sizeof(type) - 1);
            type[sizeof(type) - 1] = '\0';
        }
    } else {
        printf("Enter N (number of random elements): ");
        if (scanf("%d", &n) != 1) {
            fprintf(stderr, "Invalid input\n");
            return EXIT_FAILURE;
        }
    }

    if (n <= 0) {
        fprintf(stderr, "N must be a positive integer\n");
        return EXIT_FAILURE;
    }

    printf("Generating %d '%s' elements and storing to '%s' ...\n", n, type, DATA_FILE);
    int *arr = generate_and_store(n, type);

    double start = high_res_seconds();
    quicksort(arr, n);
    double end = high_res_seconds();

    store_sorted(arr, n);
    printf("Sorted data written to '%s'.\n", SORTED_FILE);
    printf("Sort time: %.9f seconds\n", end - start);

    /* Verification (optional but good practice) */
    int sorted_ok = 1;
    for (int i = 1; i < n; i++) {
        if (arr[i - 1] > arr[i]) { sorted_ok = 0; break; }
    }
    printf("Verification: %s\n", sorted_ok ? "PASSED (array is sorted)" : "FAILED");

    /* Print a small preview for sanity checking on large N */
    int preview = n < 20 ? n : 20;
    printf("First %d sorted elements: ", preview);
    for (int i = 0; i < preview; i++) printf("%d ", arr[i]);
    printf("\n");

    free(arr);
    return EXIT_SUCCESS;
}