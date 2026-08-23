/*
 * ============================================================================
 *  Application of Sorting - IV
 * ----------------------------------------------------------------------------
 *  PROBLEM:
 *    A camera tracks entry time a_i and exit time b_i (b_i > a_i) for each
 *    of n persons p_i attending a party. Determine the time at which the
 *    most people were simultaneously present.
 *    All 2n entry/exit times are DISTINCT (no ties).
 *    Required time complexity: O(n log n).
 *
 *  KEY IDEA (sweep-line / event-based technique):
 *    The count of people present only changes at an arrival or a
 *    departure -- nowhere else. So instead of examining a continuum of
 *    time, we only need to examine the 2n discrete event moments:
 *        - an ARRIVAL event  (a_i, +1)  -- one more person present
 *        - a  DEPARTURE event (b_i, -1) -- one fewer person present
 *
 *    Sorting these 2n events by time, then sweeping through them while
 *    maintaining a running count, finds the maximum overlap in one
 *    linear pass after the sort.
 *
 *  ALGORITHM:
 *    1. Build 2n events: (a_i, +1) and (b_i, -1) for each person.
 *    2. Sort all 2n events by time ascending.        -> O(n log n)
 *       (times are guaranteed distinct, so a plain numeric compare on
 *       time suffices -- no tie-breaking rule is needed.)
 *    3. Sweep through sorted events, maintaining `count`:
 *          arrival   -> count++; if count > best, record (best, time)
 *          departure -> count--; (a departure can never create a new
 *                                  maximum, so no check is needed here)
 *                                                     -> O(n)
 *    Total: O(n log n) + O(n) = O(n log n).
 *
 *  PROPER INPUT REPRESENTATION:
 *    Each person is represented as TWO independent (time, delta) events
 *    rather than as an interval [a_i, b_i]. This is exactly the
 *    representation that turns "find max overlap of n intervals" into
 *    "find the max running sum of a sequence of +1/-1 signed points,"
 *    which is what makes the sort + single sweep (and hence the
 *    O(n log n) bound) possible at all.
 *
 *  COMPLEXITY:
 *    Time  : O(n log n)  -- sorting 2n events dominates; the sweep is O(n).
 *    Space : O(n)         -- storing the 2n events is unavoidable (it is
 *                             the input itself, restructured); qsort
 *                             sorts them in place, so there is no
 *                             additional O(n) auxiliary array beyond the
 *                             event list itself.
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    long long time;   /* the moment the event occurs                    */
    int       delta;  /* +1 for an arrival, -1 for a departure          */
} Event;

/* ---- qsort comparator: ascending by time. Times are guaranteed
 * distinct (per problem statement), so no tie-breaking is required. --*/
static int cmp_event(const void *a, const void *b) {
    long long ta = ((const Event *)a)->time;
    long long tb = ((const Event *)b)->time;
    return (ta > tb) - (ta < tb);
}

/*
 * find_max_overlap:
 *   Given arrival times `entry[0..n-1]` and departure times `exit[0..n-1]`
 *   (entry[i] < exit[i] for all i, all 2n values distinct), finds the
 *   time at which the maximum number of people were simultaneously
 *   present.
 *
 *   Writes the maximum count into *out_max_count and the time it first
 *   occurred into *out_time.
 *
 *   Time  : O(n log n)
 *   Space : O(n) for the event list (unavoidable -- it IS the recast
 *           input), O(1) additional beyond that (qsort is in place).
 */
void find_max_overlap(const long long *entry, const long long *exit_, int n,
                       int *out_max_count, long long *out_time) {
    int total_events = 2 * n;
    Event *events = (Event *)malloc((size_t)total_events * sizeof(Event));
    if (!events) { fprintf(stderr, "Memory allocation failed\n"); exit(1); }

    for (int i = 0; i < n; i++) {
        events[2 * i]     = (Event){ entry[i], +1 };
        events[2 * i + 1] = (Event){ exit_[i], -1 };
    }

    qsort(events, (size_t)total_events, sizeof(Event), cmp_event);  /* O(n log n) */

    int count = 0, best_count = 0;
    long long best_time = 0;

    for (int i = 0; i < total_events; i++) {                        /* O(n) */
        count += events[i].delta;
        if (events[i].delta > 0 && count > best_count) {
            best_count = count;
            best_time  = events[i].time;
        }
    }

    free(events);
    *out_max_count = best_count;
    *out_time      = best_time;
}

/* ---- I/O utilities ---- */
static long long *read_times(int n, const char *label) {
    long long *arr = (long long *)malloc((size_t)n * sizeof(long long));
    if (!arr) { fprintf(stderr, "Memory allocation failed\n"); exit(1); }
    printf("Enter %d %s: ", n, label);
    for (int i = 0; i < n; i++) {
        if (scanf("%lld", &arr[i]) != 1) {
            fprintf(stderr, "Invalid input while reading %s\n", label);
            free(arr);
            exit(1);
        }
    }
    return arr;
}

int main(void) {
    int n;
    printf("Enter n (number of persons): ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        fprintf(stderr, "Invalid n\n");
        return 1;
    }

    long long *entry = read_times(n, "entry times a_i (in order p_1..p_n)");
    long long *exit_ = read_times(n, "exit times b_i (in order p_1..p_n)");

    /* Validate b_i > a_i as the problem assumes */
    for (int i = 0; i < n; i++) {
        if (exit_[i] <= entry[i]) {
            fprintf(stderr, "Invalid data: exit time must be greater than entry time for person %d\n", i + 1);
            free(entry); free(exit_);
            return 1;
        }
    }

    printf("\nEntry times : { ");
    for (int i = 0; i < n; i++) printf("%lld%s", entry[i], (i == n - 1) ? "" : ", ");
    printf(" }\nExit times  : { ");
    for (int i = 0; i < n; i++) printf("%lld%s", exit_[i], (i == n - 1) ? "" : ", ");
    printf(" }\n\n");

    int max_count;
    long long best_time;
    find_max_overlap(entry, exit_, n, &max_count, &best_time);

    printf("RESULT: Maximum simultaneous attendance = %d person(s), first reached at time %lld\n",
           max_count, best_time);

    free(entry);
    free(exit_);
    return 0;
}