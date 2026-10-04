/*
 * Rod Cutting with Reconstruction
 *   (i)  maximum revenue obtainable by cutting a rod of length n
 *   (ii) the exact piece lengths of one optimal decomposition
 *
 * PORTABLE ANSI C (C89): no type aliases, no struct, no long long, no compiler builtins.
 *
 * Input representation : n (rod length) followed by the n prices p1 p2 ... pn,
 *                        where p_i is the price of a piece of length i.
 *                        Prices are non-negative whole numbers (each must fit in a C long).
 *
 * Algorithm  : bottom-up DP (unbounded-knapsack style)
 *      r[0] = 0
 *      r[j] = max over i = 1..j of ( p_i + r[j-i] )     -> best revenue for a rod of length j
 *      first[j] = the i that achieved the maximum       -> first piece of an optimal cut of j
 *      cnt[j]   = fewest pieces among all optimal cuts of j (used to break ties)
 *   Reconstruction: j = n; while (j > 0) { print first[j]; j = j - first[j]; }
 *   i = j (leaving the rod uncut) is one of the choices, so "no cuts" is covered.
 *
 * Big numbers: r[j] is stored as an exact big integer (4 limbs of base 10^9, up to 10^36),
 *              so the revenue can never overflow: it is at most n * max(p_i), far below 10^36.
 *
 * Time  : O(n^2)   (n values of j, up to j choices of i each; 4-limb arithmetic is a constant)
 * Space : O(n)     (r[], first[], cnt[] and the prices)
 *
 * Output: maximum revenue, the piece lengths (in cutting order), number of cuts.
 *         Among all optimal decompositions the one with the FEWEST pieces is reported
 *         (ties beyond that prefer the larger first piece).
 *
 * Build : gcc -std=c89 -O2 -Wall -Wextra -pedantic -o rod rod.c
 * Test  : ./rod --test        (validates against exhaustive search over all cuttings)
 * Judge : add -DPROMPTS=0 to print only the revenue and the pieces
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define BASE 1000000000UL
#define L 4

#ifndef PROMPTS
#define PROMPTS 1   /* 1 = friendly prompts / full report, 0 = silent (online judge) */
#endif

/* Solves the DP.  r must have (n+1)*L limbs and first (n+1) ints (caller-allocated).
   Returns 0 on success, 1 if memory could not be allocated.                         */
static int rod_cut(long n, const long *p, unsigned long *r, int *first)
{
    unsigned long *pl;
    long *cnt;
    long i, j, best_i, c;
    unsigned long cand[L], carry, s, u;
    int t, cmp;

    pl  = (unsigned long *)calloc((size_t)(n + 1) * L, sizeof(unsigned long));
    cnt = (long *)calloc((size_t)(n + 1), sizeof(long));
    if (!pl || !cnt) { free(pl); free(cnt); return 1; }
    memset(r, 0, (size_t)(n + 1) * L * sizeof(unsigned long));
    memset(first, 0, (size_t)(n + 1) * sizeof(int));

    /* price i as 4 base-10^9 limbs */
    for (i = 1; i <= n; i++) {
        u = (unsigned long)p[i];
        for (t = 0; t < L; t++) { pl[i * L + t] = u % BASE; u /= BASE; }
    }

    /* r[0] = 0, cnt[0] = 0 already (calloc / memset) */
    for (j = 1; j <= n; j++) {
        unsigned long *best = r + j * L;
        best_i = 0;
        for (i = 1; i <= j; i++) {
            /* cand = p_i + r[j-i] */
            carry = 0UL;
            for (t = 0; t < L; t++) {
                s = pl[i * L + t] + r[(j - i) * L + t] + carry;
                if (s >= BASE) { s -= BASE; carry = 1UL; } else carry = 0UL;
                cand[t] = s;
            }
            c = 1 + cnt[j - i];                 /* pieces used by this choice */
            if (best_i == 0) cmp = 1;
            else {
                cmp = 0;
                for (t = L - 1; t >= 0; t--) {
                    if (cand[t] != best[t]) { cmp = (cand[t] > best[t]) ? 1 : -1; break; }
                }
                /* equal revenue: fewer pieces wins; still tied: larger first piece wins */
                if (cmp == 0) cmp = (c < cnt[j]) ? 1 : (c == cnt[j]) ? 0 : -1;
            }
            if (cmp >= 0) {
                for (t = 0; t < L; t++) best[t] = cand[t];
                cnt[j] = c;
                best_i = i;
            }
        }
        first[j] = (int)best_i;
    }
    free(pl);
    free(cnt);
    return 0;
}

/* writes the big number (limbs little-endian) as decimal text; buf needs >= 40 chars */
static void big_to_str(const unsigned long *res, char *buf)
{
    int top = L - 1, t;
    while (top > 0 && res[top] == 0UL) top--;
    buf += sprintf(buf, "%lu", res[top]);
    for (t = top - 1; t >= 0; t--) buf += sprintf(buf, "%09lu", res[t]);
}

/* ------------------------------ validation ------------------------------ */

static long tests = 0, fails = 0;

static void fail(const char *tag, long n)
{
    fails++;
    if (fails < 10) printf("FAIL [%s] n=%ld\n", tag, n);
}

/* exhaustive: try every composition of rem; track best revenue and fewest pieces */
static void rec(long rem, const long *p, long sum, long pieces, long *bestSum, long *bestPieces)
{
    long i;
    if (rem == 0) {
        if (sum > *bestSum || (sum == *bestSum && pieces < *bestPieces)) {
            *bestSum = sum; *bestPieces = pieces;
        }
        return;
    }
    for (i = 1; i <= rem; i++) rec(rem - i, p, sum + p[i], pieces + 1, bestSum, bestPieces);
}

/* run the DP on one instance and validate it against the exhaustive search */
static void validate(long n, const long *p)
{
    unsigned long r[(12 + 1) * L];
    int first[12 + 1];
    long bestSum = -1, bestPieces = 0, j, total, pieces, rev;
    char buf[64], expect[64];

    tests++;
    if (rod_cut(n, p, r, first) != 0) { fail("alloc", n); return; }
    rec(n, p, 0, 0, &bestSum, &bestPieces);
    if (n == 0) { bestSum = 0; bestPieces = 0; }

    big_to_str(r + n * L, buf);
    sprintf(expect, "%ld", bestSum);
    if (strcmp(buf, expect) != 0) { fail("revenue", n); return; }

    /* the reconstructed pieces must sum to n, be valid lengths, earn the max, use fewest pieces */
    j = n; total = 0; pieces = 0; rev = 0;
    while (j > 0) {
        if (first[j] < 1 || first[j] > j) { fail("bad piece", n); return; }
        total += first[j]; rev += p[first[j]]; pieces++;
        j -= first[j];
    }
    if (total != n) fail("sum of pieces", n);
    else if (rev != bestSum) fail("pieces revenue", n);
    else if (pieces != bestPieces) fail("fewest pieces", n);
}

static int run_tests(void)
{
    long p[13], n, i, t;
    long a, b;

    srand(2024);

    /* 1. textbook example (CLRS): prices 1 5 8 9 10 17 17 20 24 30 */
    {
        long clrs[] = {0, 1, 5, 8, 9, 10, 17, 17, 20, 24, 30};
        unsigned long r[11 * L];
        int first[11];
        char buf[64];
        long j;
        rod_cut(10, clrs, r, first);
        big_to_str(r + 10 * L, buf);
        tests++; if (strcmp(buf, "30") != 0) fail("clrs n=10", 10);
        rod_cut(7, clrs, r, first);
        big_to_str(r + 7 * L, buf);
        tests++; if (strcmp(buf, "18") != 0) fail("clrs n=7", 7);
        /* n=7 -> 18 by 1+6 or 2+2+3; fewest pieces = 2 pieces (1 + 6) */
        j = 7; t = 0;
        while (j > 0) { t++; j -= first[j]; }
        tests++; if (t != 2) fail("clrs pieces", 7);
    }

    /* 2. n = 0, and the rod left uncut is best */
    {
        long z[] = {0, 5};
        long uncut[] = {0, 1, 2, 3, 100};
        unsigned long r[5 * L];
        int first[5];
        char buf[64];
        rod_cut(0, z, r, first);
        big_to_str(r, buf);
        tests++; if (strcmp(buf, "0") != 0) fail("n=0", 0);
        rod_cut(4, uncut, r, first);
        big_to_str(r + 4 * L, buf);
        tests++; if (strcmp(buf, "100") != 0 || first[4] != 4) fail("uncut", 4);
    }

    /* 3. all-zero prices */
    for (i = 0; i <= 12; i++) p[i] = 0;
    for (n = 0; n <= 12; n++) validate(n, p);

    /* 4. 30000 random instances, tiny price range => many ties */
    for (t = 0; t < 30000; t++) {
        long range = rand() % 12 + 1;
        n = rand() % 13;
        p[0] = 0;
        for (i = 1; i <= 12; i++) p[i] = rand() % range;
        validate(n, p);
    }

    /* 5. exhaustive: every price vector over {0..3} for n = 6, over {0..2} for n = 7 */
    for (a = 0; a < 4096; a++) {
        b = a; p[0] = 0;
        for (i = 1; i <= 6; i++) { p[i] = b % 4; b /= 4; }
        validate(6, p);
    }
    for (a = 0; a < 2187; a++) {
        b = a; p[0] = 0;
        for (i = 1; i <= 7; i++) { p[i] = b % 3; b /= 3; }
        validate(7, p);
    }

    /* 6. big numbers: all prices = LONG_MAX, n = 10 -> best is ten pieces of length 1 */
#if LONG_MAX > 2147483647L
    {
        long big[11];
        unsigned long r[11 * L];
        int first[11];
        char buf[64];
        for (i = 0; i <= 10; i++) big[i] = LONG_MAX;
        big[0] = 0;
        rod_cut(10, big, r, first);
        big_to_str(r + 10 * L, buf);
        tests++;
        if (strcmp(buf, "92233720368547758070") != 0) fail("bignum", 10);
    }
#endif

    printf("Total checks: %ld | Failures: %ld\n", tests, fails);
    puts(fails == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED");
    return fails != 0;
}

/* --------------------------------- main --------------------------------- */

int main(int argc, char **argv)
{
    long n, i, j, k, cuts;
    long *p = NULL, *count = NULL;
    unsigned long *r = NULL;
    int *first = NULL;
    int status = 1;
    char buf[64];

    if (argc > 1 && strcmp(argv[1], "--test") == 0) return run_tests();

    if (PROMPTS) printf("Enter rod length (n): ");
    if (scanf("%ld", &n) != 1 || n < 0) { fprintf(stderr, "Error: invalid n\n"); return 1; }
    if ((size_t)n > ((size_t)-1) / (L * sizeof(unsigned long)) - 2) {
        fprintf(stderr, "Error: n too large\n");
        return 1;
    }

    p     = (long *)malloc((size_t)(n + 1) * sizeof(long));
    r     = (unsigned long *)calloc((size_t)(n + 1) * L, sizeof(unsigned long));
    first = (int *)calloc((size_t)(n + 1), sizeof(int));
    if (!p || !r || !first) { fprintf(stderr, "Error: out of memory\n"); goto done; }

    if (PROMPTS) printf("Enter the %ld prices p1 ... p%ld (space separated, non-negative): ", n, n);
    p[0] = 0;
    for (i = 1; i <= n; i++) {
        if (scanf("%ld", &p[i]) != 1) {
            fprintf(stderr, "Error: expected %ld prices but could not read price %ld\n", n, i);
            goto done;
        }
        if (p[i] < 0) { fprintf(stderr, "Error: prices must be non-negative\n"); goto done; }
    }

    if (rod_cut(n, p, r, first) != 0) { fprintf(stderr, "Error: out of memory\n"); goto done; }

    /* ----- print the maximum revenue ----- */
    big_to_str(r + n * L, buf);
    if (PROMPTS) printf("\nMaximum revenue = ");
    printf("%s\n", buf);

    /* ----- reconstruction ----- */
    if (PROMPTS) {
        count = (long *)calloc((size_t)(n + 1), sizeof(long));
        printf("Pieces (in cutting order): ");
        if (n == 0) printf("(none)");
        j = n; k = 0;
        while (j > 0) {
            if (k++) printf(" + ");
            printf("%d", first[j]);
            if (count) count[first[j]]++;
            j -= first[j];
        }
        printf("\n");
        cuts = k > 0 ? k - 1 : 0;
        printf("Number of pieces = %ld, number of cuts = %ld\n", k, cuts);
        if (count && n > 0) {
            printf("Summary (length x count, price of one piece):\n");
            for (i = 1; i <= n; i++)
                if (count[i]) printf("  length %ld x %ld   (price %ld each)\n", i, count[i], p[i]);
        }
    } else {
        j = n; k = 0;
        while (j > 0) {
            if (k++) printf(" ");
            printf("%d", first[j]);
            j -= first[j];
        }
        printf("\n");
    }
    status = 0;

done:
    free(p); free(r); free(first); free(count);
    return status;
}