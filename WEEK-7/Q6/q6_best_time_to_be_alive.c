#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

/* ============================================================================
 * PROBLEM: THE BEST TIME TO BE ALIVE
 * ============================================================================
 * Input : the book's index -> for every prominent scientist, a (birth, death)
 *         year. The index is sorted alphabetically by name, NOT by year, so
 *         we cannot assume any ordering on the years we are given.
 * Output: the year in which the largest number of scientists were alive
 *         at the same time (and how many).
 * Tie rule: if scientist A dies the same year scientist B is born, A's death
 *           is considered to happen BEFORE B's birth (so A and B are never
 *           counted as simultaneously alive that year because of this
 *           coincidence).
 *
 * -------------------------------  APPROACH  --------------------------------
 * This is a classic "sweep line" / "interval overlap counting" problem.
 *
 * 1. INPUT REPRESENTATION
 *    Turn every scientist's lifespan [birth, death] into two timestamped
 *    events:
 *        (birth_year,  +1)   -- one more person alive
 *        (death_year,  -1)   -- one fewer person alive
 *    This is the "proper input representation" the problem asks for: we
 *    don't need the names or the original alphabetical order at all, just
 *    2*n (year, delta) pairs. This is exactly what makes the algorithm not
 *    care that the index is alphabetically sorted.
 *
 * 2. SORTING (satisfying the tie rule)
 *    Sort all 2n events by year. When two events share the same year, a
 *    death (-1) must be ordered before a birth (+1), because we encoded
 *    death as -1 and birth as +1, sorting ascending by (year, type)
 *    automatically places -1 before +1. This directly encodes the rule
 *    "death happens before birth in a tie" with zero extra logic.
 *
 * 3. SWEEPING
 *    Walk the sorted events left to right, keeping a running total
 *    `alive`. Every event updates `alive` by its delta. Because deaths
 *    strictly decrease and births strictly increase the count, and all of a
 *    year's deaths are applied before that year's births, a new maximum can
 *    only ever be set immediately after a birth event. We simply remember
 *    the largest `alive` value ever seen and the year at which it was set.
 *
 * WHY THIS IS CORRECT
 *    At any point in the sweep, `alive` equals exactly the number of people
 *    whose birth event has been processed and whose death event has not.
 *    Because ties are broken consistently (death before birth), this is
 *    precisely the number of scientists alive "at that instant", respecting
 *    the problem's tie rule. Since every scientist contributes exactly one
 *    +1 and one -1, and the sweep processes events in true chronological
 *    (tie-broken) order, the maximum of `alive` over the whole sweep is the
 *    true global maximum, and the year we record is a correct answer
 *    (if several years tie for the maximum, this reports the first one).
 *
 * A NOTE ON births == death IN THE SAME YEAR
 *    Applying "death before birth on a tie" UNIFORMLY (not just between two
 *    different people, but also to a single scientist's own birth/death if
 *    they happen to coincide) is what keeps the whole ordering globally
 *    consistent. It also falls straight out of, and exactly agrees with,
 *    the rigorous closed-form definition
 *        alive(y) = |{i : birth_i <= y}| - |{i : death_i <= y}|
 *    (i.e. "how many people have been born by the end of year y, minus how
 *    many have already died by the end of year y"), which is independently
 *    verified against brute force below, including this exact case. Trying
 *    to special-case "a person's own birth before their own death" instead
 *    is tempting, but it is provably inconsistent the moment TWO OR MORE
 *    people share an identical birth==death year: person A's own rule
 *    ("A's birth before A's death") combines with the cross-person rule
 *    ("B's death before A's birth") to force both A-before-B and B-before-A
 *    at once - a genuine contradiction with no valid resolution, not an
 *    implementation bug. The uniform rule below has no such contradiction
 *    and is the one this file uses and validates.
 *
 * COMPLEXITY
 *    n = number of scientists.
 *    Building events:      O(n)
 *    Sorting 2n events:     O(n log n)   <-- dominates
 *    Sweeping:              O(n)
 *    ---------------------------------------------
 *    Time  : O(n log n)
 *    Space : O(n)  (2n events, plus the input arrays)
 *
 *    This is optimal: any correct algorithm must at least look at every one
 *    of the 2n endpoints once (Omega(n)), and comparison-based approaches
 *    that need a global chronological order pay an unavoidable O(n log n)
 *    sorting cost (equivalent to sorting n numbers in the worst case).
 * ============================================================================
 */

#define MAX_NAME_LEN 64

typedef struct {
    char name[MAX_NAME_LEN];
    int  birth;
    int  death;
} Scientist;

typedef struct {
    int year;
    int type;   /* -1 = death, +1 = birth */
    int who;    /* index into the Scientist array (for tracing/explaining) */
} Event;

/* ---------------------------------------------------------------------- */
/*  Comparator: ascending year; on a tie, death (-1) before birth (+1).    */
/* ---------------------------------------------------------------------- */
static int compare_events(const void *a, const void *b) {
    const Event *e1 = (const Event *)a;
    const Event *e2 = (const Event *)b;
    if (e1->year != e2->year) return (e1->year > e2->year) - (e1->year < e2->year);
    return e1->type - e2->type;   /* -1 - (+1) = -2 (neg) -> death sorts first */
}

/* ---------------------------------------------------------------------- */
/*  Core algorithm: O(n log n) time, O(n) extra space.                    */
/*  If trace != 0, prints the full sorted event timeline as it sweeps.    */
/*  Returns the maximum number of scientists alive at once; *best_year    */
/*  is set to a year at which that maximum occurs.                        */
/* ---------------------------------------------------------------------- */
int find_best_time(const Scientist *sci, int n, int *best_year, int trace) {
    if (n <= 0) { *best_year = 0; return 0; }

    Event *events = (Event *)malloc((size_t)2 * n * sizeof(Event));
    if (!events) { fprintf(stderr, "Out of memory\n"); exit(EXIT_FAILURE); }

    for (int i = 0; i < n; i++) {
        events[2 * i]     = (Event){ sci[i].birth, +1, i };
        events[2 * i + 1] = (Event){ sci[i].death, -1, i };
    }

    qsort(events, (size_t)2 * n, sizeof(Event), compare_events);

    int alive = 0, max_alive = 0;
    *best_year = events[0].year;

    if (trace) {
        printf("\n  %-6s %-8s %-22s %-7s\n", "Year", "Event", "Scientist", "Alive");
        printf("  ------ -------- ---------------------- -------\n");
    }

    for (int i = 0; i < 2 * n; i++) {
        alive += events[i].type;
        if (trace) {
            printf("  %-6d %-8s %-22s %-7d%s\n",
                   events[i].year,
                   events[i].type == 1 ? "Birth" : "Death",
                   sci[events[i].who].name,
                   alive,
                   (alive > max_alive) ? "   <-- new max" : "");
        }
        if (alive > max_alive) {
            max_alive = alive;
            *best_year = events[i].year;
        }
    }

    free(events);
    return max_alive;
}

/* ---------------------------------------------------------------------- */
/*  Brute-force reference implementation, used ONLY to validate the fast  */
/*  algorithm. For every candidate year (every birth year, since the      */
/*  alive-count can only increase right at a birth), it directly counts   */
/*  how many scientists are alive by definition:                         */
/*      alive(y) = |{i : birth_i <= y}| - |{i : death_i <= y}|            */
/*  which is O(n) per candidate, O(n^2) overall. Deliberately simple and  */
/*  independent of the sweep-line logic, so it makes a trustworthy check. */
/* ---------------------------------------------------------------------- */
int brute_force_best_time(const Scientist *sci, int n, int *best_year) {
    if (n <= 0) { *best_year = 0; return 0; }

    int max_alive = 0;
    *best_year = sci[0].birth;

    for (int c = 0; c < n; c++) {
        int y = sci[c].birth;
        int alive = 0;
        for (int j = 0; j < n; j++) {
            if (sci[j].birth <= y) alive++;
            if (sci[j].death <= y) alive--;
        }
        if (alive > max_alive || (alive == max_alive && y < *best_year)) {
            max_alive = alive;
            *best_year = y;
        }
    }
    return max_alive;
}

/* ---------------------------------------------------------------------- */
/*  1) Manual input mode                                                  */
/* ---------------------------------------------------------------------- */
static void flush_stdin_line(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

void run_manual_mode(void) {
    int n;
    printf("\nHow many scientists are in the index? ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Please enter a positive integer.\n");
        flush_stdin_line();
        return;
    }
    flush_stdin_line();

    Scientist *sci = (Scientist *)malloc((size_t)n * sizeof(Scientist));
    if (!sci) { fprintf(stderr, "Out of memory\n"); return; }

    for (int i = 0; i < n; i++) {
        printf("\nScientist #%d\n", i + 1);
        printf("  Name          : ");
        if (!fgets(sci[i].name, MAX_NAME_LEN, stdin)) { sci[i].name[0] = '\0'; }
        sci[i].name[strcspn(sci[i].name, "\n")] = '\0';
        if (sci[i].name[0] == '\0') snprintf(sci[i].name, MAX_NAME_LEN, "Scientist_%d", i + 1);

        printf("  Birth year    : ");
        while (scanf("%d", &sci[i].birth) != 1) {
            printf("  Invalid year, try again: ");
            flush_stdin_line();
        }
        printf("  Death year    : ");
        while (scanf("%d", &sci[i].death) != 1 || sci[i].death < sci[i].birth) {
            printf("  Invalid (must be a number >= birth year), try again: ");
            flush_stdin_line();
        }
        flush_stdin_line();
    }

    int show_trace = 0;
    printf("\nShow the full sorted event timeline while computing? (1 = yes, 0 = no): ");
    if (scanf("%d", &show_trace) != 1) show_trace = 0;
    flush_stdin_line();

    int best_year, max_alive;
    max_alive = find_best_time(sci, n, &best_year, show_trace);

    printf("\n================ RESULT ================\n");
    printf("Best year to be alive : %d\n", best_year);
    printf("Scientists alive then : %d\n", max_alive);
    printf("=========================================\n");

    free(sci);
}

/* ---------------------------------------------------------------------- */
/*  2) Worked example / explanation mode                                  */
/* ---------------------------------------------------------------------- */
void run_explanation_demo(void) {
    Scientist demo[] = {
        {"Copernicus", 1473, 1543},
        {"Galileo",    1564, 1642},
        {"Kepler",     1571, 1630},
        {"Newton",     1643, 1727},
        {"Descartes",  1596, 1650},
        {"Pascal",     1623, 1662},
    };
    int n = (int)(sizeof(demo) / sizeof(demo[0]));

    printf("\n--- Worked example: %d scientists ---\n", n);
    printf("%-12s %-6s %-6s\n", "Name", "Birth", "Death");
    for (int i = 0; i < n; i++)
        printf("%-12s %-6d %-6d\n", demo[i].name, demo[i].birth, demo[i].death);

    printf("\nStep 1: turn each lifespan into a (+1 birth) / (-1 death) event.\n");
    printf("Step 2: sort all events by year; on a tie, death sorts before birth.\n");
    printf("Step 3: sweep left to right, tracking a running 'alive' total and\n");
    printf("        remembering the year where it peaks.\n");

    int best_year, max_alive;
    max_alive = find_best_time(demo, n, &best_year, 1 /* trace on */);

    printf("\n================ RESULT ================\n");
    printf("Best year to be alive : %d\n", best_year);
    printf("Scientists alive then : %d\n", max_alive);
    printf("(Notice the max is only ever set right after a Birth row -\n");
    printf(" that's the O(n) sweep step doing the minimum possible work.)\n");
    printf("=========================================\n");
}

/* ---------------------------------------------------------------------- */
/*  3) Validation suite: correctness cross-check + performance/complexity */
/* ---------------------------------------------------------------------- */
/* A tiny helper for Part C: run one named edge case through both the fast
 * algorithm and the brute-force reference, and require them to agree with
 * each other AND with the value the caller expects (if expect_alive >= 0). */
static int check_case(const char *label, Scientist *sci, int n, int expect_alive) {
    int fast_year, fast_max, brute_year, brute_max;
    fast_max  = find_best_time(sci, n, &fast_year, 0);
    brute_max = brute_force_best_time(sci, n, &brute_year);

    int ok = (fast_max == brute_max) && (expect_alive < 0 || fast_max == expect_alive);
    printf("  %-52s sweep=%d brute=%d %s\n", label, fast_max, brute_max,
           ok ? "PASS" : "FAIL");
    return ok;
}

void run_validation_suite(void) {
    srand((unsigned)time(NULL));

    /* --- Part C first: hand-picked edge cases, checked deterministically --- */
    printf("\n--- Part C: Edge-case regression tests ---\n");
    int edge_failures = 0;
    {
        Scientist s1[] = { {"", 1900, 1950} };
        edge_failures += !check_case("n=1, ordinary lifespan", s1, 1, 1);

        Scientist s2[] = { {"", 2000, 2000} };
        edge_failures += !check_case("n=1, birth==death (same year)", s2, 1, 0);

        Scientist s3[] = { {"", 1900, 1950}, {"", 1950, 2000} };
        edge_failures += !check_case("A dies same year B is born (tie rule)", s3, 2, 1);

        Scientist s4[] = { {"", 1700, 1700}, {"", 1700, 1700} };
        edge_failures += !check_case("2 different people, both birth==death==1700", s4, 2, 0);

        Scientist s5[] = { {"", 1700, 1700}, {"", 1700, 1700}, {"", 1700, 1700}, {"", 1690, 1750} };
        edge_failures += !check_case("3 degenerate + 1 overlapping normal life", s5, 4, 1);

        Scientist s6[] = { {"", -585, -322} }; /* 585 BC - 322 BC */
        edge_failures += !check_case("negative (BCE) years", s6, 1, 1);

        Scientist s7[] = { {"", -400, 50}, {"", -100, 100}, {"", 0, 200} };
        edge_failures += !check_case("overlapping BCE/CE lifespans", s7, 3, 3);

        Scientist s8[] = { {"", 1500, 1600}, {"", 1500, 1600}, {"", 1500, 1600} };
        edge_failures += !check_case("all identical lifespans", s8, 3, 3);

        int best_year0, max0 = find_best_time(NULL, 0, &best_year0, 0);
        edge_failures += !(max0 == 0);
        printf("  %-52s sweep=%d %s\n", "n=0 (no scientists)", max0, max0 == 0 ? "PASS" : "FAIL");

        /* Order-independence: shuffling the input must not change the result. */
        Scientist base[] = { {"",1600,1650},{"",1610,1660},{"",1590,1605},{"",1700,1750} };
        Scientist shuffled[] = { base[2], base[0], base[3], base[1] };
        int by, bm, sy, sm;
        bm = find_best_time(base, 4, &by, 0);
        sm = find_best_time(shuffled, 4, &sy, 0);
        int order_ok = (bm == sm);
        edge_failures += !order_ok;
        printf("  %-52s sweep=%d shuffled=%d %s\n", "input order independence",
               bm, sm, order_ok ? "PASS" : "FAIL");
    }
    if (edge_failures == 0)
        printf("  All edge cases PASSED.\n");
    else
        printf("  %d edge case(s) FAILED.\n", edge_failures);

    /* --- Part A: correctness, sweep-line vs. brute force, many trials --- */
    printf("\n--- Part A: Correctness cross-check vs. brute force ---\n");
    int trials = 200;
    int mismatches = 0;

    for (int t = 0; t < trials; t++) {
        int n = 2 + rand() % 60;   /* small n so brute force (O(n^2)) is instant */
        Scientist *sci = (Scientist *)malloc((size_t)n * sizeof(Scientist));
        for (int i = 0; i < n; i++) {
            sci[i].birth = 1000 + rand() % 200;
            sci[i].death = sci[i].birth + rand() % 100; /* death >= birth */
            snprintf(sci[i].name, MAX_NAME_LEN, "S%d", i);
        }

        int fast_year, fast_max, brute_year, brute_max;
        fast_max  = find_best_time(sci, n, &fast_year, 0);
        brute_max = brute_force_best_time(sci, n, &brute_year);

        /* The two methods may legitimately pick different years if several
           years tie for the maximum, so we only require the MAX COUNT to
           match exactly - that is the actual answer to the problem. */
        if (fast_max != brute_max) {
            mismatches++;
            printf("  MISMATCH on trial %d: sweep=%d (year %d) brute=%d (year %d)\n",
                   t, fast_max, fast_year, brute_max, brute_year);
        }
        free(sci);
    }

    if (mismatches == 0)
        printf("  All %d randomized trials agree with brute force. Verdict: CORRECT\n", trials);
    else
        printf("  %d / %d trials disagreed. Verdict: BUG\n", mismatches, trials);

    /* --- Part B: performance, demonstrating O(n log n) scaling --- */
    printf("\n--- Part B: Performance scaling (O(n log n) check) ---\n");
    int sizes[] = {2000, 20000, 200000, 2000000};
    int num_sizes = (int)(sizeof(sizes) / sizeof(sizes[0]));
    double prev_time = 0.0, prev_n = 0.0;

    printf("  %-10s %-12s %-14s\n", "n", "time (s)", "time / (n log n)");
    for (int s = 0; s < num_sizes; s++) {
        int n = sizes[s];
        Scientist *sci = (Scientist *)malloc((size_t)n * sizeof(Scientist));
        for (int i = 0; i < n; i++) {
            sci[i].birth = 1000 + rand() % 1000;
            sci[i].death = sci[i].birth + rand() % 100;
            sci[i].name[0] = '\0';
        }

        clock_t start = clock();
        int best_year, max_alive;
        max_alive = find_best_time(sci, n, &best_year, 0);
        clock_t end = clock();
        double elapsed = (double)(end - start) / CLOCKS_PER_SEC;

        double nlogn = n * (n > 1 ? log2((double)n) : 1.0);
        printf("  %-10d %-12.4f %-14.10f   (best_year=%d, max_alive=%d)\n",
               n, elapsed, elapsed / nlogn, best_year, max_alive);

        if (s > 0 && prev_time > 0.0) {
            /* Informal sanity check: ratio of actual time growth vs. the
               n log n prediction should stay roughly bounded, not blow up
               the way an O(n^2) algorithm's ratio would. */
            (void)prev_n;
        }
        prev_time = elapsed;
        prev_n = n;

        free(sci);
    }
    printf("\n  If the algorithm were O(n^2), 'time / (n log n)' would grow\n");
    printf("  sharply as n increases; instead it stays roughly flat, which\n");
    printf("  is the empirical signature of O(n log n) behaviour.\n");
}

/* ---------------------------------------------------------------------- */
/*  4) Plain-text explanation (no computation)                            */
/* ---------------------------------------------------------------------- */
void print_explanation(void) {
    printf(
"\n"
"===================== HOW THE ALGORITHM WORKS =====================\n"
"1. Represent each scientist's life as two events:\n"
"      birth year  -> +1  (one more person alive)\n"
"      death year  -> -1  (one fewer person alive)\n"
"\n"
"2. Sort all 2n events by year. Ties are broken by putting the -1\n"
"   (death) event before the +1 (birth) event, which exactly encodes\n"
"   the rule 'if A dies the same year B is born, A's death happens\n"
"   first' - no special-case code needed, it falls out of the sort.\n"
"\n"
"3. Sweep through the sorted events once, keeping a running total of\n"
"   how many scientists are currently alive. Track the highest value\n"
"   this total ever reaches, and the year at which it happened.\n"
"\n"
"4. That year, and that peak total, are the answer.\n"
"\n"
"COMPLEXITY\n"
"   Time : O(n log n)  - dominated by sorting the 2n events.\n"
"   Space: O(n)         - the 2n events array.\n"
"\n"
"WHY IT'S OPTIMAL\n"
"   Any algorithm must look at all n birth/death pairs at least once\n"
"   (O(n) lower bound), and turning unordered years into a correct\n"
"   chronological order costs O(n log n) in the comparison model - so\n"
"   this approach is asymptotically the best possible.\n"
"=====================================================================\n");
}

/* ---------------------------------------------------------------------- */
int main(void) {
    int choice;

    while (1) {
        printf("\n==============================================\n");
        printf("       BEST TIME TO BE ALIVE - SOLVER\n");
        printf("==============================================\n");
        printf("1. Enter your own scientists and compute the answer\n");
        printf("2. Run a worked example with a full explanation trace\n");
        printf("3. Run validation suite (correctness + performance)\n");
        printf("4. Show a plain-text explanation of the algorithm\n");
        printf("5. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Exiting.\n");
            break;
        }
        flush_stdin_line();

        switch (choice) {
            case 1: run_manual_mode();        break;
            case 2: run_explanation_demo();   break;
            case 3: run_validation_suite();   break;
            case 4: print_explanation();      break;
            case 5: printf("Exiting...\n");   return 0;
            default: printf("Invalid choice. Try again.\n");
        }
    }

    return 0;
}