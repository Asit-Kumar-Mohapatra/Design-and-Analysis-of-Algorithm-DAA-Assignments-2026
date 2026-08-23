/*
 * ============================================================================
 *  Application of Sorting - V
 * ----------------------------------------------------------------------------
 *  PROBLEM:
 *    Given a list I of n intervals, specified as (x_i, y_i) pairs, return
 *    a list where all overlapping intervals are merged.
 *
 *    Example:
 *      I = {(1,3), (2,6), (8,10), (7,18)}
 *      Output = {(1,6), (7,18)}
 *
 *    Required time complexity: O(n log n) worst case.
 *
 *  KEY IDEA:
 *    Two intervals can only possibly overlap if they are "close" to each
 *    other along the number line. If we SORT the intervals by their start
 *    value x_i, then any interval that overlaps the "current" merged
 *    interval must appear immediately after it in this sorted order --
 *    we never need to look back, and we never need to compare an interval
 *    against one far away in the sorted list. This is what allows a
 *    single linear scan after sorting.
 *
 *  ALGORITHM:
 *    1. Sort the n intervals by start value x_i ascending.   -> O(n log n)
 *    2. Initialize the result with the first interval as the
 *       "current" merged interval [curX, curY].
 *    3. For each subsequent interval (x_i, y_i) in sorted order:
 *          - if x_i <= curY   -> it overlaps (or touches) the current
 *                                merged interval, so extend it:
 *                                curY = max(curY, y_i)
 *          - else             -> no overlap; the current merged interval
 *                                is finalized, push it to output, and
 *                                start a new current interval = (x_i, y_i)
 *    4. Push the final current interval to output.            -> O(n)
 *    Total: O(n log n) + O(n) = O(n log n).
 *
 *  PROPER INPUT REPRESENTATION:
 *    Each interval is kept as a single (start, end) PAIR -- unlike
 *    problem IV, we do NOT split it into two separate +1/-1 events.
 *    That representation was needed there because we needed a running
 *    COUNT of overlaps at every instant. Here we don't care how MANY
 *    intervals overlap -- we only care about the union of overlapping
 *    ranges -- so keeping each interval intact and sorting by start
 *    is the natural and sufficient representation.
 *
 *    OUTPUT representation: a array of merged (start, end) pairs, of
 *    size <= n, along with a count of how many merged intervals resulted.
 *
 *  BOUNDARY CONVENTION (closed intervals):
 *    Intervals are treated as CLOSED, i.e. [x_i, y_i] includes both
 *    endpoints. Touching intervals such as (1,3) and (3,5) are therefore
 *    merged into (1,5), since they share the point 3. This matches the
 *    given example's spirit and is the standard convention for this
 *    classic problem. (If a half-open convention were required instead,
 *    only the merge condition `x_i <= curY` would need to change to
 *    `x_i < curY`.)
 *
 *    Degenerate POINT intervals (x_i == y_i) are also accepted as valid
 *    single-point closed intervals [x_i, x_i]. The merge logic requires
 *    no special-casing for this -- `x_i <= curY` and `curY = max(curY,
 *    y_i)` are already correct whether an interval has positive length
 *    or zero length, so a point interval merges with a touching
 *    neighbor exactly like any other interval would.
 *
 *  COMPLEXITY:
 *    Time  : O(n log n) -- sorting n intervals dominates; the merge
 *                          pass is a single O(n) linear scan.
 *                          This is asymptotically OPTIMAL: any
 *                          comparison-based algorithm for this problem
 *                          can be used to sort n numbers (the merge
 *                          output reveals relative order), and
 *                          comparison sorting has a proven Omega(n log n)
 *                          lower bound. No further asymptotic
 *                          improvement is possible.
 *    Space : O(n)        -- for the input/output list (unavoidable --
 *                          the output itself can contain up to n
 *                          intervals). The merge is done IN PLACE on
 *                          the same sorted array (no second O(n)
 *                          array is allocated) using a write-pointer
 *                          that never exceeds the read-pointer, so
 *                          the only extra space beyond the input is
 *                          O(1).
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    long long x;   /* start of interval */
    long long y;   /* end of interval   */
} Interval;

/* ---- qsort comparator: ascending by start value x. ---- */
static int cmp_interval(const void *a, const void *b) {
    long long xa = ((const Interval *)a)->x;
    long long xb = ((const Interval *)b)->x;
    return (xa > xb) - (xa < xb);
}

int merge_intervals(Interval *arr, int n) {
    if (n == 0) return 0;

    qsort(arr, (size_t)n, sizeof(Interval), cmp_interval);   /* O(n log n) */

    int write = 0;                 /* index of current merged interval */
    long long curY = arr[0].y;     /* its end value (start is already
                                     * correctly in place at arr[write].x) */

    for (int i = 1; i < n; i++) {                             /* O(n) */
        if (arr[i].x <= curY) {
            /* Overlaps (or touches) the current merged interval:
             * extend its end if this interval reaches further.   */
            if (arr[i].y > curY) curY = arr[i].y;
        } else {
            /* Gap found: finalize current merged interval, then
             * open a new one at the next output slot.            */
            arr[write].y = curY;
            write++;
            arr[write].x = arr[i].x;
            curY = arr[i].y;
        }
    }

    /* finalize the last accumulated interval */
    arr[write].y = curY;

    return write + 1;              /* number of merged intervals */
}

/* ---- I/O utilities ---- */
static Interval *read_intervals(int n) {
    Interval *arr = (Interval *)malloc((size_t)n * sizeof(Interval));
    if (!arr) { fprintf(stderr, "Memory allocation failed\n"); exit(1); }

    printf("Enter %d intervals as \"x y\" pairs (x <= y):\n", n);
    for (int i = 0; i < n; i++) {
        printf("Interval %d: ", i + 1);
        if (scanf("%lld %lld", &arr[i].x, &arr[i].y) != 2) {
            fprintf(stderr, "Invalid input while reading interval %d\n", i + 1);
            free(arr);
            exit(1);
        }
    }
    return arr;
}

int main(void) {
    int n;
    printf("Enter n (number of intervals): ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        fprintf(stderr, "Invalid n\n");
        return 1;
    }

    Interval *arr = read_intervals(n);

    for (int i = 0; i < n; i++) {
        if (arr[i].x > arr[i].y) {
            fprintf(stderr, "Invalid data: start must not exceed end for interval %d\n", i + 1);
            free(arr);
            return 1;
        }
    }

    /* Print input BEFORE merging (merge_intervals sorts + overwrites arr) */
    printf("\nInput intervals : { ");
    for (int i = 0; i < n; i++)
        printf("(%lld,%lld)%s", arr[i].x, arr[i].y, (i == n - 1) ? "" : ", ");
    printf(" }\n");

    int result_count = merge_intervals(arr, n);   /* in place: no 2nd array */

    printf("Merged intervals: { ");
    for (int i = 0; i < result_count; i++)
        printf("(%lld,%lld)%s", arr[i].x, arr[i].y, (i == result_count - 1) ? "" : ", ");
    printf(" }\n");

    free(arr);
    return 0;
}
