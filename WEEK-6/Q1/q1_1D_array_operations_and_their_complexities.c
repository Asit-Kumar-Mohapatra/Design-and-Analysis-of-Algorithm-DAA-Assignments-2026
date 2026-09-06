/* =====================================================================
 * 1-D ARRAY OPERATIONS AND THEIR WORST-CASE-OPTIMAL COMPLEXITIES
 * =====================================================================
 * Input representation
 * ---------------------
 * The array is a plain C `int arr[]` allocated dynamically to size n
 * (n supplied by the user at runtime) and stored in contiguous memory.
 * This is the correct choice because:
 *   - Every sub-problem (max, mean, median, mode, dedup, reverse,
 *     partition) needs random-access indexing (arr[i]) and/or in-place
 *     swapping -> a contiguous array gives O(1) access and O(1) swap,
 *     which a linked list cannot give for the selection/partition
 *     algorithms below.
 *   - "Unsorted integers" with no stated bound on value range rules out
 *     pure counting-sort/bucket tricks as the *worst-case* solution
 *     (they would only be O(n) if the value range is O(n); we do not
 *     assume that), so comparison-based / hashing-based optimal
 *     algorithms are used instead and are noted where relevant.
 *
 * Each routine below is preceded by a comment block that states:
 *   THEORETICAL LOWER BOUND, ALGORITHM USED, and WHY IT IS OPTIMAL.
 *
 * Compile:  gcc -O2 -Wall -std=c11 array_ops.c -o array_ops -lm
 * Run    :  ./array_ops
 *           (then type n, followed by n integers, at the prompts)
 * ===================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <limits.h>

/* --------------------------------------------------------------------
 * Small helpers
 * ------------------------------------------------------------------ */
static void swapInt(int *a, int *b) { int t = *a; *a = *b; *b = t; }

static int *cloneArray(const int *arr, int n) {
    int *copy = (int *)malloc((size_t)n * sizeof(int));
    if (!copy) { fprintf(stderr, "malloc failed\n"); exit(EXIT_FAILURE); }
    memcpy(copy, arr, (size_t)n * sizeof(int));
    return copy;
}

static void printArray(const char *label, const int *arr, int n) {
    printf("%s [", label);
    for (int i = 0; i < n; i++) printf("%d%s", arr[i], (i + 1 < n) ? ", " : "");
    printf("]\n");
}

/* =====================================================================
 * (i) MAXIMUM ELEMENT                              WORST CASE: THETA(n)
 * ---------------------------------------------------------------------
 * Lower bound: every element must be examined at least once, because an
 * unexamined element could be the true maximum -> Omega(n). A single
 * linear scan keeping a running max achieves O(n). Hence THETA(n) is
 * optimal; no comparison-based algorithm can do better.
 * ===================================================================== */
int findMax(const int *arr, int n) {
    int best = arr[0];
    for (int i = 1; i < n; i++)
        if (arr[i] > best) best = arr[i];
    return best;                       /* n-1 comparisons total */
}

/* =====================================================================
 * (ii) FIRST AND SECOND LARGEST                    WORST CASE: THETA(n)
 * ---------------------------------------------------------------------
 * NOTE ON DEFINITION: "second largest" here means the second element
 * in descending sorted order (duplicates count separately), i.e. for
 * [9,9,7] the answer is 9 and 9 -- not the second *distinct* value.
 * If you need the second-largest DISTINCT value instead, track a
 * third variable and only update `second` when arr[i] != *first.
 *
 * Naive approach: find max (n-1 cmp), remove it, find max again (n-2
 * cmp) -> 2n-3 comparisons, still THETA(n) but wasteful.
 * Optimal approach used here: ONE linear pass that maintains
 * (first, second) simultaneously, using at most 2 comparisons per
 * element -> ~2n comparisons in a single traversal (still one pass,
 * same asymptotic class, better constant / cache behaviour than two
 * passes). Lower bound is still Omega(n) since every element must be
 * inspected (an unseen element could beat "second").
 * A classical tournament-method can push the comparison COUNT down to
 * n + ceil(log2 n) - 2 (information-theoretic optimum for 2nd-largest),
 * but that does not change the TIME complexity class, only the
 * constant factor, so the single linear pass below is the practical
 * optimum for THETA(n) time / O(1) space.
 * ===================================================================== */
void firstAndSecondLargest(const int *arr, int n, int *first, int *second) {
    if (arr[0] > arr[1]) { *first = arr[0]; *second = arr[1]; }
    else                 { *first = arr[1]; *second = arr[0]; }

    for (int i = 2; i < n; i++) {
        if (arr[i] > *first) {
            *second = *first;
            *first  = arr[i];
        } else if (arr[i] > *second) {
            *second = arr[i];
        }
    }
}

/* =====================================================================
 * (iii) MEAN                                       WORST CASE: THETA(n)
 * ---------------------------------------------------------------------
 * Every element contributes to the sum, so all n must be read ->
 * Omega(n); a single accumulation pass gives O(n). THETA(n) optimal.
 * ===================================================================== */
double findMean(const int *arr, int n) {
    long long sum = 0;                 /* long long avoids overflow */
    for (int i = 0; i < n; i++) sum += arr[i];
    return (double)sum / (double)n;
}

/* =====================================================================
 * (v) STANDARD DEVIATION (documented here, next to mean, computed
 *     after mean is known)                         WORST CASE: THETA(n)
 * ---------------------------------------------------------------------
 * Population std-dev needs sum((x_i - mean)^2) over all n elements ->
 * a second Omega(n) pass is unavoidable (mean itself needs a full pass
 * before any deviation can be computed). Two sequential O(n) passes
 * still compose to THETA(n) overall -> optimal.
 * ===================================================================== */
double findStdDev(const int *arr, int n, double mean) {
    double sumSqDiff = 0.0;
    for (int i = 0; i < n; i++) {
        double diff = arr[i] - mean;
        sumSqDiff += diff * diff;
    }
    return sqrt(sumSqDiff / n);         /* population std. deviation */
}

/* =====================================================================
 * INTERNAL: median-of-medians (BFPRT) deterministic SELECT
 * ---------------------------------------------------------------------
 * Used by (iv) MEDIAN below to guarantee worst-case linear time
 * (a naive quickselect with a fixed/random pivot degrades to O(n^2) in
 * the worst case -> not acceptable for a WORST-CASE analysis).
 * insertionSort is used only on the tiny (<=5 element) groups, so its
 * O(k^2) cost is O(1) per group. Group medians are gathered into a
 * small auxiliary buffer (O(n/5) total across a level, O(log n) tiny
 * allocations across the whole recursion) -- this keeps the routine
 * simple and easy to verify while remaining strictly O(n) overall,
 * since T(n) = T(n/5) + T(7n/10) + O(n) still solves to O(n) however
 * the O(n) partitioning/gathering work is laid out.
 * ===================================================================== */
static void insertionSortSmall(int *arr, int left, int right) {
    for (int i = left + 1; i <= right; i++) {
        int key = arr[i], j = i - 1;
        while (j >= left && arr[j] > key) { arr[j + 1] = arr[j]; j--; }
        arr[j + 1] = key;
    }
}

static int medianOfMedians(int *arr, int left, int right); /* fwd */

/* Lomuto partition around a given VALUE (the pivot is first located
 * and swapped to the end, then partitioned in one linear pass). */
static int partitionByValue(int *arr, int left, int right, int pivotValue) {
    int pivotIndex = left;
    for (int i = left; i <= right; i++)
        if (arr[i] == pivotValue) { pivotIndex = i; break; }
    swapInt(&arr[pivotIndex], &arr[right]);

    int store = left;
    for (int i = left; i < right; i++) {
        if (arr[i] < pivotValue) { swapInt(&arr[i], &arr[store]); store++; }
    }
    swapInt(&arr[store], &arr[right]);
    return store;                       /* final index of pivot */
}

static int medianOfMedians(int *arr, int left, int right) {
    int n = right - left + 1;
    if (n <= 5) {
        insertionSortSmall(arr, left, right);
        return arr[left + n / 2];
    }
    int numGroups = (n + 4) / 5;
    int *medians = (int *)malloc((size_t)numGroups * sizeof(int));
    if (!medians) { fprintf(stderr, "malloc failed\n"); exit(EXIT_FAILURE); }
    for (int g = 0; g < numGroups; g++) {
        int subLeft  = left + g * 5;
        int subRight = subLeft + 4 <= right ? subLeft + 4 : right;
        insertionSortSmall(arr, subLeft, subRight);
        medians[g] = arr[subLeft + (subRight - subLeft) / 2];
    }
    int mom = medianOfMedians(medians, 0, numGroups - 1);
    free(medians);
    return mom;
}

/* k is 0-indexed rank (0 = smallest). Guaranteed THETA(n) worst case. */
static int quickSelect(int *arr, int left, int right, int k) {
    while (left < right) {
        int pivotValue = medianOfMedians(arr, left, right);
        int p = partitionByValue(arr, left, right, pivotValue);
        if (k == p) return arr[p];
        else if (k < p) right = p - 1;
        else left = p + 1;
    }
    return arr[left];
}

/* =====================================================================
 * (iv) MEDIAN                                      WORST CASE: THETA(n)
 * ---------------------------------------------------------------------
 * Sorting first gives O(n log n); that is NOT optimal. The information
 * lower bound for "find the median" is Omega(n) (you must inspect
 * every element -- an unread element could be the median), and the
 * deterministic SELECT algorithm above (median-of-medians pivot,
 * "groups of 5") achieves this bound with GUARANTEED worst-case
 * T(n) = T(n/5) + T(7n/10) + O(n) = O(n), unlike ordinary quickselect
 * which is only O(n) on AVERAGE and O(n^2) worst case.
 * Operates on a private copy so the caller's array order is untouched.
 * ===================================================================== */
double findMedian(const int *arr, int n) {
    int *tmp = cloneArray(arr, n);
    double result;
    if (n % 2 == 1) {
        result = quickSelect(tmp, 0, n - 1, n / 2);
    } else {
        int lo = quickSelect(tmp, 0, n - 1, n / 2 - 1);
        /* array is now partially reordered but the lower half (indices
         * 0..n/2-1) is guaranteed <= lo, so the upper median is simply
         * the minimum of the remaining n/2..n-1 slice -> one more O(n) pass */
        int hi = tmp[n / 2];
        for (int i = n / 2 + 1; i < n; i++) if (tmp[i] < hi) hi = tmp[i];
        result = (lo + hi) / 2.0;
    }
    free(tmp);
    return result;                      /* two O(n) selects -> still THETA(n) */
}

/* =====================================================================
 * (vi) MODE                                  WORST CASE: THETA(n log n)
 * ---------------------------------------------------------------------
 * With arbitrary (unbounded-range) integers, comparison-based sorting
 * is the safe worst-case-optimal route: qsort -> THETA(n log n), then
 * one linear scan of the sorted array counts run-lengths and tracks
 * the longest run -> O(n). Total: THETA(n log n).
 * (A hash table gives THETA(n) on AVERAGE, but its WORST case degrades
 * to O(n^2) under adversarial collisions unless universal hashing with
 * amortized guarantees is used, so it is not the safe worst-case
 * answer; it is mentioned here only as the practical average-case
 * alternative.) Ties are broken by "first run of maximal length
 * encountered in sorted order", matching the ascending value order.
 * ===================================================================== */
static int cmpInt(const void *a, const void *b) { return (*(const int *)a - *(const int *)b); }

int findMode(const int *arr, int n) {
    int *tmp = cloneArray(arr, n);
    qsort(tmp, n, sizeof(int), cmpInt);          /* THETA(n log n) */

    int modeValue = tmp[0], bestCount = 1, curCount = 1;
    for (int i = 1; i < n; i++) {
        if (tmp[i] == tmp[i - 1]) curCount++;
        else curCount = 1;
        if (curCount > bestCount) { bestCount = curCount; modeValue = tmp[i]; }
    }
    free(tmp);
    return modeValue;                            /* first mode found; ties possible */
}

/* =====================================================================
 * (vii) REMOVING ALL DUPLICATES               WORST CASE: THETA(n log n)
 * ---------------------------------------------------------------------
 * Sort THETA(n log n), then a single forward pass copies each element
 * that differs from the previous one -> O(n). Total THETA(n log n),
 * done fully in place using O(1) extra space (the result occupies a
 * prefix of the same array). This is worst-case optimal for a
 * comparison-based method with unbounded integer values; a hash-set
 * approach gives O(n) on average while preserving original order, but
 * (like mode) is only average-case linear, not worst-case.
 * Returns the new length; arr[0..newLen-1] holds the unique values in
 * SORTED order.
 * ===================================================================== */
int removeDuplicatesInPlace(int *arr, int n) {
    if (n == 0) return 0;
    qsort(arr, n, sizeof(int), cmpInt);           /* THETA(n log n) */
    int writeIdx = 1;
    for (int i = 1; i < n; i++) {
        if (arr[i] != arr[writeIdx - 1]) {
            arr[writeIdx] = arr[i];
            writeIdx++;
        }
    }
    return writeIdx;                              /* O(n) compaction pass */
}

/* =====================================================================
 * (viii) REVERSING THE ARRAY                       WORST CASE: THETA(n)
 * ---------------------------------------------------------------------
 * Every one of the n elements must move to its mirrored slot ->
 * Omega(n). Two-pointer in-place swap touches each element exactly
 * once (n/2 swaps) -> O(n), and needs only O(1) extra space. THETA(n)
 * optimal, and this is the best possible extra-space bound too.
 * ===================================================================== */
void reverseArray(int *arr, int n) {
    int lo = 0, hi = n - 1;
    while (lo < hi) { swapInt(&arr[lo], &arr[hi]); lo++; hi--; }
}

/* =====================================================================
 * (ix) PARTITION AROUND A RANDOM PIVOT VALUE       WORST CASE: THETA(n)
 * ---------------------------------------------------------------------
 * Requirement: after partitioning, every element >= pivot must appear
 * BEFORE every element < pivot (reverse of the usual quicksort
 * layout). Every element must be classified at least once -> Omega(n).
 * A single sweep classifies and swaps each element exactly once ->
 * O(n) time, O(1) extra space. THETA(n) optimal; this is exactly the
 * same cost class as one quicksort partition step. Returns the index
 * of the first element belonging to the "< pivot" group (the split
 * point).
 * ===================================================================== */
int partitionAroundPivot(int *arr, int n, int pivotValue) {
    int store = 0;                       /* next slot for a ">= pivot" element */
    for (int i = 0; i < n; i++) {
        if (arr[i] >= pivotValue) { swapInt(&arr[i], &arr[store]); store++; }
    }
    return store;                        /* [0, store)=">=pivot", [store,n)="<pivot" */
}

/* =====================================================================
 * EXPLANATION / COMPLEXITY-DERIVATION FUNCTION
 * ---------------------------------------------------------------------
 * This turns the "theoretical lower bound -> algorithm -> why it meets
 * the bound" reasoning that sits above each function into something
 * the PROGRAM ITSELF states at runtime, instead of leaving it only in
 * comments the user never sees when the binary runs. One small helper
 * per operation prints: the Omega(.) lower bound argument, the
 * algorithm used, why that algorithm attains the bound, and the
 * resulting Theta(.) class. Keeping each explanation in its own
 * function (rather than one giant printf block) means the reasoning
 * for a single operation can be re-printed or reused independently
 * (e.g. a caller could print just explainMedian() on its own).
 * ===================================================================== */
static void explainMax(void) {
    printf("(i) Maximum element -- Theta(n)\n"
           "    Lower bound : an unexamined element could secretly be the\n"
           "                  true max -> every element must be read at\n"
           "                  least once -> Omega(n).\n"
           "    Algorithm   : one linear scan, running max, n-1 comparisons.\n"
           "    Optimal?    : yes -- the O(n) scan meets the Omega(n) bound.\n\n");
}

static void explainFirstSecond(void) {
    printf("(ii) First & second largest -- Theta(n)\n"
           "    Lower bound : same read-every-element argument -> Omega(n),\n"
           "                  since an unseen element could beat \"second\".\n"
           "    Algorithm   : single pass maintaining (first, second) with\n"
           "                  at most 2 comparisons per element.\n"
           "    Optimal?    : yes -- meets Omega(n); a tournament method can\n"
           "                  cut the comparison COUNT to the information-\n"
           "                  theoretic optimum n + ceil(log2 n) - 2, but that\n"
           "                  only changes the constant, not the Theta(n) class.\n\n");
}

static void explainMean(void) {
    printf("(iii) Mean -- Theta(n)\n"
           "    Lower bound : the sum depends on every value; omitting one\n"
           "                  changes the result -> Omega(n).\n"
           "    Algorithm   : single accumulation pass.\n"
           "    Optimal?    : yes -- O(n) meets Omega(n).\n\n");
}

static void explainMedian(void) {
    printf("(iv) Median -- Theta(n)\n"
           "    Lower bound : Omega(n), same read-every-element argument.\n"
           "    Naive path  : sort first -> Theta(n log n) -- does NOT meet\n"
           "                  the Omega(n) bound, so it is not optimal.\n"
           "    Algorithm   : deterministic median-of-medians (BFPRT) select.\n"
           "                  Groups-of-5 pivoting gives the recurrence\n"
           "                  T(n) = T(ceil(n/5)) + T(<=7n/10) + O(n), which\n"
           "                  solves to T(n) = O(n) -- GUARANTEED, unlike\n"
           "                  ordinary quickselect whose O(n) is only average\n"
           "                  case (worst case O(n^2)).\n"
           "    Optimal?    : yes -- O(n) meets Omega(n).\n\n");
}

static void explainStdDev(void) {
    printf("(v) Standard deviation -- Theta(n)\n"
           "    Lower bound : Omega(n), same as mean; the mean must be fully\n"
           "                  computed BEFORE any deviation can be found, so\n"
           "                  a second full pass is unavoidable.\n"
           "    Algorithm   : pass 1 computes mean O(n); pass 2 computes\n"
           "                  sum((x_i-mean)^2) O(n).\n"
           "    Optimal?    : yes -- two sequential O(n) passes still compose\n"
           "                  to Theta(n); constants don't change the class.\n\n");
}

static void explainMode(void) {
    printf("(vi) Mode -- Theta(n log n)\n"
           "    Lower bound : value range is UNBOUNDED, so O(n) counting\n"
           "                  cannot be assumed safe in the worst case;\n"
           "                  ordering the elements first needs a comparison\n"
           "                  sort, whose worst-case lower bound is\n"
           "                  Omega(n log n).\n"
           "    Algorithm   : qsort (Theta(n log n)) + one linear scan\n"
           "                  tracking the longest run (O(n)).\n"
           "    Optimal?    : yes for the worst case. A hash table gives\n"
           "                  Theta(n) on AVERAGE, but degrades to O(n^2)\n"
           "                  under adversarial collisions without universal\n"
           "                  hashing guarantees, so it is not the safe\n"
           "                  worst-case answer.\n\n");
}

static void explainDedup(void) {
    printf("(vii) Removing duplicates -- Theta(n log n)\n"
           "    Lower bound : same unbounded-range argument as mode ->\n"
           "                  Omega(n log n) for a safe worst-case method.\n"
           "    Algorithm   : sort (Theta(n log n)) + one compaction pass\n"
           "                  keeping elements that differ from the previous\n"
           "                  one (O(n)), done in place with O(1) extra space.\n"
           "    Optimal?    : yes -- Theta(n log n) + Theta(n) = Theta(n log n).\n\n");
}

static void explainReverse(void) {
    printf("(viii) Reversing the array -- Theta(n)\n"
           "    Lower bound : every element must move to its mirrored slot\n"
           "                  -> Omega(n).\n"
           "    Algorithm   : two-pointer in-place swap, n/2 swaps, O(1)\n"
           "                  extra space.\n"
           "    Optimal?    : yes -- O(n) meets Omega(n), and O(1) space is\n"
           "                  also the best possible extra-space bound.\n\n");
}

static void explainPartition(void) {
    printf("(ix) Partition around a given pivot -- Theta(n)\n"
           "    Lower bound : every element must be classified as >=pivot\n"
           "                  or <pivot at least once -> Omega(n).\n"
           "    Algorithm   : one sweep with a \"next slot for >=pivot\"\n"
           "                  pointer; one swap per qualifying element.\n"
           "    Optimal?    : yes -- O(n) meets Omega(n), same cost class\n"
           "                  as one quicksort partition step.\n\n");
}

/* Prints the full derivation, operation by operation, then a compact
 * summary table. Calling the per-operation functions individually
 * (rather than inlining everything here) is what lets a caller show
 * just one explanation on its own if that is ever useful. */
static void printComplexityAnalysis(void) {
    printf("\n================ COMPLEXITY ANALYSIS (LOWER BOUND -> ALGORITHM -> WHY OPTIMAL) ================\n\n");
    explainMax();
    explainFirstSecond();
    explainMean();
    explainMedian();
    explainStdDev();
    explainMode();
    explainDedup();
    explainReverse();
    explainPartition();

    printf("================ SUMMARY TABLE ================\n");
    printf("(i)    Maximum element               : Theta(n)\n");
    printf("(ii)   First & second largest         : Theta(n)\n");
    printf("(iii)  Mean                           : Theta(n)\n");
    printf("(iv)   Median                         : Theta(n)   [median-of-medians select]\n");
    printf("(v)    Standard deviation             : Theta(n)\n");
    printf("(vi)   Mode                           : Theta(n log n)\n");
    printf("(vii)  Removing all duplicates         : Theta(n log n)\n");
    printf("(viii) Reversing the array             : Theta(n)\n");
    printf("(ix)   Partition around random pivot   : Theta(n)\n");
}

/* --------------------------------------------------------------------
 * Robust input reading: rejects non-integer tokens and short input
 * instead of silently continuing with garbage / undefined behaviour.
 * ------------------------------------------------------------------ */
static int readInt(const char *prompt, int *out) {
    if (prompt) printf("%s", prompt);
    if (scanf("%d", out) != 1) {
        fprintf(stderr, "\nInvalid input: expected an integer.\n");
        return 0;
    }
    return 1;
}

/* =====================================================================
 * DRIVER: reads n and the n array elements from the user, runs every
 * routine on the same input, and reports the worst-case complexity of
 * each alongside its result.
 * ===================================================================== */
#ifndef UNIT_TEST
int main(void) {
    int n;
    printf("================ 1D ARRAY OPERATIONS ================\n");
    if (!readInt("Enter the number of elements (n): ", &n)) return EXIT_FAILURE;
    if (n <= 0) {
        fprintf(stderr, "n must be a positive integer.\n");
        return EXIT_FAILURE;
    }

    int *arr = (int *)malloc((size_t)n * sizeof(int));
    if (!arr) { fprintf(stderr, "malloc failed\n"); return EXIT_FAILURE; }

    printf("Enter %d integer%s (space or newline separated):\n", n, n == 1 ? "" : "s");
    for (int i = 0; i < n; i++) {
        if (!readInt(NULL, &arr[i])) { free(arr); return EXIT_FAILURE; }
    }

    printf("\n");
    printArray("Input array:  ", arr, n);
    printf("n = %d\n\n", n);

    /* (i) */
    printf("(i)   Max element                  : %d\n", findMax(arr, n));

    /* (ii) — needs at least 2 elements to be meaningful */
    if (n >= 2) {
        int first, second;
        firstAndSecondLargest(arr, n, &first, &second);
        printf("(ii)  1st largest / 2nd largest     : %d / %d\n", first, second);
    } else {
        printf("(ii)  1st largest / 2nd largest     : n/a (need n >= 2)\n");
    }

    /* (iii) */
    double mean = findMean(arr, n);
    printf("(iii) Mean                          : %.4f\n", mean);

    /* (iv) */
    printf("(iv)  Median                        : %.4f\n", findMedian(arr, n));

    /* (v) */
    printf("(v)   Standard deviation            : %.4f\n", findStdDev(arr, n, mean));

    /* (vi) */
    printf("(vi)  Mode                          : %d\n", findMode(arr, n));

    /* (vii) */
    {
        int *work = cloneArray(arr, n);
        int newLen = removeDuplicatesInPlace(work, n);
        printArray("(vii) Unique elements (sorted)     :", work, newLen);
        free(work);
    }

    /* (viii) */
    {
        int *work = cloneArray(arr, n);
        reverseArray(work, n);
        printArray("(viii) Reversed array               :", work, n);
        free(work);
    }

    /* (ix) — pivot chosen at random from the array's own values, as
     * required by the problem statement ("a given random pivot element") */
    {
        int *work = cloneArray(arr, n);
        srand((unsigned)time(NULL) ^ (unsigned)(size_t)arr);
        int pivotValue = arr[rand() % n];
        int splitIdx = partitionAroundPivot(work, n, pivotValue);
        printf("(ix)  Random pivot value chosen     : %d\n", pivotValue);
        printArray("      Partitioned (>=pivot | <pivot) :", work, n);
        printf("      Split index (count >= pivot)  : %d\n", splitIdx);
        free(work);
    }

    printComplexityAnalysis();

    free(arr);
    return 0;
}
#endif