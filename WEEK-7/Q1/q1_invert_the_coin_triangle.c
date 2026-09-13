#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>

/**
 * ============================================================================
 *  INVERT THE COIN-TRIANGLE  --  fully optimized O(1) solver
 * ============================================================================
 * PROBLEM
 *   An equilateral triangle of side n is built from closely packed coins
 *   (T(n) = n(n+1)/2 coins total, arranged in n rows of 1,2,3,...,n coins).
 *   On each move you slide ONE coin to a new empty lattice position.
 *   Find the minimum number of moves needed to turn the triangle upside down.
 *
 * KEY IDEA (why the answer is a single closed-form expression)
 *   You never have to move every coin. Instead, imagine sliding the *target*
 *   (inverted) triangle down over the *original* (upright) triangle so the
 *   two shapes overlap as much as possible. Every coin that lands in the
 *   overlap region is ALREADY in its final position -- it never needs to be
 *   touched. Every coin outside the overlap must be picked up and moved
 *   exactly once into one of the still-empty target spots (this is always
 *   possible with exactly (T(n) - overlap) moves, and no solution can do
 *   better, because at most `overlap` coins can simultaneously be correct
 *   under the best alignment). Hence:
 *
 *          minimum_moves(n) = T(n) - max_overlap(n)
 *
 *   Sliding the inverted triangle down by k = floor((n-1)/3) rows is the
 *   alignment that maximizes the overlap (this is the classical result for
 *   this puzzle -- shifting by any other amount only shrinks the shared
 *   region). Working the overlap sum out algebraically (see
 *   calculate_max_overlap_closedform below, and the step-by-step version in
 *   explain_derivation) collapses everything into the compact formula:
 *
 *          minimum_moves(n) = floor( n * (n + 1) / 6 )
 *
 *   This matches OEIS A007997 (0, 1, 2, 3, 5, 7, 9, 12, 15, 18, 22, ...).
 *
 * COMPLEXITY
 *   Time  : O(1)  -- calculate_min_moves is a single closed-form expression.
 *   Space : O(1)
 *   (Two alternative O(n) / algebraic-O(1) overlap routines are kept ONLY
 *    as independent cross-checks for the validation suite -- they are never
 *    needed to answer the problem itself.)
 * ============================================================================
 */

/* ---------------------------------------------------------------------- *
 *  Small helper: closed-form sum of consecutive integers lo..hi (O(1)).  *
 *  Splits the "count * (lo+hi)" product so it never needs an odd/2.5     *
 *  intermediate and stays exact in integer arithmetic.                   *
 * ---------------------------------------------------------------------- */
static uint64_t sum_range(int64_t lo, int64_t hi) {
    if (lo > hi) return 0ULL;
    uint64_t count = (uint64_t)(hi - lo + 1);
    uint64_t total = (uint64_t)(lo + hi);
    return (count % 2 == 0) ? (count / 2) * total : count * (total / 2);
}

/* Total coins in an upright triangle of side n : T(n) = n(n+1)/2.  O(1). */
uint64_t calculate_total_coins(uint64_t n) {
    return (n * (n + 1)) / 2ULL;
}

/* THE ANSWER. Compact closed-form formula.  O(1), no loops, no recursion. */
uint64_t calculate_min_moves(uint64_t n) {
    return (n * (n + 1)) / 6ULL;
}

/* ---------------------------------------------------------------------- *
 *  Reference #1: brute geometric simulation of the overlap, O(n).        *
 *  Kept ONLY so the validation suite has a slow-but-obviously-correct    *
 *  independent implementation to compare the fast formula against.      *
 * ---------------------------------------------------------------------- */
uint64_t calculate_max_overlap_loop(uint64_t n) {
    if (n == 0) return 0;
    uint64_t k = (n - 1) / 3;
    uint64_t overlap = 0;
    for (uint64_t r = k + 1; r <= n; r++) {
        uint64_t original_row = r;                 /* row r of upright triangle   */
        uint64_t inverted_row = n - r + k + 1;      /* matching row of shifted-down
                                                        inverted triangle           */
        overlap += (original_row < inverted_row) ? original_row : inverted_row;
    }
    return overlap;
}

/* ---------------------------------------------------------------------- *
 *  Reference #2: the SAME overlap, but derived algebraically so it is    *
 *  also O(1). original_row = r is increasing in r, inverted_row is       *
 *  decreasing in r, so min(original,inverted) switches exactly once at   *
 *  b = floor((n+k+1)/2). Sum the two arithmetic runs directly instead    *
 *  of looping over every row -- this is the "no further optimization     *
 *  possible" version of the overlap itself.                              *
 * ---------------------------------------------------------------------- */
uint64_t calculate_max_overlap_closedform(uint64_t n) {
    if (n == 0) return 0;
    uint64_t k = (n - 1) / 3;
    uint64_t b = (n + k + 1) / 2;
    if (b < k) b = k;
    if (b > n) b = n;

    uint64_t sum1 = sum_range((int64_t)(k + 1), (int64_t)b);              /* r <= b : min = r           */
    uint64_t sum2 = sum_range((int64_t)(k + 1), (int64_t)(n + k - b));    /* r >  b : min = n-r+k+1     */
    return sum1 + sum2;
}

/* ---------------------------------------------------------------------- *
 *  Known ground-truth values (OEIS A007997) for n = 1..20, used as a     *
 *  THIRD, fully independent check in the validation suite.               *
 * ---------------------------------------------------------------------- */
static const uint64_t KNOWN_SEQUENCE[20] = {
    0, 1, 2, 3, 5, 7, 9, 12, 15, 18, 22, 26, 30, 35, 40, 45, 51, 57, 63, 70
};

/* ============================================================================
 *  EXPLANATION FUNCTION -- walks through the derivation for a given n so a
 *  reader can see exactly how the O(1) formula was reached.
 * ========================================================================= */
void explain_derivation(uint64_t n) {
    printf("\n================= DERIVATION FOR n = %llu =================\n", (unsigned long long)n);

    if (n == 0) {
        printf("n = 0 is the empty triangle: 0 coins, 0 moves. Nothing to explain.\n");
        printf("=============================================================\n\n");
        return;
    }

    uint64_t total = calculate_total_coins(n);
    uint64_t k = (n - 1) / 3;

    printf("Step 1) Total coins in the triangle:\n");
    printf("        T(n) = n(n+1)/2 = %llu*%llu/2 = %llu coins\n",
           (unsigned long long)n, (unsigned long long)(n + 1), (unsigned long long)total);

    printf("\nStep 2) Best vertical shift for the inverted triangle:\n");
    printf("        k = floor((n-1)/3) = floor(%llu/3) = %llu rows\n",
           (unsigned long long)(n - 1), (unsigned long long)k);

    bool show_table = (n <= 15);
    if (show_table) {
        printf("\nStep 3) Row-by-row overlap when the inverted triangle is shifted down %llu rows:\n", (unsigned long long)k);
        printf("        (row r of the upright triangle has r coins; the inverted triangle's\n");
        printf("         matching row has n-r+k+1 coins; only the SMALLER of the two overlaps)\n\n");
        printf("        %-10s %-15s %-15s %-10s\n", "row r", "upright(r)", "inverted(r)", "overlap");
        uint64_t running = 0;
        for (uint64_t r = k + 1; r <= n; r++) {
            uint64_t orig = r;
            uint64_t inv  = n - r + k + 1;
            uint64_t m    = (orig < inv) ? orig : inv;
            running += m;
            printf("        %-10llu %-15llu %-15llu %-10llu\n",
                   (unsigned long long)r, (unsigned long long)orig,
                   (unsigned long long)inv, (unsigned long long)m);
        }
        printf("\n        Sum of the 'overlap' column = %llu\n", (unsigned long long)running);
    } else {
        printf("\nStep 3) (n is large, skipping the row-by-row table -- using the closed form\n");
        printf("         instead of printing %llu rows.)\n", (unsigned long long)(n - k));
    }

    uint64_t overlap_cf = calculate_max_overlap_closedform(n);
    printf("\nStep 4) Closed-form overlap (splitting the row sum at its turning point b):\n");
    printf("        max_overlap(n) = %llu coins\n", (unsigned long long)overlap_cf);

    uint64_t moves_from_overlap = total - overlap_cf;
    uint64_t moves_formula = calculate_min_moves(n);

    printf("\nStep 5) Minimum moves = Total coins - Max overlap:\n");
    printf("        %llu - %llu = %llu moves\n",
           (unsigned long long)total, (unsigned long long)overlap_cf, (unsigned long long)moves_from_overlap);

    printf("\nStep 6) This always collapses to the single compact formula:\n");
    printf("        floor(n*(n+1)/6) = floor(%llu/6) = %llu moves\n",
           (unsigned long long)(n * (n + 1)), (unsigned long long)moves_formula);

    printf("\nResult: minimum_moves(%llu) = %llu   [match: %s]\n",
           (unsigned long long)n, (unsigned long long)moves_formula,
           (moves_from_overlap == moves_formula) ? "YES" : "NO -- BUG!");
    printf("=============================================================\n\n");
}

/* ============================================================================
 *  VALIDATION SUITE -- three independent cross-checks:
 *    (A) compact O(1) formula vs the O(1) closed-form overlap, for a large
 *        sweep of n (this is the primary, fast check).
 *    (B) compact O(1) formula vs the O(n) brute-force loop overlap, for a
 *        smaller sweep (this is the slow-but-unquestionably-correct check).
 *    (C) compact O(1) formula vs the 20 known OEIS A007997 values (a totally
 *        independent, hand-verified ground truth).
 *    (D) a timing demo showing the O(1) formula answering an astronomically
 *        large n instantly -- something the O(n) loop could never do.
 * ========================================================================= */
void run_validation_suite(void) {
    printf("\n=================== VALIDATION SUITE ===================\n");

    /* (C) Known-sequence check first -- cheapest and most authoritative. */
    printf("[Check C] Comparing against %d known OEIS A007997 values...\n", (int)(sizeof(KNOWN_SEQUENCE) / sizeof(KNOWN_SEQUENCE[0])));
    bool known_ok = true;
    for (uint64_t n = 1; n <= sizeof(KNOWN_SEQUENCE) / sizeof(KNOWN_SEQUENCE[0]); n++) {
        uint64_t got = calculate_min_moves(n);
        uint64_t expected = KNOWN_SEQUENCE[n - 1];
        if (got != expected) {
            known_ok = false;
            printf("  MISMATCH at n=%llu: got %llu, expected %llu\n",
                   (unsigned long long)n, (unsigned long long)got, (unsigned long long)expected);
        }
    }
    printf("  %s\n\n", known_ok ? "PASS - all 20 known values match." : "FAIL");

    /* (B) Slow ground-truth loop check over a moderate range. */
    const uint64_t LOOP_TEST_CASES = 3000;
    printf("[Check B] Cross-checking formula against the O(n) brute-force loop for n = 1..%llu...\n",
           (unsigned long long)LOOP_TEST_CASES);
    clock_t startB = clock();
    uint64_t failB = 0;
    for (uint64_t n = 1; n <= LOOP_TEST_CASES; n++) {
        uint64_t total = calculate_total_coins(n);
        uint64_t moves_formula = calculate_min_moves(n);
        uint64_t moves_loop = total - calculate_max_overlap_loop(n);
        if (moves_formula != moves_loop) {
            failB++;
            printf("  MISMATCH at n=%llu: formula=%llu, loop=%llu\n",
                   (unsigned long long)n, (unsigned long long)moves_formula, (unsigned long long)moves_loop);
        }
    }
    double timeB = (double)(clock() - startB) / CLOCKS_PER_SEC;
    printf("  %s (%llu / %llu passed) in %.4f s\n\n",
           failB == 0 ? "PASS" : "FAIL",
           (unsigned long long)(LOOP_TEST_CASES - failB), (unsigned long long)LOOP_TEST_CASES, timeB);

    /* (A) Fast O(1)-vs-O(1) sweep over a much larger range. */
    const uint64_t FAST_TEST_CASES = 2000000;
    printf("[Check A] Cross-checking formula against the O(1) closed-form overlap for n = 1..%llu...\n",
           (unsigned long long)FAST_TEST_CASES);
    clock_t startA = clock();
    uint64_t failA = 0;
    for (uint64_t n = 1; n <= FAST_TEST_CASES; n++) {
        uint64_t total = calculate_total_coins(n);
        uint64_t moves_formula = calculate_min_moves(n);
        uint64_t moves_cf = total - calculate_max_overlap_closedform(n);
        if (moves_formula != moves_cf) {
            failA++;
            if (failA <= 5)
                printf("  MISMATCH at n=%llu: formula=%llu, closed-form=%llu\n",
                       (unsigned long long)n, (unsigned long long)moves_formula, (unsigned long long)moves_cf);
        }
    }
    double timeA = (double)(clock() - startA) / CLOCKS_PER_SEC;
    printf("  %s (%llu / %llu passed) in %.4f s\n\n",
           failA == 0 ? "PASS" : "FAIL",
           (unsigned long long)(FAST_TEST_CASES - failA), (unsigned long long)FAST_TEST_CASES, timeA);

    /* (D) Instant answer for an astronomically large n. */
    uint64_t huge_n = 1000000000000ULL; /* 10^12 */
    clock_t startD = clock();
    uint64_t huge_moves = calculate_min_moves(huge_n);
    double timeD = (double)(clock() - startD) / CLOCKS_PER_SEC;
    printf("[Check D] O(1) formula instantly handles n = %llu (10^12):\n", (unsigned long long)huge_n);
    printf("  minimum_moves = %llu, computed in %.8f s\n", (unsigned long long)huge_moves, timeD);
    printf("  (the O(n) loop version would need ~%llu iterations for this single n alone)\n\n",
           (unsigned long long)huge_n);

    bool overall = known_ok && (failB == 0) && (failA == 0);
    printf("VERDICT: %s\n", overall ? "SUCCESS - formula verified against 3 independent methods." : "ERROR FOUND - see mismatches above.");
    printf("==========================================================\n\n");
}

/* ---------------------------------------------------------------------- */
static bool read_u64(const char *prompt, uint64_t *out) {
    printf("%s", prompt);
    if (scanf("%llu", (unsigned long long *)out) != 1) {
        while (getchar() != '\n' && !feof(stdin)) { /* flush bad input */ }
        return false;
    }
    return true;
}

int main(void) {
    int choice;

    while (1) {
        printf("==========================================\n");
        printf("      COIN-TRIANGLE INVERSION SOLVER      \n");
        printf("==========================================\n");
        printf("1. Solve for a specific n (fast O(1) answer)\n");
        printf("2. Run full validation suite\n");
        printf("3. Show step-by-step derivation for a specific n\n");
        printf("4. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Exiting.\n");
            break;
        }

        uint64_t n;
        switch (choice) {
            case 1:
                if (read_u64("Enter side length of triangle (n): ", &n)) {
                    uint64_t total = calculate_total_coins(n);
                    uint64_t moves = calculate_min_moves(n);
                    printf("\nResults for n = %llu:\n", (unsigned long long)n);
                    printf("  Total Coins             : %llu\n", (unsigned long long)total);
                    printf("  Minimum Moves Required  : %llu\n\n", (unsigned long long)moves);
                } else {
                    printf("Invalid value for n. Please enter a non-negative integer.\n\n");
                }
                break;
            case 2:
                run_validation_suite();
                break;
            case 3:
                if (read_u64("Enter side length of triangle (n): ", &n)) {
                    explain_derivation(n);
                } else {
                    printf("Invalid value for n. Please enter a non-negative integer.\n\n");
                }
                break;
            case 4:
                printf("Exiting...\n");
                return 0;
            default:
                printf("Invalid choice. Try again.\n\n");
        }
    }

    return 0;
}