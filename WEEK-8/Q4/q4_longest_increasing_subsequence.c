/*
 * Longest Strictly Increasing Subsequence  --  patience sorting / tails array
 *
 * tails[k] = smallest possible last element of a strictly increasing
 *            subsequence of length k+1 seen so far.  tails is always strictly
 *            increasing, so each new element x is placed by binary search:
 *            find first index with tails[idx] >= x and overwrite it (or append).
 *            ">= x" (not "> x") is what makes the subsequence STRICT:
 *            an equal element replaces, it never extends.
 *
 * Input representation : integer array A[0..n-1] plus its length n.
 *
 * Time  : O(n log n)     (n elements x one O(log L) binary search each, L <= n)
 * Space : O(n)           (tails array; O(L) really used, L = answer)
 *
 * Build : gcc -std=c99 -O2 -Wall -Wextra -pedantic -o lis lis.c
 * Judge : add -DPROMPTS=0 to print only the answers (no prompt text)
 * Test  : gcc -std=c99 -O2 -DTEST -o lis_test lis.c && ./lis_test
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* 1 = friendly prompts (typing by hand), 0 = silent, only answers (judges/files) */
#ifndef PROMPTS
#define PROMPTS 1
#endif

/* Returns length of the longest strictly increasing subsequence,
   or -1 on allocation failure / impossible size. */
long long lisLength(const int *a, size_t n)
{
    int *tails;
    size_t len = 0, i;

    if (n == 0) return 0;
    if (n > SIZE_MAX / sizeof(int)) return -1;        /* size overflow guard */
    tails = (int *)malloc(n * sizeof(int));
    if (!tails) return -1;

    for (i = 0; i < n; i++) {
        int x = a[i];
        size_t lo = 0, hi = len;              /* first position with tails[pos] >= x */
        while (lo < hi) {
            size_t mid = lo + (hi - lo) / 2;
            if (tails[mid] < x) lo = mid + 1;
            else                hi = mid;
        }
        tails[lo] = x;
        if (lo == len) len++;
    }
    free(tails);
    return (long long)len;
}

#ifdef TEST
#include <limits.h>

/* Reference 1: classic O(n^2) DP */
static long long dpRef(const int *a, int n)
{
    int *d, best = 0, i, j;
    if (n == 0) return 0;
    d = (int *)malloc((size_t)n * sizeof(int));
    if (!d) { printf("test: out of memory\n"); exit(2); }
    for (i = 0; i < n; i++) {
        d[i] = 1;
        for (j = 0; j < i; j++)
            if (a[j] < a[i] && d[j] + 1 > d[i]) d[i] = d[j] + 1;
        if (d[i] > best) best = d[i];
    }
    free(d);
    return best;
}

/* Reference 2: exhaustive over all 2^n subsets (n <= ~16) */
static long long subsetRef(const int *a, int n)
{
    int best = 0;
    unsigned m;
    for (m = 0; m < (1u << n); m++) {
        int cnt = 0, ok = 1, last = 0, have = 0, i;
        for (i = 0; i < n && ok; i++)
            if ((m >> i) & 1u) {
                if (have && a[i] <= last) ok = 0;
                last = a[i]; have = 1; cnt++;
            }
        if (ok && cnt > best) best = cnt;
    }
    return best;
}

static long long tests = 0, fails = 0;
static void check(long long got, long long exp, const char *tag)
{
    tests++;
    if (got != exp) {
        fails++;
        if (fails < 10) printf("FAIL [%s] got %lld expected %lld\n", tag, got, exp);
    }
}

int main(void)
{
    int t, n, i, k, code, x;
    srand(777);

    /* 1. hand-made edge cases */
    {
        struct { int a[10]; int n; int exp; } E[] = {
            {{10,9,2,5,3,7,101,18},8,4}, {{0,1,0,3,2,3},6,4}, {{7,7,7,7},4,1},
            {{5},1,1}, {{0},0,0}, {{1,2,3,4,5},5,5}, {{5,4,3,2,1},5,1},
            {{INT_MIN,INT_MAX},2,2}, {{INT_MAX,INT_MIN},2,1}, {{-3,-2,-2,-1},4,3},
            {{1,3,2,3,4,1},6,4}, {{2,2,3,3,4,4},6,3}
        };
        for (k = 0; k < (int)(sizeof E / sizeof E[0]); k++)
            check(lisLength(E[k].a, (size_t)E[k].n), E[k].exp, "edge");
    }

    /* 2. 10000 random tests vs O(n^2) DP (+ subset brute force for n<=14) */
    for (t = 0; t < 10000; t++) {
        int a[40], mode = rand() % 7;
        long long g;
        n = rand() % 40;
        for (i = 0; i < n; i++) {
            switch (mode) {
                case 0: a[i] = rand() % 3; break;                      /* many duplicates */
                case 1: a[i] = rand() % 10 - 5; break;                 /* negatives */
                case 2: a[i] = rand() - RAND_MAX / 2; break;           /* wide */
                case 3: a[i] = i; break;                               /* increasing */
                case 4: a[i] = -i; break;                              /* decreasing */
                case 5: a[i] = (rand() & 1) ? INT_MAX - rand() % 3
                                            : INT_MIN + rand() % 3; break; /* extremes */
                default: a[i] = (i % 5) * 10 + rand() % 4; break;      /* saw-tooth */
            }
        }
        g = lisLength(a, (size_t)n);
        check(g, dpRef(a, n), "dp");
        if (n <= 14) check(g, subsetRef(a, n), "subset");
    }

    /* 3. large random arrays vs O(n^2) DP */
    for (t = 0; t < 30; t++) {
        int range = (t % 3 == 0) ? 50 : (t % 3 == 1) ? 5000 : 1000000000;
        int *a;
        n = 20000;
        a = (int *)malloc((size_t)n * sizeof(int));
        if (!a) { printf("test: out of memory\n"); return 2; }
        for (i = 0; i < n; i++) a[i] = (int)(((long)rand() * 32768L + rand()) % range);
        check(lisLength(a, (size_t)n), dpRef(a, n), "large");
        free(a);
    }

    /* 4. exhaustive: all arrays of length<=7 over {0..4}, length 8 over {0..3} */
    for (n = 0; n <= 8; n++) {
        int base = (n == 8) ? 4 : 5, total = 1, a[8];
        for (i = 0; i < n; i++) total *= base;
        for (code = 0; code < total; code++) {
            x = code;
            for (i = 0; i < n; i++) { a[i] = x % base; x /= base; }
            check(lisLength(a, (size_t)n), subsetRef(a, n), "exhaustive");
        }
    }

    /* 5. big performance / closed form: n = 2,000,000 */
    {
        size_t N = 2000000, j;
        int *a = (int *)malloc(N * sizeof(int));
        if (!a) { printf("test: out of memory\n"); return 2; }
        for (j = 0; j < N; j++) a[j] = (int)j;
        check(lisLength(a, N), (long long)N, "big-inc");
        for (j = 0; j < N; j++) a[j] = (int)(N - j);
        check(lisLength(a, N), 1, "big-dec");
        free(a);
    }

    printf("Total tests: %lld | Failures: %lld\n", tests, fails);
    return fails != 0;
}

#else
/*
 * INPUT FORMAT (whitespace-separated; spaces or new lines both work):
 *   T                  number of test cases
 *   then, for each case:
 *   n                  length of the array
 *   a0 a1 ... a(n-1)   the n integers
 * OUTPUT: one line per case -> length of the longest strictly increasing subsequence.
 *
 * Example input:        Output:
 *   2                     4
 *   8                     1
 *   10 9 2 5 3 7 101 18
 *   4
 *   7 7 7 7
 */
int main(void)
{
    int T, tc;

    if (PROMPTS) printf("Number of test cases T: ");
    if (scanf("%d", &T) != 1 || T < 0) { fprintf(stderr, "invalid T\n"); return 1; }

    for (tc = 1; tc <= T; tc++) {
        long long n, i, r;
        int *a;

        if (PROMPTS) printf("\n[Case %d] Array length n: ", tc);
        if (scanf("%lld", &n) != 1 || n < 0) { fprintf(stderr, "invalid n\n"); return 1; }
        if ((unsigned long long)n > SIZE_MAX / sizeof(int)) {
            fprintf(stderr, "n too large\n"); return 1;
        }

        a = (int *)malloc((n ? (size_t)n : 1) * sizeof(int));
        if (!a) { fprintf(stderr, "out of memory\n"); return 1; }
        if (PROMPTS) printf("[Case %d] Enter %lld integers: ", tc, n);
        for (i = 0; i < n; i++)
            if (scanf("%d", &a[i]) != 1) {
                fprintf(stderr, "invalid element\n"); free(a); return 1;
            }

        r = lisLength(a, (size_t)n);
        if (r < 0) {
            fprintf(stderr, "out of memory\n");
            free(a);
            return 1;
        }
        if (PROMPTS) printf("[Case %d] Answer: %lld\n", tc, r);
        else         printf("%lld\n", r);
        free(a);
    }
    return 0;
}
#endif