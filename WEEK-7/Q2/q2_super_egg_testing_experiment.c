#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

/**
 * ============================================================================
 * SUPER EGG DROP PROBLEM  --  OPTIMIZED SOLUTION + EXPLANATION + VALIDATION
 * ============================================================================
 *
 * PROBLEM
 * -------
 * Given E identical eggs and a building with F floors, find the minimum
 * number of droppings that GUARANTEES (worst case) finding the highest
 * floor from which an egg can be dropped without breaking.
 *
 * NAIVE DP (for reference only)
 * ------------------------------
 *   dp(e, f) = 1 + min_{1<=x<=f} ( max( dp(e-1, x-1), dp(e, f-x) ) )
 *   dp(e, 0) = 0 , dp(e, 1) = 1 , dp(1, f) = f
 *
 *   Time  : O(E * F^2)              -> too slow for large F (e.g. F=1e5)
 *   Space : O(E * F)
 *
 * OPTIMIZED DP  (the "max floors coverable" trick)
 * -------------------------------------------------
 * Instead of asking "minimum drops for F floors", flip the question:
 *   "With e eggs and d drops, what is the maximum number of floors
 *    I can fully resolve?"
 *
 *   maxFloors(e, d) = maxFloors(e-1, d-1) + maxFloors(e, d-1) + 1
 *
 * Reasoning for the recurrence: drop 1 egg from some floor x.
 *   - If it BREAKS   -> we have (e-1) eggs and (d-1) drops left, and we
 *                       can resolve maxFloors(e-1, d-1) floors BELOW x.
 *   - If it SURVIVES -> we still have e eggs and (d-1) drops left, and we
 *                       can resolve maxFloors(e, d-1) floors ABOVE x.
 *   - The "+1" accounts for floor x itself, which this single drop settles.
 *
 * We simply increase d = 1, 2, 3, ... and stop the first time
 * maxFloors(E, d) >= F. That d is the answer.
 *
 * Because maxFloors(e, d) is literally Pascal's Triangle
 * ( maxFloors(e,d) = C(d,1)+C(d,2)+...+C(d,e) ), for a fixed number of
 * eggs E, maxFloors(E, d) grows like d^E, so d only needs to grow like
 * F^(1/E). In particular, whenever E >= log2(F+1), d = O(log F).
 *
 * COMPLEXITY OF THE OPTIMIZED ALGORITHM
 * --------------------------------------
 *   Let d* be the final answer (the number of drops).
 *   - Time  : O(E * d*)   ; d* = O(log F) once E is reasonably large,
 *             and in the worst case (E small, e.g. E=1 or 2) d* is at
 *             most O(F) or O(sqrt(F)) respectively -- still far better
 *             than the naive O(E*F^2).
 *   - Space : O(E)   (only one 1-D array of size E+1 is kept, updated
 *             in-place from right to left each round)
 *
 * This lets the program comfortably handle tens of thousands of test
 * cases even with F up to 100,000+ and E up to 50.
 * ============================================================================
 */

/* ---------------------------------------------------------------------
 * Flush the rest of the current input line (protects scanf from getting
 * stuck in an infinite loop when the user types non-numeric garbage).
 * ------------------------------------------------------------------- */
static void flush_stdin_line(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { /* discard */ }
}

/* ---------------------------------------------------------------------
 * OPTIMIZED SOLVER
 * dp[e] = maximum number of floors resolvable with e eggs at the
 *         current number of drops.
 * ------------------------------------------------------------------- */
uint64_t calculate_min_drops(uint64_t eggs, uint64_t floors) {
    if (floors == 0 || floors == 1) return floors;   /* 0 or 1 floor: 0 or 1 drop */
    if (eggs == 1) return floors;                    /* must test floors one by one */

    uint64_t *dp = (uint64_t *)calloc(eggs + 1, sizeof(uint64_t));
    if (!dp) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    uint64_t drops = 0;
    while (dp[eggs] < floors) {
        drops++;
        /* update from e = eggs down to 1 so dp[e-1] used on the RHS is
           still the value from the PREVIOUS round (drops-1) */
        for (uint64_t e = eggs; e >= 1; e--) {
            dp[e] = dp[e - 1] + dp[e] + 1;
        }
    }

    free(dp);
    return drops;
}

/* ---------------------------------------------------------------------
 * BRUTE-FORCE SOLVER (classic O(E*F) DP, monotonic two-pointer search)
 * Used ONLY to cross-validate the optimized solver on small inputs.
 * dp[e][f] = min drops for e eggs, f floors.
 * ------------------------------------------------------------------- */
static uint64_t brute_force_min_drops(uint64_t eggs, uint64_t floors) {
    if (floors == 0 || floors == 1) return floors;
    if (eggs == 1) return floors;

    uint64_t **dp = malloc((eggs + 1) * sizeof(uint64_t *));
    for (uint64_t e = 0; e <= eggs; e++)
        dp[e] = calloc(floors + 1, sizeof(uint64_t));

    for (uint64_t f = 0; f <= floors; f++) dp[1][f] = f;
    for (uint64_t e = 0; e <= eggs; e++) { dp[e][0] = 0; if (floors >= 1) dp[e][1] = 1; }

    for (uint64_t e = 2; e <= eggs; e++) {
        uint64_t x = 1; /* two-pointer: best split point is monotonic in f */
        for (uint64_t f = 2; f <= floors; f++) {
            /* advance x while breaking-branch value < surviving-branch value */
            while (x < f && dp[e - 1][x - 1] < dp[e][f - x]) x++;
            uint64_t best = 1 + dp[e - 1][x - 1];
            uint64_t alt  = 1 + dp[e][f - x];
            dp[e][f] = (best > alt) ? best : alt;
            if (x > 1) {
                uint64_t best2 = 1 + dp[e - 1][x - 2];
                uint64_t alt2  = 1 + dp[e][f - (x - 1)];
                uint64_t cand  = (best2 > alt2) ? best2 : alt2;
                if (cand < dp[e][f]) dp[e][f] = cand;
            }
        }
    }

    uint64_t result = dp[eggs][floors];
    for (uint64_t e = 0; e <= eggs; e++) free(dp[e]);
    free(dp);
    return result;
}

/* ---------------------------------------------------------------------
 * EXPLANATION FUNCTION
 * Walks through the optimized DP round by round for a user-chosen
 * (eggs, floors) pair, printing the "max floors coverable" table so
 * the whole reasoning is visible, then states the final answer.
 * ------------------------------------------------------------------- */
void explain_algorithm(uint64_t eggs, uint64_t floors) {
    printf("\n=================== STEP-BY-STEP EXPLANATION ===================\n");
    printf("Eggs (E) = %llu, Floors (F) = %llu\n\n",
           (unsigned long long)eggs, (unsigned long long)floors);

    if (floors == 0 || floors == 1) {
        printf("Trivial case: with %llu floor(s), the answer is simply %llu drop(s).\n",
               (unsigned long long)floors, (unsigned long long)floors);
        printf("==================================================================\n\n");
        return;
    }
    if (eggs == 1) {
        printf("Only 1 egg: we cannot risk breaking it early, so we must test\n");
        printf("floor 1, then 2, then 3, ... one at a time. Worst case needs\n");
        printf("exactly F = %llu drops.\n", (unsigned long long)floors);
        printf("==================================================================\n\n");
        return;
    }

    uint64_t *dp = (uint64_t *)calloc(eggs + 1, sizeof(uint64_t));
    uint64_t drops = 0;

    printf("We track dp[e] = \"max floors we can fully resolve with e eggs\n");
    printf("at the current number of drops\", and grow the drop count until\n");
    printf("dp[E] >= F.\n\n");
    printf("Recurrence: dp[e] (this round) = dp[e-1] (prev round)\n");
    printf("                                + dp[e]   (prev round) + 1\n\n");

    while (dp[eggs] < floors) {
        drops++;
        for (uint64_t e = eggs; e >= 1; e--) {
            dp[e] = dp[e - 1] + dp[e] + 1;
        }
        printf("After drop count d = %2llu :  ", (unsigned long long)drops);
        uint64_t show_max = eggs < 12 ? eggs : 12; /* keep the line readable */
        for (uint64_t e = 1; e <= show_max; e++) {
            printf("dp[%llu]=%llu  ", (unsigned long long)e, (unsigned long long)dp[e]);
        }
        if (eggs > show_max) printf("...");
        printf("\n");

        if (drops > floors + 2) { /* safety guard, should never trigger */
            printf("Safety stop.\n");
            break;
        }
    }

    printf("\ndp[E] = dp[%llu] = %llu, which is now >= F = %llu.\n",
           (unsigned long long)eggs, (unsigned long long)dp[eggs],
           (unsigned long long)floors);
    printf("=> Minimum guaranteed drops = %llu\n", (unsigned long long)drops);
    printf("==================================================================\n\n");

    free(dp);
}

/* ---------------------------------------------------------------------
 * VALIDATION SUITE
 *  Part A: cross-check optimized solver against the brute-force O(E*F)
 *          DP on many small random cases (proves CORRECTNESS).
 *  Part B: stress the optimized solver on large random cases and check
 *          bound sanity + timing (proves SCALABILITY/PERFORMANCE).
 * ------------------------------------------------------------------- */
void run_30000_testcases(void) {
    srand((unsigned int)time(NULL));

    /* ---- Part A: correctness cross-check vs brute force ---- */
    const int SMALL_TESTS = 2000;
    printf("\n--- Part A: Correctness check vs brute-force DP (%d cases) ---\n",
           SMALL_TESTS);
    int mismatches = 0;
    for (int i = 0; i < SMALL_TESTS; i++) {
        uint64_t e = (rand() % 8) + 1;      /* 1..8 eggs   */
        uint64_t f = (rand() % 500) + 1;    /* 1..500 floors */
        uint64_t fast = calculate_min_drops(e, f);
        uint64_t slow = brute_force_min_drops(e, f);
        if (fast != slow) {
            mismatches++;
            printf("MISMATCH  Eggs=%llu Floors=%llu  optimized=%llu brute=%llu\n",
                   (unsigned long long)e, (unsigned long long)f,
                   (unsigned long long)fast, (unsigned long long)slow);
        }
    }
    printf("Correctness check: %d / %d matched brute force.\n",
           SMALL_TESTS - mismatches, SMALL_TESTS);

    /* ---- Part B: large-scale stress test ---- */
    const int TEST_CASES = 30000;
    printf("\n--- Part B: Large-scale stress test (%d cases) ---\n", TEST_CASES);
    clock_t start = clock();
    uint64_t passed = 0;

    for (int i = 0; i < TEST_CASES; i++) {
        uint64_t e = (rand() % 50) + 1;        /* 1..50 eggs      */
        uint64_t f = (rand() % 100000) + 1;    /* 1..100000 floors */
        uint64_t drops = calculate_min_drops(e, f);

        /* sanity bounds: 1 <= drops <= f, and 1 egg must need exactly f */
        int ok = (drops >= 1 && drops <= f);
        if (e == 1) ok = ok && (drops == f);

        if (ok) {
            passed++;
        } else {
            printf("FAIL at Eggs=%llu, Floors=%llu: Drops=%llu\n",
                   (unsigned long long)e, (unsigned long long)f,
                   (unsigned long long)drops);
        }
    }

    clock_t end = clock();
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("Passed: %llu / %d\n", (unsigned long long)passed, TEST_CASES);
    printf("Execution Time: %.4f seconds\n", time_taken);
    printf("Verdict: %s\n",
           (mismatches == 0 && passed == TEST_CASES)
               ? "SUCCESS - Correct AND perfectly optimized!"
               : "ERROR FOUND");
    printf("------------------------------------------------\n\n");
}

/* ---------------------------------------------------------------------
 * MAIN MENU
 * ------------------------------------------------------------------- */
int main(void) {
    int choice;
    uint64_t eggs, floors;

    while (1) {
        printf("==========================================\n");
        printf("     SUPER EGG TESTING EXPERIMENT SOLVER   \n");
        printf("==========================================\n");
        printf("1. Solve for specific Eggs and Floors\n");
        printf("2. Explain algorithm step-by-step (custom input)\n");
        printf("3. Run full validation suite (correctness + stress test)\n");
        printf("4. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Exiting.\n");
            flush_stdin_line();
            break;
        }

        switch (choice) {
            case 1:
            case 2:
                printf("Enter number of Eggs (E): ");
                if (scanf("%llu", (unsigned long long *)&eggs) != 1 || eggs == 0) {
                    printf("Invalid eggs. Must be a positive integer.\n");
                    flush_stdin_line();
                    continue;
                }
                printf("Enter number of Floors (F): ");
                if (scanf("%llu", (unsigned long long *)&floors) != 1) {
                    printf("Invalid floors.\n");
                    flush_stdin_line();
                    continue;
                }

                if (choice == 1) {
                    uint64_t drops = calculate_min_drops(eggs, floors);
                    printf("\nResults for Eggs = %llu, Floors = %llu:\n",
                           (unsigned long long)eggs, (unsigned long long)floors);
                    printf("  Minimum Guaranteed Drops: %llu\n\n",
                           (unsigned long long)drops);
                } else {
                    explain_algorithm(eggs, floors);
                }
                break;
            case 3:
                run_30000_testcases();
                break;
            case 4:
                printf("Exiting...\n");
                return 0;
            default:
                printf("Invalid choice. Try again.\n");
        }
    }

    return 0;
}