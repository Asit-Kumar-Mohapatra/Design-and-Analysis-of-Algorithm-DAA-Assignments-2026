/*
 * Optimal Binary Search Tree (OBST)
 *
 * Input representation (1-based keys, sorted, distinct):
 *   p[1..n]  : probability of searching key k_i
 *   q[0..n]  : probability of searching a value that falls in dummy key d_i
 *              (q[i] lies between k_i and k_{i+1})
 *
 * e[i][j] = minimum expected cost of a BST holding keys k_i..k_j (+ dummies d_{i-1}..d_j)
 * w(i,j)  = sum(p_i..p_j) + sum(q_{i-1}..q_j)  -> O(1) via prefix sums A[]
 *
 *   e[i][i-1] = q[i-1]
 *   e[i][j]   = min_{i<=r<=j} ( e[i][r-1] + e[r+1][j] ) + w(i,j)
 *
 * Knuth's optimisation: root[i][j-1] <= root[i][j] <= root[i+1][j].
 * Over one diagonal (fixed j-i) the ranges telescope:
 *   sum (root[i+1][j] - root[i][j-1] + 1) <= n + n = O(n),
 * so each diagonal costs O(n) and all n diagonals cost O(n^2).
 *
 * Time : O(n^2)   (plain DP is O(n^3))
 * Space: O(n^2)   (e table + root table; every sub-interval cost is needed)
 *
 * Input : n, then p_1..p_n, then q_0..q_n   (whitespace separated)
 * Output: minimum expected search cost e[1][n] (and the tree shape in a terminal)
 *
 * Usage : ./obst            reads from stdin (prompts only when typing in a terminal)
 *         ./obst --test     validates against the O(n^3) DP and the textbook example
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#ifdef _WIN32
#include <io.h>
#define IS_TTY() _isatty(_fileno(stdin))
#else
#include <unistd.h>
#define IS_TTY() isatty(0)
#endif

/* p[1..n], q[0..n]. Returns min expected cost, or -1 on allocation failure.
 * If root_out != NULL, *root_out receives the root table (row stride n+1,
 * root(i,j) at [i*(n+1)+j]); the caller must free it. */
static double obst_knuth(const double *p, const double *q, int n, int **root_out) {
    if (root_out) *root_out = NULL;
    if (n == 0) return q[0];
    size_t S = (size_t)n + 1;                       /* row stride, j in 0..n     */
    double *e = (double *)malloc((size_t)(n + 2) * S * sizeof(double));
    int *root = (int *)malloc((size_t)(n + 2) * S * sizeof(int));
    double *A = (double *)malloc(S * sizeof(double));
    if (!e || !root || !A) { free(e); free(root); free(A); return -1; }
#define E(i, j) e[(size_t)(i) * S + (j)]
#define R(i, j) root[(size_t)(i) * S + (j)]

    A[0] = 0;
    for (int k = 1; k <= n; k++) A[k] = A[k - 1] + p[k] + q[k];

    for (int i = 1; i <= n + 1; i++) E(i, i - 1) = q[i - 1];
    for (int i = 1; i <= n; i++) {                  /* length 1 */
        E(i, i) = E(i, i - 1) + E(i + 1, i) + (q[i - 1] + p[i] + q[i]);
        R(i, i) = i;
    }
    for (int len = 2; len <= n; len++) {
        for (int i = 1, j = len; j <= n; i++, j++) {
            double w = A[j] - A[i - 1] + q[i - 1];
            double best = DBL_MAX;
            int br = i;
            int lo = R(i, j - 1), hi = R(i + 1, j);
            if (lo > hi) { lo = i; hi = j; }        /* only possible via rounding: full search */
            for (int r = lo; r <= hi; r++) {
                double c = E(i, r - 1) + E(r + 1, j);
                if (c < best) { best = c; br = r; }
            }
            E(i, j) = best + w;
            R(i, j) = br;
        }
    }
    double ans = E(1, n);
    free(e); free(A);
    if (root_out) *root_out = root; else free(root);
    return ans;
#undef E
#undef R
}

/* Prints the tree in CLRS style. Dummy leaf d_j is reported when i > j. */
static void print_tree(const int *root, int n, int i, int j, int parent, const char *side) {
    if (i > j) { printf("  d%d is the %s child of k%d\n", j, side, parent); return; }
    int r = root[(size_t)i * ((size_t)n + 1) + j];
    if (parent == 0) printf("  k%d is the root\n", r);
    else             printf("  k%d is the %s child of k%d\n", r, side, parent);
    print_tree(root, n, i, r - 1, r, "left");
    print_tree(root, n, r + 1, j, r, "right");
}

/* Reference O(n^3) DP, used only for validation. Returns -1 on allocation failure. */
static double obst_cubic(const double *p, const double *q, int n) {
    size_t S = (size_t)n + 1;
    double *e = (double *)malloc((size_t)(n + 2) * S * sizeof(double));
    double *w = (double *)malloc((size_t)(n + 2) * S * sizeof(double));
    if (!e || !w) { free(e); free(w); return -1; }
    for (int i = 1; i <= n + 1; i++) { e[i * S + i - 1] = q[i - 1]; w[i * S + i - 1] = q[i - 1]; }
    for (int len = 1; len <= n; len++)
        for (int i = 1; i + len - 1 <= n; i++) {
            int j = i + len - 1;
            w[i * S + j] = w[i * S + j - 1] + p[j] + q[j];
            double best = DBL_MAX;
            for (int r = i; r <= j; r++) {
                double c = e[i * S + r - 1] + e[(r + 1) * S + j];
                if (c < best) best = c;
            }
            e[i * S + j] = best + w[i * S + j];
        }
    double ans = e[1 * S + n];
    free(e); free(w);
    return ans;
}

static int run_tests(void) {
    /* Textbook example (CLRS 15.5): expected 2.75 */
    double p0[] = {0, 0.15, 0.10, 0.05, 0.10, 0.20};
    double q0[] = {0.05, 0.10, 0.05, 0.05, 0.05, 0.10};
    double c0 = obst_knuth(p0, q0, 5, NULL);
    printf("CLRS example: %.2f (expected 2.75)\n", c0);
    if (fabs(c0 - 2.75) > 1e-9) return 1;

    srand(2024);
    for (int t = 0; t < 20000; t++) {               /* integer weights */
        int n = rand() % 30 + 1;
        double p[32], q[32];
        p[0] = 0;
        for (int i = 1; i <= n; i++) p[i] = rand() % 20 + 1;
        for (int i = 0; i <= n; i++) q[i] = rand() % 20;
        double a = obst_knuth(p, q, n, NULL), b = obst_cubic(p, q, n);
        if (fabs(a - b) > 1e-6) { printf("MISMATCH (int) test %d: %f vs %f\n", t, a, b); return 1; }
    }
    for (int t = 0; t < 20000; t++) {               /* decimal, normalised weights */
        int n = rand() % 30 + 1;
        double p[32], q[32], sum = 0;
        p[0] = 0;
        for (int i = 1; i <= n; i++) { p[i] = (rand() % 1000 + 1) / 1000.0; sum += p[i]; }
        for (int i = 0; i <= n; i++) { q[i] = (rand() % 1000) / 1000.0;     sum += q[i]; }
        for (int i = 1; i <= n; i++) p[i] /= sum;
        for (int i = 0; i <= n; i++) q[i] /= sum;
        double a = obst_knuth(p, q, n, NULL), b = obst_cubic(p, q, n);
        if (a < 0 || b < 0 || fabs(a - b) > 1e-9 * (b > 1 ? b : 1)) {
            printf("MISMATCH (real) test %d: %.12f vs %.12f\n", t, a, b); return 1;
        }
    }
    puts("All 40000 random tests passed");
    return 0;
}

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--test") == 0) return run_tests();

    int tty = IS_TTY();
    int n;
    if (tty) printf("Enter n (number of keys): ");
    if (scanf("%d", &n) != 1 || n < 0) { fprintf(stderr, "Invalid n\n"); return 1; }
    double *p = (double *)calloc((size_t)n + 1, sizeof(double));
    double *q = (double *)calloc((size_t)n + 1, sizeof(double));
    if (!p || !q) { fprintf(stderr, "Out of memory\n"); free(p); free(q); return 1; }
    if (tty) printf("Enter %d key probabilities p1..p%d: ", n, n);
    for (int i = 1; i <= n; i++)
        if (scanf("%lf", &p[i]) != 1) { fprintf(stderr, "Invalid p input\n"); free(p); free(q); return 1; }
    if (tty) printf("Enter %d dummy probabilities q0..q%d: ", n + 1, n);
    for (int i = 0; i <= n; i++)
        if (scanf("%lf", &q[i]) != 1) { fprintf(stderr, "Invalid q input\n"); free(p); free(q); return 1; }

    double total = 0;
    for (int i = 1; i <= n; i++) total += p[i];
    for (int i = 0; i <= n; i++) total += q[i];
    if (tty && fabs(total - 1.0) > 1e-6)
        printf("Warning: probabilities sum to %.6f, not 1.\n", total);

    int *root = NULL;
    double ans = obst_knuth(p, q, n, (tty && n >= 1 && n <= 100) ? &root : NULL);
    if (ans < 0) { fprintf(stderr, "Out of memory\n"); free(p); free(q); return 1; }

    if (tty) {
        printf("Minimum expected search cost = %.6f\n", ans);
        if (root) { printf("Optimal tree:\n"); print_tree(root, n, 1, n, 0, ""); }
    } else printf("%.6f\n", ans);

    free(root); free(p); free(q);
    return 0;
}