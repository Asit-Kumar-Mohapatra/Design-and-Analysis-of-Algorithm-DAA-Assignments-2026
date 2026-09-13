#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <math.h>

/* ============================================================================
 * PROBLEM: MATRIX CHAIN MULTIPLICATION (MCM)
 * ============================================================================
 * Input : dimensions p[0..n], where matrix A_i has size p[i-1] x p[i], for a
 *         chain of n matrices A_1, A_2, ..., A_n.
 * Output: the minimum total number of scalar multiplications needed to
 *         compute A_1 * A_2 * ... * A_n, and a parenthesization achieving it.
 *         (Matrix multiplication is associative, so different groupings give
 *         the same product but wildly different amounts of work.)
 *
 * -------------------------------  APPROACH  --------------------------------
 * This is the textbook "interval DP" problem.
 *
 * 1. DP STATE
 *      m[i][j] = the minimum number of scalar multiplications needed to
 *                compute the product A_i * A_(i+1) * ... * A_j.
 *      Base case: m[i][i] = 0 (a single matrix needs no multiplication).
 *
 * 2. RECURRENCE
 *      To compute A_i..A_j, some multiplication is the LAST one performed;
 *      say it splits the chain into (A_i..A_k) and (A_(k+1)..A_j). Those two
 *      sub-products are matrices of size p[i-1] x p[k] and p[k] x p[j], so
 *      multiplying them costs p[i-1]*p[k]*p[j] scalar multiplications. Trying
 *      every possible split point k and keeping the cheapest gives:
 *
 *          m[i][j] = min over i <= k < j of:
 *                      m[i][k] + m[k+1][j] + p[i-1]*p[k]*p[j]
 *
 *      We also store s[i][j] = the k that achieved the minimum, so the
 *      optimal parenthesization can be reconstructed afterwards.
 *
 * 3. EVALUATION ORDER
 *      m[i][j] depends only on sub-chains SHORTER than (j - i + 1), so we
 *      fill the table by increasing chain length L = j - i + 1, from L = 2
 *      up to L = n. This guarantees every value m[i][j] needs is already
 *      computed by the time we need it.
 *
 * WHY THIS IS CORRECT (optimal substructure + no missed cases)
 *      Any valid full parenthesization of A_i..A_j has SOME last
 *      multiplication, and that multiplication necessarily splits the chain
 *      at some k with i <= k < j. The recurrence enumerates every possible
 *      k, so the true optimum for (i, j) is guaranteed to be among the
 *      candidates considered - and picking the best sub-solutions for each
 *      side is valid because the cost of (A_i..A_k) doesn't depend on how
 *      (A_(k+1)..A_j) is parenthesized, and vice versa (optimal
 *      substructure). Hence m[i][j] as computed equals the true minimum.
 *
 * COMPLEXITY
 *      There are O(n^2) states (i, j). Each state tries O(n) split points.
 *      Time  : O(n^3)
 *      Space : O(n^2)   for the m and s tables.
 *
 *      This is the standard, optimal DP complexity for this problem; the
 *      naive alternative of trying every possible full parenthesization is
 *      exponential (Catalan(n-1) of them), so the DP is an enormous
 *      practical improvement even though O(n^3) sounds large in isolation.
 * ============================================================================
 */

/* ---------------------------------------------------------------------- */
/*  Allocate an (n+1) x (n+1) table of the given element size, zeroed.     */
/* ---------------------------------------------------------------------- */
static long long **alloc_ll_table(int n) {
    long long **t = (long long **)malloc((size_t)(n + 1) * sizeof(long long *));
    if (!t) { fprintf(stderr, "Out of memory\n"); exit(EXIT_FAILURE); }
    for (int i = 0; i <= n; i++) {
        t[i] = (long long *)calloc((size_t)(n + 1), sizeof(long long));
        if (!t[i]) { fprintf(stderr, "Out of memory\n"); exit(EXIT_FAILURE); }
    }
    return t;
}

static int **alloc_int_table(int n) {
    int **t = (int **)malloc((size_t)(n + 1) * sizeof(int *));
    if (!t) { fprintf(stderr, "Out of memory\n"); exit(EXIT_FAILURE); }
    for (int i = 0; i <= n; i++) {
        t[i] = (int *)calloc((size_t)(n + 1), sizeof(int));
        if (!t[i]) { fprintf(stderr, "Out of memory\n"); exit(EXIT_FAILURE); }
    }
    return t;
}

static void free_ll_table(long long **t, int n) {
    for (int i = 0; i <= n; i++) free(t[i]);
    free(t);
}
static void free_int_table(int **t, int n) {
    for (int i = 0; i <= n; i++) free(t[i]);
    free(t);
}

/* ---------------------------------------------------------------------- */
/*  Reconstructs and prints the optimal parenthesization from s[][].       */
/* ---------------------------------------------------------------------- */
void print_optimal_parens(int **s, int i, int j) {
    if (i == j) {
        printf("A%d", i);
    } else {
        printf("(");
        print_optimal_parens(s, i, s[i][j]);
        print_optimal_parens(s, s[i][j] + 1, j);
        printf(")");
    }
}

/* ---------------------------------------------------------------------- */
/*  Core DP algorithm. n = number of matrices, p[0..n] = dimensions.       */
/*  O(n^3) time, O(n^2) space. If trace != 0, prints the table fill order  */
/*  and the completed m[][] / s[][] tables (only meant for small n).      */
/*  On return, *out_s (if non-NULL) receives the split table so the       */
/*  caller can print the parenthesization; caller must free it with       */
/*  free_int_table(*out_s, n).                                            */
/* ---------------------------------------------------------------------- */
long long matrix_chain_order(const int *p, int n, int trace, int ***out_s) {
    if (n <= 0) { if (out_s) *out_s = NULL; return 0; }

    long long **m = alloc_ll_table(n);
    int **s = alloc_int_table(n);

    if (trace) printf("\nFilling DP table by increasing chain length L:\n");

    for (int L = 2; L <= n; L++) {
        for (int i = 1; i <= n - L + 1; i++) {
            int j = i + L - 1;
            m[i][j] = LLONG_MAX;
            for (int k = i; k <= j - 1; k++) {
                long long cost_left  = m[i][k];
                long long cost_right = m[k + 1][j];
                long long merge_cost = (long long)p[i - 1] * p[k] * p[j];
                long long total = cost_left + cost_right + merge_cost;
                if (total < m[i][j]) {
                    m[i][j] = total;
                    s[i][j] = k;
                }
            }
            if (trace) {
                printf("  L=%d  m[%d][%d] = %lld   (split at k=%d: A%d..A%d | A%d..A%d)\n",
                       L, i, j, m[i][j], s[i][j], i, s[i][j], s[i][j] + 1, j);
            }
        }
    }

    if (trace && n <= 12) {
        printf("\nFinal m[i][j] table (minimum cost of A_i..A_j):\n     ");
        for (int j = 1; j <= n; j++) printf("%8d", j);
        printf("\n");
        for (int i = 1; i <= n; i++) {
            printf("  i=%d", i);
            for (int j = 1; j <= n; j++) {
                if (j < i) printf("%8s", "-");
                else printf("%8lld", m[i][j]);
            }
            printf("\n");
        }
    }

    long long result = m[1][n];
    free_ll_table(m, n);

    if (out_s) *out_s = s; else free_int_table(s, n);
    return result;
}

/* ---------------------------------------------------------------------- */
/*  Independent brute-force reference: tries EVERY possible full          */
/*  parenthesization via plain recursion (no memoization). Exponential    */
/*  (Catalan(n-1) parenthesizations), so only used for small n to         */
/*  validate the DP above.                                                */
/* ---------------------------------------------------------------------- */
long long mcm_brute_force(const int *p, int i, int j) {
    if (i == j) return 0;
    long long best = LLONG_MAX;
    for (int k = i; k < j; k++) {
        long long left  = mcm_brute_force(p, i, k);
        long long right = mcm_brute_force(p, k + 1, j);
        long long merge = (long long)p[i - 1] * p[k] * p[j];
        long long total = left + right + merge;
        if (total < best) best = total;
    }
    return best;
}

/* ---------------------------------------------------------------------- */
/*  Recomputes the cost implied by a reconstructed parenthesization (s[][])*/
/*  independently of m[][], to sanity-check that the printed split table  */
/*  actually achieves the reported minimum (not just that the numbers     */
/*  match by construction).                                               */
/* ---------------------------------------------------------------------- */
static long long cost_from_split_table(const int *p, int **s, int i, int j) {
    if (i == j) return 0;
    int k = s[i][j];
    long long left  = cost_from_split_table(p, s, i, k);
    long long right = cost_from_split_table(p, s, k + 1, j);
    long long merge = (long long)p[i - 1] * p[k] * p[j];
    return left + right + merge;
}

/* ---------------------------------------------------------------------- */
/*  1) Custom / manual input mode                                         */
/* ---------------------------------------------------------------------- */
static void flush_stdin_line(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

void run_manual_mode(void) {
    int n;
    printf("\nHow many matrices are in the chain (n)? ");
    if (scanf("%d", &n) != 1 || n < 1) {
        printf("Please enter a positive integer.\n");
        flush_stdin_line();
        return;
    }
    flush_stdin_line();

    int *p = (int *)malloc((size_t)(n + 1) * sizeof(int));
    if (!p) { fprintf(stderr, "Out of memory\n"); return; }

    printf("Enter the %d dimensions p0 p1 ... p%d\n", n + 1, n);
    printf("(matrix A_i has size p[i-1] x p[i], all dimensions must be positive):\n");
    for (int i = 0; i <= n; i++) {
        printf("  p[%d] = ", i);
        while (scanf("%d", &p[i]) != 1 || p[i] <= 0) {
            printf("  Invalid (must be a positive integer), try again p[%d] = ", i);
            flush_stdin_line();
        }
    }
    flush_stdin_line();

    int show_trace = 0;
    printf("Show the full DP table fill trace? (1 = yes, 0 = no): ");
    if (scanf("%d", &show_trace) != 1) show_trace = 0;
    flush_stdin_line();

    int **s = NULL;
    long long min_ops = matrix_chain_order(p, n, show_trace, &s);

    printf("\n================ RESULT ================\n");
    printf("Minimum scalar multiplications : %lld\n", min_ops);
    printf("Optimal parenthesization        : ");
    print_optimal_parens(s, 1, n);
    printf("\n=========================================\n");

    free_int_table(s, n);
    free(p);
}

/* ---------------------------------------------------------------------- */
/*  2) Worked example / explanation demo (classic CLRS example)           */
/* ---------------------------------------------------------------------- */
void run_explanation_demo(void) {
    /* A1: 30x35, A2: 35x15, A3: 15x5, A4: 5x10, A5: 10x20, A6: 20x25 */
    int p[] = {30, 35, 15, 5, 10, 20, 25};
    int n = (int)(sizeof(p) / sizeof(p[0])) - 1;

    printf("\n--- Worked example: %d matrices ---\n", n);
    for (int i = 1; i <= n; i++)
        printf("  A%d : %d x %d\n", i, p[i - 1], p[i]);

    printf("\nStep 1: m[i][i] = 0 for every single matrix (base case).\n");
    printf("Step 2: fill m[i][j] for increasing chain length L = j-i+1,\n");
    printf("        trying every split point k and keeping the cheapest.\n");

    int **s = NULL;
    long long min_ops = matrix_chain_order(p, n, 1 /* trace on */, &s);

    printf("\n================ RESULT ================\n");
    printf("Minimum scalar multiplications : %lld\n", min_ops);
    printf("Optimal parenthesization        : ");
    print_optimal_parens(s, 1, n);
    printf("\n");
    printf("(Classic CLRS example -- expected answer is 15125 with\n");
    printf(" parenthesization ((A1(A2A3))((A4A5)A6)).)\n");
    printf("=========================================\n");

    free_int_table(s, n);
}

/* ---------------------------------------------------------------------- */
/*  3) Validation suite: correctness cross-check + performance/complexity */
/* ---------------------------------------------------------------------- */
static void fill_random_dims(int *p, int n, int lo, int hi) {
    for (int i = 0; i <= n; i++) p[i] = lo + rand() % (hi - lo + 1);
}

/* One deterministic named edge case, checked against brute force AND
 * against re-deriving the cost from the printed split table. */
static int check_case(const char *label, const int *p, int n, long long expect) {
    int **s = NULL;
    long long dp = matrix_chain_order(p, n, 0, &s);
    long long brute = mcm_brute_force(p, 1, n);
    long long from_split = cost_from_split_table(p, s, 1, n);
    int ok = (dp == brute) && (dp == from_split) && (expect < 0 || dp == expect);
    printf("  %-46s dp=%-8lld brute=%-8lld split=%-8lld %s\n",
           label, dp, brute, from_split, ok ? "PASS" : "FAIL");
    free_int_table(s, n);
    return ok;
}

void run_validation_suite(void) {
    srand((unsigned)time(NULL));

    /* --- Part C: hand-picked edge cases --- */
    printf("\n--- Part C: Edge-case regression tests ---\n");
    int edge_failures = 0;
    {
        int p1[] = {10, 20};                       /* n=1: single matrix       */
        edge_failures += !check_case("n=1, single matrix", p1, 1, 0);

        int p2[] = {10, 20, 30};                    /* n=2: only one way to go */
        edge_failures += !check_case("n=2, only one possible split", p2, 2, 10*20*30);

        int p3[] = {30, 35, 15, 5, 10, 20, 25};      /* classic CLRS example    */
        edge_failures += !check_case("classic CLRS 6-matrix example", p3, 6, 15125);

        int p4[] = {40, 20, 30, 10, 30};             /* another classic example */
        edge_failures += !check_case("classic 4-matrix example", p4, 4, 26000);

        int p5[] = {5, 5, 5, 5, 5};                  /* all equal dimensions    */
        edge_failures += !check_case("all-equal dimensions", p5, 4, -1);

        int p6[] = {1, 100, 1, 100, 1};              /* extreme alternating     */
        edge_failures += !check_case("extreme alternating dimensions", p6, 4, -1);
    }
    if (edge_failures == 0)
        printf("  All edge cases PASSED.\n");
    else
        printf("  %d edge case(s) FAILED.\n", edge_failures);

    /* --- Part A: correctness, DP vs. brute force, many random trials --- */
    printf("\n--- Part A: Correctness cross-check vs. brute force ---\n");
    int trials = 300;
    int mismatches = 0;
    for (int t = 0; t < trials; t++) {
        int n = 1 + rand() % 10;                     /* keep small: brute force is exponential */
        int *p = (int *)malloc((size_t)(n + 1) * sizeof(int));
        fill_random_dims(p, n, 1, 50);

        int **s = NULL;
        long long dp = matrix_chain_order(p, n, 0, &s);
        long long brute = mcm_brute_force(p, 1, n);
        long long from_split = cost_from_split_table(p, s, 1, n);

        if (dp != brute || dp != from_split) {
            mismatches++;
            printf("  MISMATCH trial %d (n=%d): dp=%lld brute=%lld split=%lld\n",
                   t, n, dp, brute, from_split);
        }
        free_int_table(s, n);
        free(p);
    }
    if (mismatches == 0)
        printf("  All %d randomized trials agree with brute force. Verdict: CORRECT\n", trials);
    else
        printf("  %d / %d trials disagreed. Verdict: BUG\n", mismatches, trials);

    /* --- Part B: performance, demonstrating O(n^3) scaling --- */
    printf("\n--- Part B: Performance scaling (O(n^3) check) ---\n");
    int sizes[] = {50, 100, 200, 400, 800};
    int num_sizes = (int)(sizeof(sizes) / sizeof(sizes[0]));

    printf("  %-8s %-12s %-16s\n", "n", "time (s)", "time / n^3");
    for (int s_idx = 0; s_idx < num_sizes; s_idx++) {
        int n = sizes[s_idx];
        int *p = (int *)malloc((size_t)(n + 1) * sizeof(int));
        fill_random_dims(p, n, 1, 100);

        clock_t start = clock();
        long long min_ops = matrix_chain_order(p, n, 0, NULL);
        clock_t end = clock();
        double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
        double n3 = (double)n * n * n;

        printf("  %-8d %-12.4f %-16.10f  (min_ops=%lld)\n",
               n, elapsed, elapsed / n3, min_ops);
        free(p);
    }
    printf("\n  If the algorithm were, say, O(n^4), 'time / n^3' would grow\n");
    printf("  steadily as n increases; instead it stays roughly flat, which\n");
    printf("  is the empirical signature of O(n^3) behaviour.\n");
}

/* ---------------------------------------------------------------------- */
/*  4) Plain-text explanation (no computation)                            */
/* ---------------------------------------------------------------------- */
void print_explanation(void) {
    printf(
"\n"
"===================== HOW THE ALGORITHM WORKS =====================\n"
"1. State: m[i][j] = minimum scalar multiplications to compute the\n"
"   product A_i * A_(i+1) * ... * A_j. Base case m[i][i] = 0.\n"
"\n"
"2. Recurrence: the LAST multiplication in any parenthesization of\n"
"   A_i..A_j splits it at some k (i <= k < j) into (A_i..A_k) and\n"
"   (A_(k+1)..A_j). Those two pieces are p[i-1] x p[k] and p[k] x p[j]\n"
"   matrices, so combining them costs p[i-1]*p[k]*p[j]. Trying every k:\n"
"\n"
"       m[i][j] = min_{i<=k<j} ( m[i][k] + m[k+1][j] + p[i-1]*p[k]*p[j] )\n"
"\n"
"3. Fill order: compute by increasing chain length L = j-i+1, from 2\n"
"   up to n, so every m[i][k] and m[k+1][j] needed is already known.\n"
"\n"
"4. Store s[i][j] = the best k for each (i,j) while filling the table,\n"
"   then recursively reconstruct the optimal parenthesization from it.\n"
"\n"
"COMPLEXITY\n"
"   Time : O(n^3)  -  O(n^2) states, O(n) work (split points) each.\n"
"   Space: O(n^2)  -  the m and s tables.\n"
"\n"
"WHY DP INSTEAD OF BRUTE FORCE\n"
"   The number of distinct full parenthesizations of n matrices is the\n"
"   Catalan number C(n-1), which grows exponentially. DP avoids\n"
"   recomputing the same sub-chain costs over and over (overlapping\n"
"   subproblems), collapsing exponential brute force into O(n^3).\n"
"=====================================================================\n");
}

/* ---------------------------------------------------------------------- */
int main(void) {
    int choice;

    while (1) {
        printf("\n==============================================\n");
        printf("     MATRIX CHAIN MULTIPLICATION - SOLVER\n");
        printf("==============================================\n");
        printf("1. Enter your own matrix chain and compute the answer\n");
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