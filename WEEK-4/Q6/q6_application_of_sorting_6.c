/*
 * ============================================================================
 *  Application of Sorting - VI
 * ----------------------------------------------------------------------------
 *  PROBLEM:
 *    Given a set S of n intervals on a line, the i-th interval described
 *    by its left and right endpoints (l_i, r_i), find a point p on the
 *    line that lies in the LARGEST number of intervals. An endpoint
 *    counts as being IN its own interval (closed interval [l_i, r_i]).
 *
 *    Example:
 *      S = {(10,40), (20,60), (50,90), (15,70)}
 *      No point lies in all four; p = 50 lies in three: (20,60), (50,90),
 *      (15,70). (Note: p = 20 also lies in exactly three -- (10,40),
 *      (20,60), (15,70) -- so the point of maximum coverage need not be
 *      unique; the question only asks for *a* point achieving the max.)
 *
 *    Required time complexity: O(n log n).
 *
 *  RELATION TO "Application of Sorting - IV" (max simultaneous overlap):
 *    This is the same sweep-line idea as problem IV -- convert each
 *    interval into an arrival (+1) and a departure (-1) event, sort by
 *    coordinate, and sweep while tracking a running count.
 *
 *    BUT there is one crucial difference. In problem IV all 2n event
 *    times were GUARANTEED DISTINCT, so tie-breaking never arose. HERE,
 *    intervals are explicitly CLOSED -- an endpoint belongs to its own
 *    interval -- so two different intervals' endpoints CAN legitimately
 *    coincide (e.g. (1,5) and (5,10) both contain the point 5), and nothing
 *    in the problem statement rules this out. If a departure event and an
 *    arrival event land on the same coordinate and we process the
 *    departure first, the count would momentarily drop BEFORE the
 *    arrival's contribution is added -- silently hiding the true maximum
 *    at that exact coordinate. So here, unlike problem IV, a secondary
 *    sort key is not a cosmetic "robustness" add-on -- it is REQUIRED
 *    for correctness: at equal coordinates, all ARRIVALS must be
 *    processed before any DEPARTURES, so the count correctly reflects
 *    every interval that includes that shared point before the
 *    departures bring it back down.
 *
 *  ALGORITHM:
 *    1. Build 2n events: (l_i, START) and (r_i, END) for each interval.
 *    2. Sort events by:
 *         (a) coordinate ascending                          -- primary
 *         (b) START before END, for equal coordinates        -- secondary
 *                                                       -> O(n log n)
 *    3. Sweep through sorted events, maintaining `count`:
 *          START -> count++; if count > best, record (best, coordinate)
 *          END   -> count--  (a departure can never itself create a new
 *                             maximum, so no check is needed there --
 *                             see justification below)
 *                                                             -> O(n)
 *    Total: O(n log n) + O(n) = O(n log n).
 *
 *  WHY CHECKING ONLY ON "START" EVENTS IS STILL SUFFICIENT:
 *    The running count only ever *increases* on a START event, so any
 *    new maximum can only be set at a START. Because of the secondary
 *    sort (all STARTs at a coordinate processed before any ENDs at that
 *    same coordinate), the count reaches its true local peak at that
 *    coordinate right after the LAST start sharing it is processed --
 *    which is exactly what the "if count > best" check inside the START
 *    branch captures, coincidence or not.
 *
 *  PROPER INPUT REPRESENTATION:
 *    Exactly as in problem IV, each interval becomes two independent
 *    (coordinate, event-type) events rather than being kept as a
 *    [l_i, r_i] pair -- this is what turns "find the point of maximum
 *    interval coverage" into "find the max running sum of a sequence of
 *    +1/-1 signed points", enabling the sort + single sweep. The only
 *    addition versus problem IV is the event TYPE, carried so ties can
 *    be broken correctly (see above).
 *
 *  OUTPUT REPRESENTATION:
 *    The maximum coverage count, and one coordinate p that achieves it
 *    (the first such coordinate encountered in the sweep; any point
 *    achieving the max is an equally valid answer per the problem's own
 *    example, which shows the max point is not claimed to be unique).
 *
 *  COMPLEXITY:
 *    Time  : O(n log n) WORST CASE, GUARANTEED -- sorting the 2n events
 *                          via merge sort dominates (merge sort's
 *                          O(n log n) bound holds for every input, not
 *                          just on average -- see the note above
 *                          merge_sort_events()); the sweep afterward
 *                          is a single O(n) linear pass.
 *    Space : O(n)        -- the 2n events (the necessary restructured
 *                          input) plus a same-sized O(n) temporary
 *                          buffer used only during the merge sort
 *                          (freed immediately after); still O(n)
 *                          overall, just a larger constant than an
 *                          in-place sort would use.
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#define EV_START 0   /* arrival: +1, must be processed first on a tie */
#define EV_END   1   /* departure: -1, processed after starts on a tie */

typedef struct {
    long long time;  /* the coordinate the event occurs at */
    int       type;  /* EV_START or EV_END                 */
} Event;

/*
 * event_less: strict ordering used by the sort.
 *   Primary key   : time, ascending.
 *   Secondary key : type, ascending (EV_START == 0 sorts before
 *                   EV_END == 1) -- REQUIRED for correctness when
 *                   endpoints coincide, as explained in the header.
 */
static int event_less(Event a, Event b) {
    if (a.time != b.time) return a.time < b.time;
    return a.type < b.type;
}

/*
 * WHY MERGE SORT INSTEAD OF qsort:
 *   The C standard does not specify WHICH algorithm qsort() must use,
 *   or bound its worst-case complexity -- only that it sorts correctly.
 *   Most C libraries implement it as an introsort/quicksort variant,
 *   which is O(n log n) on average but can degrade to O(n^2) on
 *   adversarial or pathological input orderings. Since the question
 *   explicitly asks for O(n log n) WORST-CASE, relying on qsort leaves
 *   that guarantee resting on library-implementation behavior rather
 *   than the algorithm itself.
 *
 *   Merge sort's O(n log n) bound holds for EVERY input, with no
 *   adversarial case -- it depends only on n, not on the data. The
 *   trade-off is a temporary O(n) auxiliary buffer during the sort
 *   (merge sort is not in-place), which does not change the overall
 *   space complexity class: the program already needs O(n) for the
 *   event list itself, and this adds another same-sized O(n) buffer,
 *   not a new asymptotic order of space.
 */
static void merge_events(Event *arr, Event *temp, int left, int mid, int right) {
    int i = left, j = mid + 1, k = left;
    while (i <= mid && j <= right) {
        if (!event_less(arr[j], arr[i]))
            temp[k++] = arr[i++];   /* arr[i] <= arr[j]: take left first (stable) */
        else
            temp[k++] = arr[j++];
    }
    while (i <= mid)   temp[k++] = arr[i++];
    while (j <= right) temp[k++] = arr[j++];
    for (i = left; i <= right; i++) arr[i] = temp[i];
}

static void merge_sort_recursive(Event *arr, Event *temp, int left, int right) {
    if (left >= right) return;              /* 0 or 1 element: already sorted */
    int mid = left + (right - left) / 2;     /* avoids overflow vs (l+r)/2 */
    merge_sort_recursive(arr, temp, left, mid);
    merge_sort_recursive(arr, temp, mid + 1, right);
    merge_events(arr, temp, left, mid, right);
}

/*
 * merge_sort_events: sorts `arr[0..n-1]` by event_less(), guaranteed
 * O(n log n) WORST CASE (recursion depth O(log n), O(n) work per
 * level -> O(n log n) total, independent of input arrangement).
 * Space: O(n) auxiliary (the `temp` buffer), freed before returning.
 */
static void merge_sort_events(Event *arr, int n) {
    if (n <= 1) return;
    Event *temp = (Event *)malloc((size_t)n * sizeof(Event));
    if (!temp) { fprintf(stderr, "Memory allocation failed\n"); exit(1); }
    merge_sort_recursive(arr, temp, 0, n - 1);
    free(temp);
}

/*

 * find_max_coverage_point:
 *   Given `l[0..n-1]` and `r[0..n-1]` (left/right endpoints, closed
 *   intervals, l[i] <= r[i]), finds the maximum number of intervals
 *   that simultaneously contain some point, and one point that
 *   achieves it.
 *
 *   Writes the maximum count into *out_max_count and the point into
 *   *out_point. For n == 0, writes 0 / 0 (no intervals to cover).
 *
 *   Time  : O(n log n) WORST CASE, guaranteed (merge sort -- see above).
 *   Space : O(n) for the event list + O(n) temporary merge-sort buffer
 *           (freed once sorting completes) -- still O(n) overall, not
 *           a higher complexity class.
 */
void find_max_coverage_point(const long long *l, const long long *r, int n,
                              int *out_max_count, long long *out_point) {
    if (n == 0) { *out_max_count = 0; *out_point = 0; return; }

    int total_events = 2 * n;
    Event *events = (Event *)malloc((size_t)total_events * sizeof(Event));
    if (!events) { fprintf(stderr, "Memory allocation failed\n"); exit(1); }

    for (int i = 0; i < n; i++) {
        events[2 * i]     = (Event){ l[i], EV_START };
        events[2 * i + 1] = (Event){ r[i], EV_END   };
    }

    merge_sort_events(events, total_events);      /* O(n log n), worst case */

    int count = 0, best_count = 0;
    long long best_point = 0;

    for (int i = 0; i < total_events; i++) {                       /* O(n) */
        if (events[i].type == EV_START) {
            count++;
            if (count > best_count) {
                best_count = count;
                best_point = events[i].time;
            }
        } else {
            count--;
        }
    }

    free(events);
    *out_max_count = best_count;
    *out_point     = best_point;
}

/* ---- I/O utilities ---- */
static void read_intervals(int n, long long *l, long long *r) {
    printf("Enter %d intervals as \"l r\" pairs (l <= r):\n", n);
    for (int i = 0; i < n; i++) {
        printf("Interval %d: ", i + 1);
        if (scanf("%lld %lld", &l[i], &r[i]) != 2) {
            fprintf(stderr, "Invalid input while reading interval %d\n", i + 1);
            exit(1);
        }
    }
}

int main(void) {
    int n;
    printf("Enter n (number of intervals): ");
    if (scanf("%d", &n) != 1 || n < 0) {
        fprintf(stderr, "Invalid n\n");
        return 1;
    }

    if (n == 0) {
        printf("\nNo intervals given -- no point is covered by any interval.\n");
        return 0;
    }

    long long *l = (long long *)malloc((size_t)n * sizeof(long long));
    long long *r = (long long *)malloc((size_t)n * sizeof(long long));
    if (!l || !r) { fprintf(stderr, "Memory allocation failed\n"); free(l); free(r); exit(1); }

    read_intervals(n, l, r);

    /* Validate l_i <= r_i (equality allowed: a single-point closed
     * interval [p,p] is well-formed and handled correctly). */
    for (int i = 0; i < n; i++) {
        if (l[i] > r[i]) {
            fprintf(stderr, "Invalid data: left endpoint must not exceed right endpoint for interval %d\n", i + 1);
            free(l); free(r);
            return 1;
        }
    }

    printf("\nIntervals: { ");
    for (int i = 0; i < n; i++)
        printf("(%lld,%lld)%s", l[i], r[i], (i == n - 1) ? "" : ", ");
    printf(" }\n\n");

    int max_count;
    long long best_point;
    find_max_coverage_point(l, r, n, &max_count, &best_point);

    printf("RESULT: Maximum coverage = %d interval(s), achieved at point p = %lld\n",
           max_count, best_point);

    free(l);
    free(r);
    return 0;
}