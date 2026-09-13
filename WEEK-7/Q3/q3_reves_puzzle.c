#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/**
 * ============================================================================
 * REVE'S PUZZLE (4-PEG TOWER OF HANOI) - FRAME-STEWART ALGORITHM
 * ============================================================================
 * Problem: Move n disks from peg 1 to peg 4, using 4 pegs, obeying the usual
 *          Hanoi rule (never place a larger disk on a smaller one). Solve for
 *          n = 8 in the minimum number of moves (33), and generalize to n.
 *
 * Idea (Frame-Stewart):
 *   To move n disks from `from` to `to` using ALL 4 pegs, pick a split k and:
 *     1. Move the top k disks from `from` to a spare peg s1, using all 4 pegs
 *        (recursively -- costs M(k) moves).
 *     2. Move the remaining (n-k) disks from `from` to `to` using the CLASSIC
 *        3-peg algorithm (peg s1 is occupied holding the k disks, so only 3
 *        pegs are usable) -- costs 2^(n-k) - 1 moves.
 *     3. Move the k disks from s1 onto `to`, again using all 4 pegs
 *        (recursively -- costs M(k) moves).
 *
 * Recurrence:
 *     M(0) = 0, M(1) = 1
 *     M(n) = min_{1<=k<n} { 2*M(k) + 2^(n-k) - 1 }
 *
 * For n = 8, this recurrence gives M(8) = 33 (achieved at k = 4 or k = 5),
 * matching the puzzle's required answer and the known OEIS A007664 sequence:
 * M(1..8) = 1, 3, 5, 9, 13, 17, 25, 33.
 *
 * Complexity:
 *   - The optimal split always has (n-k) = O(sqrt(2n)), which stays under 63
 *     for every n <= 1500, so we only ever need to scan the last 63 values of
 *     k. That turns the DP into O(n) time / O(n) space (instead of a naive
 *     O(n^2)).
 *   - Generating the ACTUAL list of M(n) moves costs O(M(n)) time/space, and
 *     M(n) = Theta(2^sqrt(2n)) -- sub-exponential in n but still exponential.
 *     That is why the program only prints/validates the concrete move
 *     sequence when M(n) is small enough to be practical, and otherwise
 *     reports just the move COUNT (which the O(n) DP can compute for n up to
 *     1500 without ever touching 2^64 overflow).
 * ============================================================================
 */

#define DP_MAX 1500                 /* safe limit before uint64_t overflow    */
#define GEN_MOVE_LIMIT 2000000ULL   /* max moves we will actually generate/print */

/* ---------------------------------------------------------------------- */
/* DP table: minimum move count and the optimal split k for each n        */
/* ---------------------------------------------------------------------- */
static uint64_t dp[DP_MAX + 1];
static int      bestK[DP_MAX + 1];
static int      dp_built_upto = -1;

static uint64_t pow2_safe(int exp) {
    if (exp >= 64) return UINT64_MAX;   /* defensive guard; never hit in practice */
    return (uint64_t)1 << exp;
}

/* Bottom-up DP build for n = 0..maxN. Bottom-up (not recursive) so there is
 * no recursion-depth concern even at n = 1500, and the k-range bound keeps
 * this O(n) overall. */
void build_dp_table(int maxN) {
    if (maxN > DP_MAX) maxN = DP_MAX;
    if (maxN <= dp_built_upto) return;

    dp[0] = 0; bestK[0] = 0;
    if (maxN >= 1) { dp[1] = 1; bestK[1] = 0; }

    int start = (dp_built_upto < 1) ? 2 : dp_built_upto + 1;
    for (int n = start; n <= maxN; n++) {
        uint64_t best = UINT64_MAX;
        int bk = 1;
        int k_start = (n > 63) ? n - 63 : 1;
        for (int k = k_start; k < n; k++) {
            if (dp[k] > (UINT64_MAX - 1) / 2) continue;   /* avoid overflow of 2*dp[k] */
            uint64_t candidate = 2 * dp[k] + pow2_safe(n - k) - 1;
            if (candidate < best) { best = candidate; bk = k; }
        }
        dp[n] = best;
        bestK[n] = bk;
    }
    dp_built_upto = maxN;
}

uint64_t reves_moves(int n) {
    if (n < 0) return 0;
    if (n > dp_built_upto) build_dp_table(n);
    return dp[n];
}

/* ---------------------------------------------------------------------- */
/* Peg simulation: used to actually GENERATE and VALIDATE the move list   */
/* ---------------------------------------------------------------------- */
typedef struct {
    int *disks;   /* disks[0] = bottom of stack */
    int top;      /* index of top disk, -1 if empty */
} Peg;

static Peg      pegs[4];
static uint64_t move_counter;
static int      print_moves_flag;
static int      validation_failed;

static void init_pegs(int n) {
    for (int p = 0; p < 4; p++) {
        pegs[p].disks = (int *)malloc(sizeof(int) * (size_t)(n + 1));
        pegs[p].top = -1;
    }
    for (int d = n; d >= 1; d--) {          /* largest first -> ends up at bottom */
        pegs[0].disks[++pegs[0].top] = d;
    }
}

static void free_pegs(void) {
    for (int p = 0; p < 4; p++) free(pegs[p].disks);
}

static void do_move(int from, int to) {
    if (pegs[from].top < 0) {
        printf("  VALIDATION ERROR: tried to move from empty peg %d\n", from + 1);
        validation_failed = 1;
        return;
    }
    int disk = pegs[from].disks[pegs[from].top];
    if (pegs[to].top >= 0 && pegs[to].disks[pegs[to].top] < disk) {
        printf("  VALIDATION ERROR: cannot place disk %d on smaller disk %d (peg %d -> peg %d)\n",
               disk, pegs[to].disks[pegs[to].top], from + 1, to + 1);
        validation_failed = 1;
        return;
    }
    pegs[from].top--;
    pegs[to].disks[++pegs[to].top] = disk;
    move_counter++;
    if (print_moves_flag) {
        printf("  Move %-7llu Disk %-3d : Peg %d -> Peg %d\n",
               (unsigned long long)move_counter, disk, from + 1, to + 1);
    }
}

/* classic 3-peg recursive Hanoi -- used as the "no spare peg available" step */
static void hanoi3(int n, int from, int to, int spare) {
    if (n <= 0) return;
    hanoi3(n - 1, from, spare, to);
    do_move(from, to);
    hanoi3(n - 1, spare, to, from);
}

/* Frame-Stewart recursive move generator for 4 pegs (0-indexed peg ids) */
static void frame_stewart(int n, int from, int to, int s1, int s2) {
    if (n <= 0) return;
    if (n == 1) { do_move(from, to); return; }
    int k = bestK[n];
    frame_stewart(k, from, s1, to, s2);   /* stash k disks on s1, using all 4 pegs   */
    hanoi3(n - k, from, to, s2);          /* move n-k disks with only 3 usable pegs  */
    frame_stewart(k, s1, to, from, s2);   /* bring the k disks onto the destination  */
}

/* Runs the algorithm for n disks, optionally printing each move, and checks:
 *   - every individual move was legal (checked live inside do_move)
 *   - the total move count matches the DP-predicted minimum M(n)
 *   - all n disks end up on the destination peg, correctly ordered
 * Returns 1 if everything checks out, 0 otherwise. */
int solve_and_validate(int n, int print) {
    if (n <= 0) return 1;
    reves_moves(n);              /* make sure dp[]/bestK[] are populated up to n */

    init_pegs(n);
    move_counter = 0;
    print_moves_flag = print;
    validation_failed = 0;

    frame_stewart(n, 0, 3, 1, 2);   /* peg 1 -> peg 4, using pegs 2 & 3 as spares */

    int ok = !validation_failed;

    if (ok && move_counter != dp[n]) {
        printf("  VALIDATION ERROR: move count mismatch. Expected %llu, executed %llu\n",
               (unsigned long long)dp[n], (unsigned long long)move_counter);
        ok = 0;
    }
    if (ok) {
        if (pegs[3].top != n - 1) ok = 0;
        for (int p = 0; p < 3 && ok; p++) if (pegs[p].top != -1) ok = 0;
        for (int i = 0; i < n && ok; i++) if (pegs[3].disks[i] != n - i) ok = 0;
    }

    free_pegs();
    return ok;
}

/* ---------------------------------------------------------------------- */
/* Explanation function                                                  */
/* ---------------------------------------------------------------------- */
void explain_algorithm(int n) {
    if (n < 1) n = 8;
    build_dp_table(n);

    printf("\n==================== ALGORITHM EXPLANATION ====================\n");
    printf("Problem : move n disks across 4 pegs (Reve's Puzzle); a larger disk\n");
    printf("          may never sit on top of a smaller one.\n\n");
    printf("Frame-Stewart idea -- to move n disks with 4 pegs, pick a split k:\n");
    printf("  1) Move the top k disks out of the way to a spare peg, using ALL\n");
    printf("     4 pegs (recursively -- M(k) moves).\n");
    printf("  2) Move the remaining (n-k) disks to the destination with the\n");
    printf("     classic 3-peg method, since the peg holding the k disks is\n");
    printf("     unusable for now (2^(n-k) - 1 moves).\n");
    printf("  3) Move the k disks from the spare peg onto the destination,\n");
    printf("     again using all 4 pegs (M(k) moves).\n\n");
    printf("Recurrence : M(n) = min over 1<=k<n of [ 2*M(k) + 2^(n-k) - 1 ]\n");
    printf("             M(0) = 0, M(1) = 1\n");
    printf("(k is only searched in the last 63 values below n, because the\n");
    printf(" optimal split always has n-k = O(sqrt(2n)), far under 63 here.)\n\n");

    printf("For n = %d:\n", n);
    printf("  Optimal split k* = %d\n", bestK[n]);
    printf("  M(%d) = 2*M(%d) + 2^(%d-%d) - 1 = 2*%llu + %llu - 1 = %llu\n",
           n, bestK[n], n, bestK[n],
           (unsigned long long)dp[bestK[n]],
           (unsigned long long)pow2_safe(n - bestK[n]),
           (unsigned long long)dp[n]);

    printf("\nComplexity:\n");
    printf("  Building the DP table up to n : O(n) time, O(n) space (63 candidate\n");
    printf("                                   k values checked per n).\n");
    printf("  Generating/validating the real move list for n disks : O(M(n)) time\n");
    printf("                                   and space, where M(n) = Theta(2^sqrt(2n)).\n");
    printf("  That's why this program only materializes the full move list while\n");
    printf("  M(n) <= %llu, and reports just the count for larger n.\n",
           (unsigned long long)GEN_MOVE_LIMIT);
    printf("=================================================================\n\n");
}

/* ---------------------------------------------------------------------- */
/* Validation suite                                                      */
/* ---------------------------------------------------------------------- */
void run_validation_suite(void) {
    printf("\n--- Running Validation Suite ---\n");
    clock_t start = clock();
    int all_ok = 1;

    build_dp_table(DP_MAX);

    /* 1) Known Frame-Stewart values, OEIS A007664, n = 1..8 */
    static const uint64_t known[9] = {0, 1, 3, 5, 9, 13, 17, 25, 33};
    printf("Known-answer check (n = 1..8):\n");
    for (int n = 1; n <= 8; n++) {
        int ok = (dp[n] == known[n]);
        all_ok &= ok;
        printf("  M(%d): expected=%-3llu got=%-3llu  %s\n", n,
               (unsigned long long)known[n], (unsigned long long)dp[n],
               ok ? "PASS" : "FAIL");
    }

    /* 2) Actually generate and validate the real move sequence for a spread of n */
    printf("\nMove-sequence generation + live validation:\n");
    int gen_tests[] = {1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 15, 20};
    int ngen = (int)(sizeof(gen_tests) / sizeof(gen_tests[0]));
    for (int i = 0; i < ngen; i++) {
        int n = gen_tests[i];
        int ok = solve_and_validate(n, 0);
        all_ok &= ok;
        printf("  n=%-2d -> %llu moves, all legal, ends in correct order:  %s\n",
               n, (unsigned long long)dp[n], ok ? "PASS" : "FAIL");
    }

    /* 3) Large-n sanity: table must be non-decreasing, matches CRITICAL check for n=8 */
    int mono_ok = 1;
    for (int n = 1; n <= DP_MAX; n++) if (dp[n] < dp[n - 1]) mono_ok = 0;
    all_ok &= mono_ok;
    printf("\nMonotonicity check for n=1..%d: %s\n", DP_MAX, mono_ok ? "PASS" : "FAIL");

    if (dp[8] != 33) {
        printf("CRITICAL FAIL: 8 disks should require exactly 33 moves. Got %llu\n",
               (unsigned long long)dp[8]);
        all_ok = 0;
    }

    clock_t end = clock();
    printf("\nExecution time: %.4f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    printf("Verdict: %s\n", all_ok ? "SUCCESS - Algorithm Verified!" : "ERROR FOUND");
    printf("---------------------------------\n\n");
}

/* ---------------------------------------------------------------------- */
/* Interactive menu                                                      */
/* ---------------------------------------------------------------------- */
int main(void) {
    int choice;
    build_dp_table(64);   /* warm up a small table so option 1 is instant */

    while (1) {
        printf("==========================================\n");
        printf("         REVE'S PUZZLE SOLVER (4 pegs)    \n");
        printf("==========================================\n");
        printf("1. Solve the default puzzle (8 disks) - show moves + validate\n");
        printf("2. Solve for a specific n (you enter n)\n");
        printf("3. Explain the algorithm (formula/complexity, worked example)\n");
        printf("4. Run automated validation suite\n");
        printf("5. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) { printf("Invalid input. Exiting.\n"); break; }

        switch (choice) {
            case 1: {
                int n = 8;
                uint64_t m = reves_moves(n);
                printf("\nMinimum moves for %d disks: %llu\n\n", n, (unsigned long long)m);
                int ok = solve_and_validate(n, 1);
                printf("\n%s\n\n", ok ? "All moves legal -- puzzle solved correctly."
                                       : "Validation FAILED.");
                break;
            }
            case 2: {
                long n_in;
                printf("Enter number of disks (1-%d): ", DP_MAX);
                if (scanf("%ld", &n_in) != 1 || n_in <= 0 || n_in > DP_MAX) {
                    printf("Invalid input (must be between 1 and %d).\n\n", DP_MAX);
                    break;
                }
                int n = (int)n_in;
                uint64_t m = reves_moves(n);
                printf("\nMinimum moves for %d disks: %llu\n", n, (unsigned long long)m);

                if (m <= GEN_MOVE_LIMIT) {
                    char resp[16];
                    printf("Print & validate the full move sequence? (y/n): ");
                    if (scanf("%15s", resp) == 1 && (resp[0] == 'y' || resp[0] == 'Y')) {
                        int ok = solve_and_validate(n, 1);
                        printf("\n%s\n\n", ok ? "All moves legal -- puzzle solved correctly."
                                               : "Validation FAILED.");
                    } else {
                        printf("\n");
                    }
                } else {
                    printf("(Move count too large to print/generate; showing count only.)\n\n");
                }
                break;
            }
            case 3: {
                long n_in;
                printf("Explain for how many disks? (default 8): ");
                if (scanf("%ld", &n_in) != 1 || n_in <= 0) n_in = 8;
                explain_algorithm((int)n_in);
                break;
            }
            case 4:
                run_validation_suite();
                break;
            case 5:
                printf("Exiting...\n");
                return 0;
            default:
                printf("Invalid choice. Try again.\n\n");
        }
    }
    return 0;
}