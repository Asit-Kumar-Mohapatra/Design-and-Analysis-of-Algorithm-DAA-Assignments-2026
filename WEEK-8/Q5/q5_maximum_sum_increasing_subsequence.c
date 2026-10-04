/*
 * Maximum Sum Increasing Subsequence (strictly increasing, positive integers)
 *
 * Input representation: coordinate-compress values to ranks 1..m, then keep a
 * Fenwick tree (BIT) over ranks storing the best subsequence sum ending at a
 * value of that rank (prefix-max BIT).
 *
 *   dp[i] = a[i] + max{ dp[j] : j < i, a[j] < a[i] }  (0 if none)
 *   answer = max dp[i]
 *
 * Why a prefix-MAX Fenwick tree is valid: entries are only ever raised
 * (bit[k] = max(bit[k], cur)), never lowered, so every node always holds the
 * maximum over the range it covers.
 *
 * Time : O(n log n)   (sort for compression + n BIT queries/updates)
 * Space: O(n)
 *
 * Input  : first the number of elements n, then the n positive integers
 *          (separated by spaces and/or newlines)
 * Output : one integer - the maximum sum of a strictly increasing subsequence
 *          (errors go to stderr, exit code 1)
 *
 * Build  : gcc -std=c99 -O2 -Wall -Wextra -pedantic -o msis msis.c
 * Usage  : ./msis            reads stdin (prompts only when typing in a terminal)
 *          ./msis --test     validates against two independent brute forces
 * Prompts: force with -DPROMPTS=1 (always) or -DPROMPTS=0 (never)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#ifdef PROMPTS
#define SHOW_PROMPTS() (PROMPTS)
#else
#ifdef _WIN32
#include <io.h>
#define SHOW_PROMPTS() _isatty(_fileno(stdin))
#else
#include <unistd.h>
#define SHOW_PROMPTS() isatty(0)
#endif
#endif

typedef long long ll;

/* status codes returned by max_sum_increasing */
enum { MS_OK = 0, MS_NOMEM = 1, MS_OVERFLOW = 2, MS_BADVALUE = 3 };

static int cmp_ll(const void *x, const void *y)
{
    ll a = *(const ll *)x, b = *(const ll *)y;
    return (a > b) - (a < b);
}

/* rank of v in sorted unique array u[0..m-1], returned 1-based */
static int rank_of(const ll *u, int m, ll v)
{
    int lo = 0, hi = m - 1;
    while (lo < hi) {
        int mid = lo + ((hi - lo) >> 1);
        if (u[mid] < v) lo = mid + 1; else hi = mid;
    }
    return lo + 1;
}

/* Computes the maximum sum of a strictly increasing subsequence into *out.
   Returns MS_OK, or an error code (then *out is not meaningful).           */
static int max_sum_increasing(const ll *a, int n, ll *out)
{
    ll *u, *bit, best = 0;
    int m = 0, i, k;

    *out = 0;
    if (n <= 0) return MS_OK;

    for (i = 0; i < n; i++)                          /* problem guarantees positives */
        if (a[i] <= 0) return MS_BADVALUE;

    if ((size_t)n > SIZE_MAX / sizeof(ll) - 1) return MS_NOMEM;
    u   = (ll *)malloc((size_t)n * sizeof(ll));
    bit = (ll *)calloc((size_t)n + 1, sizeof(ll));
    if (!u || !bit) { free(u); free(bit); return MS_NOMEM; }

    memcpy(u, a, (size_t)n * sizeof(ll));
    qsort(u, (size_t)n, sizeof(ll), cmp_ll);
    for (i = 0; i < n; i++)
        if (i == 0 || u[i] != u[i - 1]) u[m++] = u[i];

    for (i = 0; i < n; i++) {
        int r = rank_of(u, m, a[i]);
        ll q = 0, cur;                               /* q = max over ranks < r */
        for (k = r - 1; k > 0; k -= k & -k)
            if (bit[k] > q) q = bit[k];
        if (q > LLONG_MAX - a[i]) { free(u); free(bit); return MS_OVERFLOW; }
        cur = q + a[i];
        if (cur > best) best = cur;
        for (k = r; k <= m; k += k & -k)
            if (bit[k] < cur) bit[k] = cur;
    }
    free(u);
    free(bit);
    *out = best;
    return MS_OK;
}

static void report_error(int code)
{
    switch (code) {
        case MS_NOMEM:     fprintf(stderr, "Error: out of memory\n"); break;
        case MS_OVERFLOW:  fprintf(stderr, "Error: sum exceeds the range of a 64-bit integer\n"); break;
        case MS_BADVALUE:  fprintf(stderr, "Error: all elements must be positive integers\n"); break;
        default:           fprintf(stderr, "Error\n"); break;
    }
}

/* ------------------------------ validation ------------------------------ */

/* Reference 1: classic O(n^2) DP */
static ll brute(const ll *a, int n)
{
    ll *dp, best = 0;
    int i, j;
    if (n <= 0) return 0;
    dp = (ll *)malloc((size_t)n * sizeof(ll));
    if (!dp) { fprintf(stderr, "test: out of memory\n"); exit(2); }
    for (i = 0; i < n; i++) {
        dp[i] = a[i];
        for (j = 0; j < i; j++)
            if (a[j] < a[i] && dp[j] + a[i] > dp[i]) dp[i] = dp[j] + a[i];
        if (dp[i] > best) best = dp[i];
    }
    free(dp);
    return best;
}

/* Reference 2: exhaustive over all 2^n subsets (n <= ~16) */
static ll subset_ref(const ll *a, int n)
{
    ll best = 0;
    unsigned mask;
    for (mask = 0; mask < (1u << n); mask++) {
        ll sum = 0, last = 0;
        int ok = 1, have = 0, i;
        for (i = 0; i < n && ok; i++)
            if ((mask >> i) & 1u) {
                if (have && a[i] <= last) ok = 0;
                last = a[i]; have = 1; sum += a[i];
            }
        if (ok && sum > best) best = sum;
    }
    return best;
}

static long tests = 0, fails = 0;

static void check(ll got, ll exp, const char *tag)
{
    tests++;
    if (got != exp) {
        fails++;
        if (fails < 10)
            printf("FAIL [%s] got %lld expected %lld\n", tag, got, exp);
    }
}

static ll solve(const ll *a, int n)
{
    ll r;
    int st = max_sum_increasing(a, n, &r);
    if (st != MS_OK) { printf("unexpected status %d\n", st); exit(2); }
    return r;
}

static int run_tests(void)
{
    int t, i, n, code, x;
    srand(12345);

    /* 1. hand-made cases with known answers */
    {
        ll ex1[] = {1, 101, 2, 3, 100, 4, 5};
        ll ex2[] = {3, 2, 1};
        ll ex3[] = {5, 5, 5};
        ll ex4[] = {10, 5, 4, 3};
        ll ex5[] = {1, 2, 3, 4, 5};
        ll ex6[] = {7};
        ll ex7[] = {3, 4, 5, 10};
        ll ex8[] = {10, 20, 10, 20, 30};
        check(solve(ex1, 7), 106, "ex1");
        check(solve(ex2, 3), 3, "ex2");
        check(solve(ex3, 3), 5, "ex3");
        check(solve(ex4, 4), 10, "ex4");
        check(solve(ex5, 5), 15, "ex5");
        check(solve(ex6, 1), 7, "ex6");
        check(solve(ex7, 4), 22, "ex7");
        check(solve(ex8, 5), 60, "ex8");
        check(solve(ex6, 0), 0, "n=0");
    }

    /* 2. error handling: non-positive values and overflow */
    {
        ll bad1[] = {1, 0, 3}, bad2[] = {4, -2}, out;
        ll ov[] = {LLONG_MAX - 1, LLONG_MAX}, mx[] = {LLONG_MAX};
        check(max_sum_increasing(bad1, 3, &out), MS_BADVALUE, "zero");
        check(max_sum_increasing(bad2, 2, &out), MS_BADVALUE, "negative");
        check(max_sum_increasing(ov, 2, &out), MS_OVERFLOW, "overflow");
        check(max_sum_increasing(mx, 1, &out), MS_OK, "max ok status");
        check(out, LLONG_MAX, "max ok value");
    }

    /* 3. 20000 small random tests vs both brute forces (many duplicates) */
    for (t = 0; t < 20000; t++) {
        ll a[40], g;
        int range = rand() % 30 + 1;
        n = rand() % 40 + 1;
        for (i = 0; i < n; i++) a[i] = rand() % range + 1;
        g = solve(a, n);
        check(g, brute(a, n), "dp");
        if (n <= 14) check(g, subset_ref(a, n), "subset");
    }

    /* 4. wide-range random tests (values up to ~1e9) vs O(n^2) DP */
    for (t = 0; t < 300; t++) {
        ll a[400];
        n = rand() % 400 + 1;
        for (i = 0; i < n; i++)
            a[i] = (ll)(((long)rand() * 32768L + rand()) % 1000000000L) + 1;
        check(solve(a, n), brute(a, n), "wide");
    }

    /* 5. exhaustive: all arrays of length <= 6 over {1..5}, length 7 over {1..4} */
    for (n = 0; n <= 7; n++) {
        int base = (n == 7) ? 4 : 5, total = 1;
        ll a[7];
        for (i = 0; i < n; i++) total *= base;
        for (code = 0; code < total; code++) {
            x = code;
            for (i = 0; i < n; i++) { a[i] = x % base + 1; x /= base; }
            check(solve(a, n), subset_ref(a, n), "exhaustive");
        }
    }

    /* 6. large closed-form cases: n = 1,000,000 */
    {
        int N = 1000000;
        ll *a = (ll *)malloc((size_t)N * sizeof(ll));
        if (!a) { fprintf(stderr, "test: out of memory\n"); return 2; }
        for (i = 0; i < N; i++) a[i] = i + 1;
        check(solve(a, N), (ll)N * (N + 1) / 2, "big-inc");
        for (i = 0; i < N; i++) a[i] = N - i;
        check(solve(a, N), N, "big-dec");
        for (i = 0; i < N; i++) a[i] = 42;
        check(solve(a, N), 42, "big-equal");
        free(a);
    }

    printf("Total checks: %ld | Failures: %ld\n", tests, fails);
    puts(fails == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED");
    return fails != 0;
}

/* --------------------------------- main --------------------------------- */

int main(int argc, char **argv)
{
    int prompts, n, i, st;
    ll *a, ans;

    if (argc > 1 && strcmp(argv[1], "--test") == 0) return run_tests();

    prompts = SHOW_PROMPTS();
    if (prompts) printf("Enter n (number of elements): ");
    if (scanf("%d", &n) != 1 || n < 0) {
        fprintf(stderr, "Error: invalid n\n");
        return 1;
    }
    if ((size_t)n > SIZE_MAX / sizeof(ll)) {
        fprintf(stderr, "Error: n too large\n");
        return 1;
    }
    a = (ll *)malloc((size_t)(n ? n : 1) * sizeof(ll));
    if (!a) { fprintf(stderr, "Error: out of memory\n"); return 1; }

    if (prompts) printf("Enter %d positive integers (space or newline separated): ", n);
    for (i = 0; i < n; i++)
        if (scanf("%lld", &a[i]) != 1) {
            fprintf(stderr, "Error: expected %d integers but could not read element %d\n", n, i + 1);
            free(a);
            return 1;
        }

    st = max_sum_increasing(a, n, &ans);
    free(a);
    if (st != MS_OK) { report_error(st); return 1; }

    if (prompts) printf("Maximum sum of strictly increasing subsequence = %lld\n", ans);
    else         printf("%lld\n", ans);
    return 0;
}