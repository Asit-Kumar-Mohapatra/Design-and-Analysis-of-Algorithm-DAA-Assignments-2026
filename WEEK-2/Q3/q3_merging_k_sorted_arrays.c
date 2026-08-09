/*
 * =====================================================================
 *  merging_k_sorted_arrays.c
 * =====================================================================
 *
 *  PROBLEM
 *  -------
 *  Given k sorted arrays, each with n elements, combine them into a
 *  single sorted array of k*n elements.
 *
 *  METHOD 1  -  Sequential ("Repeated") Merge
 *  --------------------------------------------
 *      merge(A1, A2)              -> R2   (size 2n)
 *      merge(R2, A3)               -> R3   (size 3n)
 *      merge(R3, A4)               -> R4   (size 4n)
 *      ...
 *      merge(R(k-1), Ak)           -> Rk   (size kn)
 *
 *      A merge of two sorted arrays of length p and q takes O(p+q) time
 *      (classic linear "zip" merge, as in merge sort).
 *
 *      Cost of step i (i = 2 .. k):  O( (i-1)n + n ) = O(i*n)
 *
 *      Total cost = sum_{i=2}^{k} O(i*n)
 *                 = O( n * (2 + 3 + 4 + ... + k) )
 *                 = O( n * ( k(k+1)/2 - 1 ) )
 *                 = O( n * k^2 )
 *
 *      ==>  WORST-CASE RUNNING TIME OF METHOD 1:   O(n * k^2)
 *
 *  METHOD 2  -  Pairwise / Divide-and-Conquer Merge
 *  --------------------------------------------------
 *      Round 1:  pair up the k arrays of size n      -> k/2 merges,
 *                each O(2n)                          -> level cost O(k*n)
 *      Round 2:  pair up the k/2 arrays of size 2n    -> k/4 merges,
 *                each O(4n)                           -> level cost O(k*n)
 *      Round 3:  pair up the k/4 arrays of size 4n    -> k/8 merges,
 *                each O(8n)                           -> level cost O(k*n)
 *      ...
 *      This continues for log2(k) rounds, and EVERY round costs O(k*n)
 *      because the total number of elements being "touched" per round
 *      is always k*n (each element is compared/copied once per round).
 *
 *      Total cost = O(k*n) * log2(k) = O(n * k * log k)
 *
 *      ==>  WORST-CASE RUNNING TIME OF METHOD 2:   O(n * k * log k)
 *
 *  CONCLUSION (also printed by the program at the end of validation)
 *  -------------------------------------------------------------------
 *      Since log k grows far slower than k, Method 2's bound
 *      O(n*k*log k) is asymptotically and empirically far better than
 *      Method 1's O(n*k^2) for large k. Method 1 degenerates to
 *      essentially insertion-sort-like behaviour over the *arrays*
 *      (each new array has to be merged against a partial result that
 *      keeps growing), whereas Method 2 is the classic k-way
 *      "merge-sort style" divide and conquer, giving it the same
 *      asymptotic advantage merge sort has over insertion sort.
 *
 *  This program:
 *    1. Implements both methods.
 *    2. VALIDATES correctness: for many random trials it checks that
 *       (a) both outputs are sorted, (b) both outputs have length k*n,
 *       (c) both outputs are IDENTICAL to each other and to a
 *           reference sort (qsort of the concatenation) -- i.e. both
 *           methods really do produce the correct merged array.
 *    3. BENCHMARKS both methods over a range of k (n fixed) and a
 *       range of n (k fixed), using a portable high-resolution timer
 *       (see TIMING NOTE below).
 *    4. Writes all benchmark results to merging_k_sorted_arrays.csv
 *       so they can be plotted by plot_merging_k_sorted_arrays.py.
 *
 *  TIMING NOTE
 *  ------------
 *  Earlier versions of this benchmark used clock() with a fixed repeat
 *  count. That works fine on Linux/macOS, but on Windows clock()'s
 *  effective tick resolution is often only ~10-16 ms even though
 *  CLOCKS_PER_SEC reports 1000 -- so very fast calls (small k, small n)
 *  round to 0 or to identical quantized values, producing flat/stepped
 *  plateaus in the "vary_k" plot at low k instead of a smooth curve.
 *
 *  This version fixes that two ways:
 *    (a) Uses a portable high-resolution monotonic timer:
 *          - QueryPerformanceCounter on Windows (sub-microsecond)
 *          - clock_gettime(CLOCK_MONOTONIC, ...) on POSIX (nanosecond)
 *    (b) Times each configuration with an ADAPTIVE repeat loop: instead
 *        of a fixed repeat count, it keeps re-running the merge until
 *        the *cumulative* measured time comfortably exceeds the timer's
 *        resolution (MIN_BENCH_TIME_SEC, default 0.05s = 50ms), then
 *        divides by however many repeats that took. This self-adjusts
 *        to any machine speed or timer resolution automatically, so
 *        even the fastest (k=2, n=small) cases get a stable, non-zero,
 *        non-stepped measurement.
 *
 *  Compile:   gcc -O2 -o merging_k_sorted_arrays merging_k_sorted_arrays.c
 *  Run:       ./merging_k_sorted_arrays
 * =====================================================================
 */

/* Needed on POSIX systems so <time.h> exposes clock_gettime()/
 * CLOCK_MONOTONIC even when compiling in strict standard mode
 * (e.g. -std=c11, which alone hides POSIX extensions). Must be
 * defined before any system header is included. No effect on Windows. */
#ifndef _WIN32
    #define _POSIX_C_SOURCE 199309L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------------------------------------------------------------------
 * Portable high-resolution monotonic timer, returning seconds as a
 * double. This replaces clock(), whose effective resolution on some
 * platforms (notably Windows) is too coarse to time very fast calls.
 * ------------------------------------------------------------------- */
#ifdef _WIN32
    #include <windows.h>
    static double get_time_sec(void) {
        static LARGE_INTEGER freq;
        static int initialized = 0;
        LARGE_INTEGER now;
        if (!initialized) {
            QueryPerformanceFrequency(&freq);
            initialized = 1;
        }
        QueryPerformanceCounter(&now);
        return (double)now.QuadPart / (double)freq.QuadPart;
    }
#else
    #include <time.h>
    static double get_time_sec(void) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
    }
#endif

/* ---------------------------------------------------------------------
 * merge_two: classic linear merge of two sorted int arrays a[0..na) and
 * b[0..nb) into a freshly malloc'd array of size na+nb.  O(na+nb).
 * ------------------------------------------------------------------- */
static int *merge_two(const int *a, int na, const int *b, int nb) {
    int *out = (int *)malloc((size_t)(na + nb) * sizeof(int));
    if (!out) { fprintf(stderr, "malloc failed\n"); exit(1); }

    int i = 0, j = 0, m = 0;
    while (i < na && j < nb) {
        if (a[i] <= b[j]) out[m++] = a[i++];
        else               out[m++] = b[j++];
    }
    while (i < na) out[m++] = a[i++];
    while (j < nb) out[m++] = b[j++];
    return out;
}

/* ---------------------------------------------------------------------
 * METHOD 1: sequential / repeated merge.
 * arrays[i] has length n for all i = 0..k-1.
 * Returns a malloc'd array of length k*n.
 * ------------------------------------------------------------------- */
static int *method1_sequential_merge(int **arrays, int k, int n) {
    if (k <= 0) return NULL;

    int *result = (int *)malloc((size_t)n * sizeof(int));
    memcpy(result, arrays[0], (size_t)n * sizeof(int));
    int result_len = n;

    for (int i = 1; i < k; i++) {
        int *merged = merge_two(result, result_len, arrays[i], n);
        free(result);
        result = merged;
        result_len += n;
    }
    return result; /* length k*n */
}

/* ---------------------------------------------------------------------
 * METHOD 2: pairwise (divide and conquer) merge, implemented
 * bottom-up/iteratively with a working list of (pointer,length) pairs.
 * Each round merges consecutive pairs; an unpaired leftover (when the
 * count is odd) is carried over untouched to the next round.
 * ------------------------------------------------------------------- */
static int *method2_pairwise_merge(int **arrays, int k, int n) {
    if (k <= 0) return NULL;

    /* working arrays of pointers + lengths, copy so we can free freely */
    int **cur = (int **)malloc((size_t)k * sizeof(int *));
    int  *len = (int  *)malloc((size_t)k * sizeof(int));
    for (int i = 0; i < k; i++) {
        cur[i] = (int *)malloc((size_t)n * sizeof(int));
        memcpy(cur[i], arrays[i], (size_t)n * sizeof(int));
        len[i] = n;
    }
    int count = k;

    while (count > 1) {
        int newCount = (count + 1) / 2;
        int **next = (int **)malloc((size_t)newCount * sizeof(int *));
        int  *nlen = (int  *)malloc((size_t)newCount * sizeof(int));

        int idx = 0;
        int i = 0;
        for (; i + 1 < count; i += 2) {
            next[idx] = merge_two(cur[i], len[i], cur[i + 1], len[i + 1]);
            nlen[idx] = len[i] + len[i + 1];
            free(cur[i]);
            free(cur[i + 1]);
            idx++;
        }
        if (i < count) {              /* odd one out: carry forward */
            next[idx] = cur[i];
            nlen[idx] = len[i];
            idx++;
        }
        free(cur);
        free(len);
        cur = next;
        len = nlen;
        count = newCount;
    }

    int *result = cur[0];
    free(cur);
    free(len);
    return result; /* length k*n */
}

/* ---------------------------------------------------------------------
 * Helpers for validation & data generation
 * ------------------------------------------------------------------- */
static int is_sorted(const int *a, int len) {
    for (int i = 1; i < len; i++)
        if (a[i - 1] > a[i]) return 0;
    return 1;
}

static int cmp_int(const void *a, const void *b) {
    return (*(const int *)a - *(const int *)b);
}

/* generate k sorted arrays of n random ints each */
static int **generate_arrays(int k, int n) {
    int **arrays = (int **)malloc((size_t)k * sizeof(int *));
    for (int i = 0; i < k; i++) {
        arrays[i] = (int *)malloc((size_t)n * sizeof(int));
        for (int j = 0; j < n; j++)
            arrays[i][j] = rand() % 1000000;
        qsort(arrays[i], (size_t)n, sizeof(int), cmp_int);
    }
    return arrays;
}

static void free_arrays(int **arrays, int k) {
    for (int i = 0; i < k; i++) free(arrays[i]);
    free(arrays);
}

/* ---------------------------------------------------------------------
 * VALIDATION: run both methods on the same random input, cross-check
 * against each other and against a reference qsort of the concatenated
 * data. Also checks the output is sorted and of the right length.
 * ------------------------------------------------------------------- */
static void validate(int trials) {
    printf("=====================================================\n");
    printf(" VALIDATION  (checking both methods against each other\n");
    printf("  and against a reference qsort, over %d random trials)\n", trials);
    printf("=====================================================\n");

    int all_ok = 1;
    for (int t = 0; t < trials; t++) {
        int k = 2 + rand() % 15;   /* k in [2,16]   */
        int n = 1 + rand() % 50;   /* n in [1,50]   */

        int **arrays = generate_arrays(k, n);

        int *r1 = method1_sequential_merge(arrays, k, n);
        int *r2 = method2_pairwise_merge(arrays, k, n);

        /* reference: concatenate everything and qsort it */
        int total = k * n;
        int *ref = (int *)malloc((size_t)total * sizeof(int));
        int p = 0;
        for (int i = 0; i < k; i++)
            for (int j = 0; j < n; j++)
                ref[p++] = arrays[i][j];
        qsort(ref, (size_t)total, sizeof(int), cmp_int);

        int ok = 1;
        if (!is_sorted(r1, total)) { ok = 0; printf("  [FAIL] trial %d: method1 output not sorted\n", t); }
        if (!is_sorted(r2, total)) { ok = 0; printf("  [FAIL] trial %d: method2 output not sorted\n", t); }
        if (memcmp(r1, ref, (size_t)total * sizeof(int)) != 0) { ok = 0; printf("  [FAIL] trial %d: method1 != reference\n", t); }
        if (memcmp(r2, ref, (size_t)total * sizeof(int)) != 0) { ok = 0; printf("  [FAIL] trial %d: method2 != reference\n", t); }
        if (memcmp(r1, r2, (size_t)total * sizeof(int)) != 0)  { ok = 0; printf("  [FAIL] trial %d: method1 != method2\n", t); }

        if (!ok) all_ok = 0;

        free(r1); free(r2); free(ref);
        free_arrays(arrays, k);
    }

    if (all_ok)
        printf("  ALL %d TRIALS PASSED: both methods produce a correctly\n"
               "  sorted array identical to the reference solution.\n", trials);
    printf("=====================================================\n\n");
}

/* ---------------------------------------------------------------------
 * BENCHMARKING: time both methods and log results to CSV.
 *
 * Adaptive repeat-until-threshold timing: keep re-running the merge
 * until the cumulative elapsed time exceeds MIN_BENCH_TIME_SEC, then
 * report the average per-call time. This self-adjusts the repeat count
 * to whatever the machine/timer needs -- very fast configurations
 * (small k, small n) automatically get many more repeats than slow
 * ones, so every reported time is a stable average rather than a
 * single near-zero (and therefore noisy/quantized) sample.
 * ------------------------------------------------------------------- */
#define MIN_BENCH_TIME_SEC 0.05   /* keep measuring until >= 50 ms elapsed */
#define MAX_REPEATS        20000  /* safety cap so pathological cases can't spin forever */

static double time_method1(int **arrays, int k, int n) {
    int repeats = 0;
    double start = get_time_sec();
    double elapsed = 0.0;
    do {
        int *res = method1_sequential_merge(arrays, k, n);
        free(res);
        repeats++;
        elapsed = get_time_sec() - start;
    } while (elapsed < MIN_BENCH_TIME_SEC && repeats < MAX_REPEATS);
    return (elapsed / repeats) * 1000.0; /* average time per call, in ms */
}

static double time_method2(int **arrays, int k, int n) {
    int repeats = 0;
    double start = get_time_sec();
    double elapsed = 0.0;
    do {
        int *res = method2_pairwise_merge(arrays, k, n);
        free(res);
        repeats++;
        elapsed = get_time_sec() - start;
    } while (elapsed < MIN_BENCH_TIME_SEC && repeats < MAX_REPEATS);
    return (elapsed / repeats) * 1000.0; /* average time per call, in ms */
}

int main(void) {
    srand(42);

    validate(200);

    FILE *csv = fopen("merging_k_sorted_arrays.csv", "w");
    if (!csv) { fprintf(stderr, "could not open csv file for writing\n"); return 1; }
    fprintf(csv, "series,k,n,method1_time_ms,method2_time_ms\n");

    printf("=====================================================\n");
    printf(" BENCHMARK 1: n fixed, k varies (exposes the k^2 vs k*log(k)\n");
    printf(" growth difference)\n");
    printf("=====================================================\n");
    printf("%6s %8s %18s %18s\n", "k", "n", "Method1 (ms)", "Method2 (ms)");
    {
        int n = 400;
        int ks[] = {2, 4, 8, 16, 32, 64, 96, 128, 160, 192, 224, 256};
        int nk = (int)(sizeof(ks) / sizeof(ks[0]));
        for (int idx = 0; idx < nk; idx++) {
            int k = ks[idx];
            int **arrays = generate_arrays(k, n);
            double t1 = time_method1(arrays, k, n);
            double t2 = time_method2(arrays, k, n);
            printf("%6d %8d %18.4f %18.4f\n", k, n, t1, t2);
            fprintf(csv, "vary_k,%d,%d,%.6f,%.6f\n", k, n, t1, t2);
            free_arrays(arrays, k);
        }
    }

    printf("\n=====================================================\n");
    printf(" BENCHMARK 2: k fixed, n varies (both methods scale linearly\n");
    printf(" in n, so this isolates the n-dependence)\n");
    printf("=====================================================\n");
    printf("%6s %8s %18s %18s\n", "k", "n", "Method1 (ms)", "Method2 (ms)");
    {
        int k = 64;
        int ns[] = {50, 100, 200, 400, 800, 1600, 3200, 6400};
        int nn = (int)(sizeof(ns) / sizeof(ns[0]));
        for (int idx = 0; idx < nn; idx++) {
            int n = ns[idx];
            int **arrays = generate_arrays(k, n);
            double t1 = time_method1(arrays, k, n);
            double t2 = time_method2(arrays, k, n);
            printf("%6d %8d %18.4f %18.4f\n", k, n, t1, t2);
            fprintf(csv, "vary_n,%d,%d,%.6f,%.6f\n", k, n, t1, t2);
            free_arrays(arrays, k);
        }
    }

    fclose(csv);

    printf("\nBenchmark data written to merging_k_sorted_arrays.csv\n");

    printf("\n=====================================================\n");
    printf(" THEORETICAL CONCLUSION\n");
    printf("=====================================================\n");
    printf(" Method 1 (sequential merge) worst-case time : O(n * k^2)\n");
    printf(" Method 2 (pairwise merge)   worst-case time : O(n * k * log2 k)\n");
    printf("\n");
    printf(" For fixed n, as k grows, Method 1's time grows quadratically\n");
    printf(" while Method 2's grows only as k*log(k). The CSV/plot shows\n");
    printf(" this gap widening exactly as predicted: Method 2 becomes\n");
    printf(" dramatically faster once k is even moderately large, while\n");
    printf(" for very small k (e.g. k=2) the two methods coincide (a\n");
    printf(" single merge either way).\n");
    printf("=====================================================\n");

    return 0;
}