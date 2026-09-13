#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

/**
 * ============================================================================
 *  SECURITY SWITCHES  --  fully optimized O(1) solver
 * ============================================================================
 * PROBLEM
 *   n switches, all initially ON. In one move:
 *     (i)   the rightmost switch may be toggled freely, or
 *     (ii)  any other switch i may be toggled iff switch i+1 is ON AND every
 *           switch to the right of i+1 is OFF.
 *   Find the minimum number of moves to turn every switch OFF.
 *
 * WHY THIS IS THE "CHINESE RINGS" PUZZLE
 *   The legality rule for switch i depends only on the switches to its
 *   right, never on its own current value -- exactly the rule that governs
 *   the classic Chinese Rings / Baguenaudier / "Spin-Out" puzzle, and the
 *   underlying state graph is the same one used to build the reflected
 *   binary Gray code. At ANY state there are at most two legal moves: the
 *   rightmost switch, and (at most) one other specific switch -- so the
 *   state graph is essentially a single path, and the shortest path from
 *   "all ON" to "all OFF" is exactly the puzzle's known move count.
 *
 * DERIVING THE RECURRENCE (intuition)
 *   To free switch n-1 for toggling you must first get switch n ON with
 *   everything past it already OFF -- reachable only by first solving the
 *   (n-2)-switch sub-puzzle, toggling switch (n-1), then re-running that
 *   same (n-2)-switch sub-puzzle to clear the way for switch n itself,
 *   which then costs an M(n-1)-sized job. That gives:
 *
 *          M(1) = 1,  M(2) = 2
 *          M(n) = M(n-1) + 2*M(n-2) + 1                       for n >= 3
 *
 * SOLVING THE RECURRENCE IN CLOSED FORM
 *   Homogeneous part x^2 - x - 2 = 0  =>  (x-2)(x+1) = 0  =>  roots 2, -1.
 *   Particular solution for the "+1": constant C with C - C - 2C = 1 => C = -1/2.
 *   General solution: M(n) = A*2^n + B*(-1)^n - 1/2.
 *   Fit M(1)=1, M(2)=2  =>  A = 2/3, B = -1/6, giving
 *
 *          M(n) = 2^(n+1)/3 - (-1)^n/6 - 1/2
 *
 *   Because 2^(n+1) mod 3 cycles between 1 and 2, this remainder-subtraction
 *   is always exact, so the whole expression collapses to a single integer
 *   floor division:
 *
 *          M(n) = floor( 2^(n+1) / 3 )                    (OEIS A000975)
 *
 * COMPLEXITY
 *   Time  : O(1)  -- one bit shift + one division.
 *   Space : O(1)
 *   (The recursive and BFS routines below exist ONLY to independently prove
 *    the formula -- they are never needed to answer the problem itself.)
 *
 * HARDWARE LIMITS AND HOW FAR WE PUSH THEM
 *   uint64_t can only hold results up to n = 62 (2^63 already overflows a
 *   64-bit shift). Rather than stopping there, this program also offers an
 *   unsigned __int128 path (GCC/Clang extension) that is exact up to
 *   n = 126 -- the practical maximum obtainable from native machine
 *   integers without pulling in an arbitrary-precision library such as GMP.
 * ============================================================================
 */

/* ---------------------------------------------------------------------- *
 *  THE ANSWER. O(1), valid for n = 0..62 (fits in a 64-bit result).      *
 * ---------------------------------------------------------------------- */
uint64_t calculate_min_moves(uint64_t n) {
    if (n == 0) return 0;
    if (n > 62) return 0;              /* caller must use the _ext() path */
    return (1ULL << (n + 1)) / 3ULL;
}

/* ---------------------------------------------------------------------- *
 *  Extended-range O(1) version using 128-bit arithmetic. Valid n=0..126. *
 *  (n=127 would require shifting by 128, which is undefined behaviour on *
 *   a 128-bit type, so 126 is the true hardware ceiling here.)           *
 * ---------------------------------------------------------------------- */
unsigned __int128 calculate_min_moves_ext(uint64_t n) {
    if (n == 0) return 0;
    if (n > 126) return 0;             /* beyond native 128-bit range     */
    return ((unsigned __int128)1 << (n + 1)) / 3;
}

/* Print an unsigned __int128 in decimal (printf has no native verb for it). */
void print_u128(unsigned __int128 value) {
    if (value == 0) { printf("0"); return; }
    char buf[45];                      /* 2^128 has at most 39 digits     */
    int i = 0;
    while (value > 0) {
        buf[i++] = (char)('0' + (int)(value % 10));
        value /= 10;
    }
    while (i > 0) putchar(buf[--i]);
}

/* ---------------------------------------------------------------------- *
 *  Reference #1: the O(2^n) recurrence, evaluated directly (no memo) so  *
 *  it is a genuinely independent (slow) way to reach the same numbers.   *
 * ---------------------------------------------------------------------- */
uint64_t calculate_moves_recursive(uint64_t n) {
    if (n == 0) return 0;
    if (n == 1) return 1;
    if (n == 2) return 2;
    return calculate_moves_recursive(n - 1) + 2 * calculate_moves_recursive(n - 2) + 1;
}

/* ---------------------------------------------------------------------- *
 *  Reference #2: TRUE ground truth. Simulates the actual switch rules    *
 *  from the problem statement and runs a real BFS shortest-path search   *
 *  over the state graph. This is independent of the recurrence entirely  *
 *  -- it proves the formula answers the ACTUAL puzzle, not just that two *
 *  algebraic derivations agree with each other.                         *
 *
 *  State = n-bit mask, bit (i-1) = 1 means switch i is ON (bit 0 =       *
 *  switch 1 = leftmost, bit n-1 = switch n = rightmost).
 * ---------------------------------------------------------------------- */
static bool is_legal_move(uint32_t mask, int n, int i /* 1..n */) {
    if (i == n) return true;                    /* rightmost: always legal */
    if (((mask >> i) & 1u) == 0u) return false;  /* switch i+1 must be ON   */
    uint32_t higher = mask >> (i + 1);           /* switches i+2..n         */
    return higher == 0u;                         /* must all be OFF         */
}

/* BFS ground-truth solver. Only safe/fast for modest n (state space 2^n). */
long long bfs_min_moves(int n) {
    if (n <= 0) return 0;
    size_t num_states = (size_t)1 << n;
    int *dist = (int *)malloc(num_states * sizeof(int));
    uint32_t *queue = (uint32_t *)malloc(num_states * sizeof(uint32_t));
    if (!dist || !queue) {
        fprintf(stderr, "Out of memory for n=%d\n", n);
        free(dist); free(queue);
        return -1;
    }
    for (size_t i = 0; i < num_states; i++) dist[i] = -1;

    uint32_t start = (uint32_t)(num_states - 1); /* all switches ON  */
    uint32_t target = 0;                         /* all switches OFF */
    size_t qh = 0, qt = 0;
    dist[start] = 0;
    queue[qt++] = start;

    while (qh < qt) {
        uint32_t cur = queue[qh++];
        if (cur == target) break;
        for (int i = 1; i <= n; i++) {
            if (is_legal_move(cur, n, i)) {
                uint32_t nxt = cur ^ (1u << (i - 1));
                if (dist[nxt] == -1) {
                    dist[nxt] = dist[cur] + 1;
                    queue[qt++] = nxt;
                }
            }
        }
    }
    long long result = dist[target];
    free(dist);
    free(queue);
    return result;
}

/* ---------------------------------------------------------------------- *
 *  Known ground-truth values (OEIS A000975) for n = 1..10, a THIRD,      *
 *  literature-sourced independent check.                                 *
 * ---------------------------------------------------------------------- */
static const uint64_t KNOWN_SEQUENCE[10] = { 1, 2, 5, 10, 21, 42, 85, 170, 341, 682 };

/* ============================================================================
 *  EXPLANATION FUNCTION -- walks through the derivation for a given n.
 * ========================================================================= */
void explain_derivation(uint64_t n) {
    printf("\n================= DERIVATION FOR n = %llu =================\n", (unsigned long long)n);

    if (n == 0) {
        printf("n = 0 means no switches at all: 0 moves. Nothing to explain.\n");
        printf("=============================================================\n\n");
        return;
    }

    printf("Step 1) The legality rule only ever looks to the RIGHT of a switch,\n");
    printf("        never at its own value -- that's what makes this the Chinese\n");
    printf("        Rings / Gray-code puzzle: at most 2 moves are ever legal from\n");
    printf("        any state (toggle switch n, or toggle exactly one other switch).\n");

    printf("\nStep 2) Recurrence (derived from the structure of the puzzle):\n");
    printf("        M(1) = 1,  M(2) = 2\n");
    printf("        M(n) = M(n-1) + 2*M(n-2) + 1   for n >= 3\n");
    if (n <= 20) {
        printf("        Unrolled for n=%llu:\n", (unsigned long long)n);
        uint64_t prev2 = 0, prev1 = 0, cur = 0;
        for (uint64_t i = 1; i <= n; i++) {
            if (i == 1) cur = 1;
            else if (i == 2) cur = 2;
            else cur = prev1 + 2 * prev2 + 1;
            if (i >= 3)
                printf("          M(%llu) = M(%llu) + 2*M(%llu) + 1 = %llu + 2*%llu + 1 = %llu\n",
                       (unsigned long long)i, (unsigned long long)(i - 1), (unsigned long long)(i - 2),
                       (unsigned long long)prev1, (unsigned long long)prev2, (unsigned long long)cur);
            prev2 = prev1;
            prev1 = cur;
        }
    }

    printf("\nStep 3) Solving the recurrence (characteristic equation x^2 - x - 2 = 0\n");
    printf("        factors as (x-2)(x+1) = 0, roots x = 2, x = -1):\n");
    printf("        M(n) = A*2^n + B*(-1)^n - 1/2,  fit to M(1)=1, M(2)=2\n");
    printf("        gives A = 2/3, B = -1/6, i.e.\n");
    printf("        M(n) = 2^(n+1)/3 - (-1)^n/6 - 1/2\n");
    printf("        which always simplifies to the exact integer:\n");
    printf("        M(n) = floor( 2^(n+1) / 3 )\n");

    if (n <= 62) {
        uint64_t formula_val = calculate_min_moves(n);
        printf("\nStep 4) Plugging in n=%llu:\n", (unsigned long long)n);
        printf("        floor(2^%llu / 3) = %llu moves\n",
               (unsigned long long)(n + 1), (unsigned long long)formula_val);

        if (n <= 20) {
            long long bfs_val = bfs_min_moves((int)n);
            printf("\nStep 5) Cross-check against a REAL breadth-first search over the\n");
            printf("        actual switch-toggling rules (not the recurrence -- the rules\n");
            printf("        themselves): shortest path found = %lld moves   [match: %s]\n",
                   bfs_val, ((uint64_t)bfs_val == formula_val) ? "YES" : "NO -- BUG!");
        }
    } else if (n <= 126) {
        unsigned __int128 formula_val = calculate_min_moves_ext(n);
        printf("\nStep 4) n=%llu exceeds uint64_t's range (max n=62), using the 128-bit\n", (unsigned long long)n);
        printf("        extended path instead: floor(2^%llu / 3) = ", (unsigned long long)(n + 1));
        print_u128(formula_val);
        printf(" moves\n");
    } else {
        printf("\nStep 4) n=%llu exceeds even 128-bit native integer range (max n=126).\n", (unsigned long long)n);
        printf("        Beyond this point an arbitrary-precision library (e.g. GMP) or a\n");
        printf("        big-integer language (Python, etc.) is required -- see the\n");
        printf("        accompanying analysis notes.\n");
    }
    printf("=============================================================\n\n");
}

/* ============================================================================
 *  VALIDATION SUITE -- three independent cross-checks:
 *    (A) formula vs the O(2^n) recurrence, for a range small enough to run
 *        the recurrence in reasonable time.
 *    (B) formula vs a REAL rule-level BFS shortest-path search -- proves the
 *        formula solves the actual stated puzzle, not just the recurrence.
 *    (C) formula vs 10 literature values (OEIS A000975).
 *    (D) a demonstration of the 128-bit extended path reaching n=126, far
 *        beyond what either the recurrence or BFS could ever reach.
 * ========================================================================= */
void run_validation_suite(void) {
    printf("\n=================== VALIDATION SUITE ===================\n");

    /* (C) Known-sequence check first -- cheapest, literature-sourced. */
    printf("[Check C] Comparing against %d known OEIS A000975 values...\n",
           (int)(sizeof(KNOWN_SEQUENCE) / sizeof(KNOWN_SEQUENCE[0])));
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
    printf("  %s\n\n", known_ok ? "PASS - all 10 known values match." : "FAIL");

    /* (A) O(2^n) recurrence cross-check. */
    const uint64_t RECUR_MAX_N = 28; /* keeps runtime well under a second */
    printf("[Check A] Cross-checking formula against the O(2^n) recurrence for n = 1..%llu...\n",
           (unsigned long long)RECUR_MAX_N);
    clock_t startA = clock();
    uint64_t failA = 0;
    for (uint64_t n = 1; n <= RECUR_MAX_N; n++) {
        uint64_t f = calculate_min_moves(n);
        uint64_t r = calculate_moves_recursive(n);
        if (f != r) {
            failA++;
            printf("  MISMATCH at n=%llu: formula=%llu, recursive=%llu\n",
                   (unsigned long long)n, (unsigned long long)f, (unsigned long long)r);
        }
    }
    double timeA = (double)(clock() - startA) / CLOCKS_PER_SEC;
    printf("  %s (%llu / %llu passed) in %.4f s\n\n",
           failA == 0 ? "PASS" : "FAIL",
           (unsigned long long)(RECUR_MAX_N - failA), (unsigned long long)RECUR_MAX_N, timeA);

    /* (B) Real rule-level BFS ground truth. */
    const int BFS_MAX_N = 18; /* 2^18 = 262144 states -- instant */
    printf("[Check B] Cross-checking formula against a TRUE rule-level BFS search\n");
    printf("          (simulates the actual switch rules, not the recurrence) for n = 1..%d...\n", BFS_MAX_N);
    clock_t startB = clock();
    uint64_t failB = 0;
    for (int n = 1; n <= BFS_MAX_N; n++) {
        uint64_t f = calculate_min_moves((uint64_t)n);
        long long b = bfs_min_moves(n);
        if ((long long)f != b) {
            failB++;
            printf("  MISMATCH at n=%d: formula=%llu, bfs=%lld\n", n, (unsigned long long)f, b);
        }
    }
    double timeB = (double)(clock() - startB) / CLOCKS_PER_SEC;
    printf("  %s (%d / %d passed) in %.4f s\n\n",
           failB == 0 ? "PASS" : "FAIL", BFS_MAX_N - (int)failB, BFS_MAX_N, timeB);

    /* (D) Extended-range demonstration. */
    printf("[Check D] 128-bit extended path reaching the true hardware ceiling n=126:\n");
    unsigned __int128 huge = calculate_min_moves_ext(126);
    printf("  minimum_moves(126) = ");
    print_u128(huge);
    printf("\n  (uint64_t alone could never represent this -- it maxes out at n=62)\n\n");

    bool overall = known_ok && (failA == 0) && (failB == 0);
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
        printf("       SECURITY SWITCHES SOLVER            \n");
        printf("==========================================\n");
        printf("1. Solve for n switches (n up to 126)\n");
        printf("2. Run full validation suite\n");
        printf("3. Show step-by-step derivation for a specific n\n");
        printf("4. Run a true rule-level BFS check for a specific n (n <= 24)\n");
        printf("5. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Exiting.\n");
            break;
        }

        uint64_t n;
        switch (choice) {
            case 1:
                if (read_u64("Enter number of switches (n, up to 126): ", &n)) {
                    if (n <= 62) {
                        uint64_t moves = calculate_min_moves(n);
                        printf("\nResults for n = %llu switches:\n", (unsigned long long)n);
                        printf("  Minimum Moves Required: %llu\n\n", (unsigned long long)moves);
                    } else if (n <= 126) {
                        unsigned __int128 moves = calculate_min_moves_ext(n);
                        printf("\nResults for n = %llu switches (128-bit extended path):\n", (unsigned long long)n);
                        printf("  Minimum Moves Required: ");
                        print_u128(moves);
                        printf("\n\n");
                    } else {
                        printf("n=%llu exceeds the native 128-bit hardware limit (max n=126).\n", (unsigned long long)n);
                        printf("Beyond this, an arbitrary-precision library (e.g. GMP) is required.\n\n");
                    }
                } else {
                    printf("Invalid input. Please enter a non-negative integer.\n\n");
                }
                break;
            case 2:
                run_validation_suite();
                break;
            case 3:
                if (read_u64("Enter number of switches (n): ", &n)) {
                    explain_derivation(n);
                } else {
                    printf("Invalid input. Please enter a non-negative integer.\n\n");
                }
                break;
            case 4:
                if (read_u64("Enter number of switches for BFS check (n <= 24): ", &n)) {
                    if (n == 0) {
                        printf("BFS result for n=0: 0 moves.\n\n");
                    } else if (n <= 24) {
                        clock_t start = clock();
                        long long bfs_val = bfs_min_moves((int)n);
                        double t = (double)(clock() - start) / CLOCKS_PER_SEC;
                        uint64_t formula_val = (n <= 62) ? calculate_min_moves(n) : 0;
                        printf("\nBFS shortest path for n=%llu: %lld moves (computed in %.4f s over 2^%llu = %llu states)\n",
                               (unsigned long long)n, bfs_val, t,
                               (unsigned long long)n, (unsigned long long)((uint64_t)1 << n));
                        printf("Formula agrees: %s\n\n", ((uint64_t)bfs_val == formula_val) ? "YES" : "NO -- BUG!");
                    } else {
                        printf("n=%llu is too large for BFS (2^n states) -- capped at n=24 for safety.\n\n",
                               (unsigned long long)n);
                    }
                } else {
                    printf("Invalid input. Please enter a non-negative integer.\n\n");
                }
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