/*
 * Minimum Coin Change  --  unbounded knapsack (bottom-up DP)
 * ==========================================================
 * Problem : given coin values C = {c1..cn} (infinite supply of each) and a
 *           target V, find the minimum number of coins that sum to V,
 *           or -1 if V cannot be formed.
 *
 * Recurrence
 *     dp[0] = 0
 *     dp[v] = min over coins c <= v of ( dp[v - c] + 1 ),   dp[v] = INF if none
 *   Answer = dp[V] (or -1 if dp[V] == INF).
 *   Correctness: an optimal solution for v ends with some coin c; removing it
 *   leaves an optimal solution for v - c (optimal substructure).
 *
 * Complexity (derivation)
 *   Time  : for each of the n coins the inner loop visits at most V cells and
 *           does O(1) work per cell (one add, one compare)
 *           => T(n, V) = n * V * O(1) = O(n * V).
 *           Pseudo-polynomial: V is a numeric value, not the input length in bits.
 *   Space : a single array of V+1 entries => O(V).  (4 bytes per entry.)
 *
 * Return codes of coinChange():  >= 0 answer | -1 impossible | -2 V too large
 *                                / memory unavailable (a table cannot be built).
 *
 * Input format (whitespace separated; spaces or newlines both fine):
 *     T                 number of test cases
 *     per case:  n      number of coin types
 *                c1..cn coin values
 *                V      target amount
 * Output: one line per case containing only the answer (-1 if impossible).
 *         Prompts are shown only when stdin is an interactive terminal.
 *
 * Example  input: "2  3  1 2 5  11  1  2  3"   ->   output: "3" and "-1"
 *
 * Build : gcc -O2 -Wall -Wextra -o coin_change coin_change.c
 * Test  : gcc -O2 -DTEST -o test coin_change.c && ./test
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>          /* isatty */

#define MAX_V 4294967293LL   /* keeps INF+1 inside uint32_t (no wrap-around) */

long long coinChange(const long long *coins, int n, long long V)
{
    if (V < 0) return -1;
    if (V == 0) return 0;                               /* zero coins needed */
    if (V > MAX_V || (uint64_t)(V + 1) > SIZE_MAX / sizeof(uint32_t)) return -2;

    uint32_t *dp = (uint32_t *)malloc((size_t)(V + 1) * sizeof(uint32_t));
    if (!dp) return -2;

    const uint32_t INF = (uint32_t)V + 1;   /* any real answer <= V; INF+1 fits in uint32 */
    const size_t   sz  = (size_t)V;
    dp[0] = 0;
    for (size_t v = 1; v <= sz; v++) dp[v] = INF;

    for (int i = 0; i < n; i++) {
        long long cl = coins[i];
        if (cl <= 0 || cl > V) continue;    /* useless / invalid coin */
        size_t c = (size_t)cl;
        for (size_t v = c; v <= sz; v++) {
            uint32_t t = dp[v - c] + 1;
            if (t < dp[v]) dp[v] = t;
        }
    }
    long long ans = dp[sz] >= INF ? -1 : (long long)dp[sz];
    free(dp);
    return ans;
}

#ifdef TEST
/* ---------------------------- self-test ------------------------------ */
static long long coinChangeInt(const int *c, int n, long long V)
{
    long long *t = malloc((n ? (size_t)n : 1) * sizeof *t);
    for (int i = 0; i < n; i++) t[i] = c[i];
    long long r = coinChange(t, n, V);
    free(t);
    return r;
}

static long long bfsRef(const int *coins, int n, int V)
{
    if (V < 0) return -1;
    if (V == 0) return 0;
    int *dist = malloc((V + 1) * sizeof(int)), *q = malloc((V + 1) * sizeof(int));
    for (int i = 0; i <= V; i++) dist[i] = -1;
    int h = 0, t = 0, res = -1; q[t++] = 0; dist[0] = 0;
    while (h < t && res < 0) {
        int u = q[h++];
        for (int i = 0; i < n; i++) {
            if (coins[i] <= 0) continue;
            long long w = (long long)u + coins[i];
            if (w > V || dist[w] != -1) continue;
            dist[w] = dist[u] + 1;
            if (w == V) { res = dist[w]; break; }
            q[t++] = (int)w;
        }
    }
    free(dist); free(q);
    return res;
}

static long long brute(const int *c, int n, long long V)
{
    if (n == 0) return V == 0 ? 0 : -1;
    long long best = -1; int cc = c[n - 1];
    if (cc <= 0) return brute(c, n - 1, V);
    for (long long k = 0; k * cc <= V; k++) {
        long long r = brute(c, n - 1, V - k * cc);
        if (r >= 0 && (best < 0 || r + k < best)) best = r + k;
    }
    return best;
}

static int fail = 0, total = 0;
static void check(long long got, long long exp, const char *tag)
{
    total++;
    if (got != exp) { fail++; printf("FAIL [%s] got %lld expected %lld\n", tag, got, exp); }
}

int main(void)
{
    srand(2024);

    /* 1. hand-made edge cases */
    struct { int c[6]; int n; long long V, exp; } E[] = {
        {{1,2,5},3,11,3}, {{2},1,3,-1}, {{1},1,0,0}, {{1,3,4},3,6,2}, {{5,10},2,3,-1},
        {{0},0,0,0}, {{0},0,7,-1}, {{7},1,7,1}, {{186,419,83,408},4,6249,20},
        {{2,5,10,1},4,27,4}, {{3,7},2,11,-1}, {{1},1,-5,-1}, {{0,-3,4},3,8,2},
        {{2147483647},1,5,-1}, {{2147483647,1},2,100,100}, {{5,5,5},3,15,3}
    };
    for (unsigned k = 0; k < sizeof E / sizeof E[0]; k++)
        check(coinChangeInt(E[k].c, E[k].n, E[k].V), E[k].exp, "edge");

    /* coins beyond int range now handled by the long long API */
    { long long c[2] = {5000000000LL, 3};
      check(coinChange(c, 2, 10), -1, "big-coin-unused");
      check(coinChange(c, 2, 9), 3, "big-coin-mixed"); }

    /* 2. 10000 small random tests vs BFS (+ exhaustive on tiny ones) */
    for (int t = 0; t < 10000; t++) {
        int n = rand() % 9, mode = rand() % 5;
        int maxc = mode==0?5 : mode==1?20 : mode==2?100 : mode==3?1000 : 12;
        int V = mode==4 ? rand()%60 : rand()%3000; if (t%50==0) V = 0;
        int c[8];
        for (int i = 0; i < n; i++) { c[i] = rand()%(maxc+3)-1; if (mode==2 && rand()%3==0) c[i]=1; }
        long long a = coinChangeInt(c, n, V);
        check(a, bfsRef(c, n, V), "small-bfs");
        if (V <= 40 && n <= 5) check(a, brute(c, n, V), "small-brute");
    }

    /* 3. larger V, many coins, large denominations (vs BFS) */
    for (int t = 0; t < 300; t++) {
        int n = 1 + rand() % 50, V = rand() % 300000, c[50];
        int maxc = (t%3==0) ? 50 : (t%3==1) ? 5000 : 1000000;
        for (int i = 0; i < n; i++) c[i] = 1 + rand() % maxc;
        check(coinChangeInt(c, n, V), bfsRef(c, n, V), "large-bfs");
    }

    /* 4. closed-form checks */
    for (int k = 1; k <= 200; k++) {              /* coins {1..k}: answer = ceil(V/k) */
        int c[200]; for (int i = 0; i < k; i++) c[i] = i + 1;
        long long V = 100000 + rand() % 50000;
        check(coinChangeInt(c, k, V), (V + k - 1) / k, "ceil(V/k)");
    }
    for (int t = 0; t < 200; t++) {               /* single coin c: V/c if divisible else -1 */
        int c = 1 + rand() % 1000, V = rand() % 200000;
        check(coinChangeInt(&c, 1, V), V % c == 0 ? V / c : -1, "single");
    }
    { int c[2] = {1000, 1001};
      check(coinChangeInt(c, 2, 500000), bfsRef(c, 2, 500000), "frobenius"); }

    /* 5. overflow / size guards */
    { long long c[1] = {1};
      check(coinChange(c, 1, MAX_V + 1), -2, "too-large");
      check(coinChange(c, 1, 9000000000000000000LL), -2, "way-too-large");
    }

    printf("Total tests: %d | Failures: %d\n", total, fail);
    return fail != 0;
}
#else
/* ------------------------------ driver ------------------------------- */
int main(void)
{
    const int tty = isatty(STDIN_FILENO);       /* prompts only when interactive */
    int T;
    if (tty) printf("Number of test cases T: ");
    if (scanf("%d", &T) != 1 || T < 0) { fprintf(stderr, "invalid T\n"); return 1; }

    for (int tc = 1; tc <= T; tc++) {
        int n; long long V;
        if (tty) printf("\n[Case %d] Number of coin types n: ", tc);
        if (scanf("%d", &n) != 1 || n < 0) { fprintf(stderr, "invalid n\n"); return 1; }

        long long *c = (long long *)malloc((n ? (size_t)n : 1) * sizeof *c);
        if (!c) { fprintf(stderr, "out of memory\n"); return 1; }
        if (tty) printf("[Case %d] Enter %d coin values: ", tc, n);
        for (int i = 0; i < n; i++)
            if (scanf("%lld", &c[i]) != 1) { fprintf(stderr, "invalid coin\n"); free(c); return 1; }
        if (tty) printf("[Case %d] Target amount V: ", tc);
        if (scanf("%lld", &V) != 1) { fprintf(stderr, "invalid V\n"); free(c); return 1; }

        long long r = coinChange(c, n, V);
        if (r == -2) {
            fprintf(stderr, "Case %d: V too large / out of memory\n", tc);
            r = -1;
        }
        if (tty) printf("[Case %d] Answer: %lld\n", tc, r);
        else     printf("%lld\n", r);
        free(c);
    }
    return 0;
}
#endif