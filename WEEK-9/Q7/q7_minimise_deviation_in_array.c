/*
 * min_deviation.c  --  Minimise Deviation in Array (two-way greedy + max-heap)
 *
 * Operations: multiply an odd element by 2, or divide an even element by 2.
 * Deviation = max(A) - min(A).  Minimise it after any number of operations.
 *
 * Build : gcc -O2 -std=c11 -Wall -Wextra -o min_deviation min_deviation.c
 *
 * Usage :
 *   min_deviation                 read stdin, print the minimum deviation
 *   min_deviation -v              also run the built-in validator (stderr)
 *   min_deviation --stress N [S]  self-test on N random cases (seed S)
 *
 * INPUT    n
 *          a[0] a[1] ... a[n-1]     positive integers, each <= 4e18
 * OUTPUT   one integer: the minimum possible deviation
 *
 * KEY IDEA (why "two-way" collapses to one direction)
 *   An odd x can only become 2x (and back), an even x can only go down to its
 *   odd part.  So every element's reachable values are  o, 2o, 4o, ..., up to
 *   max(x, 2o)  where o = odd part of x.  Doubling every odd element up front
 *   puts all elements at the TOP of their chains; from there the only useful
 *   move is "halve the current maximum":
 *       - the maximum is the only element that can lower the deviation,
 *       - once the maximum is odd it cannot be reduced -> stop.
 *   A max-heap always exposes the maximum; we track the minimum in a scalar
 *   (it only ever decreases) and record the best (max - min) after every step.
 *
 * ALGORITHM
 *   1. odd -> *2, remember min               O(n)
 *   2. build max-heap in place               O(n)    (bottom-up heapify)
 *   3. best = max - min
 *      while best > 0 and top is even:
 *          top = top/2 ; min = min(min, top) ; sift-down ; best = min(best, max-min)
 *   Every element halves at most log2(M) times -> O(n log M) steps, each O(log n).
 *   Time  O(n + n log M log n)  =  O(n log n log M)      Space  O(1) extra
 *   (the heap lives inside the input array).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_VAL (INT64_MAX / 2) /* so that 2*x never overflows */

/* ------------------------------------------------------------------ */
/*  Core algorithm                                                    */
/* ------------------------------------------------------------------ */
static void sift_down(int64_t *h, uint32_t sz, uint32_t i)
{
    int64_t x = h[i];
    for (;;) {
        uint32_t c = 2 * i + 1;
        if (c >= sz) break;
        if (c + 1 < sz && h[c + 1] > h[c]) c++;
        if (h[c] <= x) break;
        h[i] = h[c];
        i = c;
    }
    h[i] = x;
}

/* h[0..n-1] holds the input values (all in 1..MAX_VAL); it is destroyed.   */
/* *steps receives the number of heap "halve the max" steps (for analysis). */
static int64_t min_deviation(uint32_t n, int64_t *h, uint64_t *steps)
{
    uint64_t st = 0;
    if (n < 2) { *steps = 0; return 0; }

    int64_t mn = INT64_MAX;
    for (uint32_t i = 0; i < n; i++) {
        if (h[i] & 1) h[i] *= 2;          /* move to the top of its chain */
        if (h[i] < mn) mn = h[i];
    }
    for (uint32_t i = n / 2; i-- > 0;) sift_down(h, n, i);

    int64_t best = h[0] - mn;
    while (best > 0 && !(h[0] & 1)) {
        int64_t half = h[0] >> 1;
        h[0] = half;
        if (half < mn) mn = half;
        sift_down(h, n, 0);
        int64_t d = h[0] - mn;
        if (d < best) best = d;
        st++;
    }
    *steps = st;
    return best;
}

/* ------------------------------------------------------------------ */
/*  Independent oracles (used only for validation)                    */
/* ------------------------------------------------------------------ */

/* every value reachable from x by really applying the two operations */
static uint32_t reach(int64_t x, int64_t *out)
{
    uint32_t k = 0;
    out[k++] = x;
    for (uint32_t q = 0; q < k; q++) {
        int64_t v = out[q];
        int64_t nx = (v & 1) ? v * 2 : v / 2;
        int seen = 0;
        for (uint32_t j = 0; j < k; j++) if (out[j] == nx) { seen = 1; break; }
        if (!seen) out[k++] = nx;
    }
    return k;
}

typedef struct { int64_t v; uint32_t id; } Cand;
static int cand_cmp(const void *a, const void *b)
{
    const Cand *x = a, *y = b;
    if (x->v != y->v) return x->v < y->v ? -1 : 1;
    return (x->id > y->id) - (x->id < y->id);
}

/* oracle 1: smallest window over all candidate values that touches every element */
static int64_t window_oracle(uint32_t n, const int64_t *a)
{
    if (n < 2) return 0;
    int64_t tmp[80];
    size_t m = 0, cap = (size_t)n * 66;
    Cand *c = malloc(cap * sizeof *c);
    uint32_t *cnt = calloc(n, sizeof *cnt);
    if (!c || !cnt) { free(c); free(cnt); return -1; }
    for (uint32_t i = 0; i < n; i++) {
        uint32_t k = reach(a[i], tmp);
        for (uint32_t j = 0; j < k; j++) { c[m].v = tmp[j]; c[m].id = i; m++; }
    }
    qsort(c, m, sizeof *c, cand_cmp);
    int64_t best = INT64_MAX;
    uint32_t covered = 0;
    size_t l = 0;
    for (size_t r = 0; r < m; r++) {
        if (cnt[c[r].id]++ == 0) covered++;
        while (covered == n) {
            if (c[r].v - c[l].v < best) best = c[r].v - c[l].v;
            if (--cnt[c[l].id] == 0) covered--;
            l++;
        }
    }
    free(c); free(cnt);
    return best;
}

/* oracle 2: try every combination of reachable values (tiny inputs only) */
static int64_t bf_best;
static void bf_rec(uint32_t i, uint32_t n, int64_t (*R)[80], const uint32_t *K, int64_t lo, int64_t hi)
{
    if (i == n) { if (hi - lo < bf_best) bf_best = hi - lo; return; }
    for (uint32_t j = 0; j < K[i]; j++) {
        int64_t v = R[i][j];
        bf_rec(i + 1, n, R, K, v < lo ? v : lo, v > hi ? v : hi);
    }
}

/* returns NULL when the heap result agrees with both oracles */
static const char *validate(uint32_t n, const int64_t *a, int64_t result)
{
    if (n == 0) return result == 0 ? NULL : "n=0 must give 0";
    if (result < 0) return "negative deviation";
    int64_t w = window_oracle(n, a);
    if (w < 0) return "validator out of memory";
    if (w != result) return "differs from candidate-window oracle";

    if (n <= 5) {
        int64_t R[5][80];
        uint32_t K[5];
        double combos = 1;
        for (uint32_t i = 0; i < n; i++) { K[i] = reach(a[i], R[i]); combos *= K[i]; }
        if (combos <= 300000) {
            bf_best = INT64_MAX;
            bf_rec(0, n, R, K, INT64_MAX, INT64_MIN);
            if (bf_best != result) return "differs from brute force";
        }
    }
    return NULL;
}

/* ------------------------------------------------------------------ */
/*  Random stress test                                                */
/* ------------------------------------------------------------------ */
static uint64_t rs = 88172645463325252ULL;
static uint64_t rnd(void) { rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; return rs; }
static uint64_t rr(uint64_t lo, uint64_t hi) { return lo + rnd() % (hi - lo + 1); }

#define STRESS_CAP 1500
static const char *type_name[10] = {
    "tiny 1..10", "small 1..1000", "up to 1e9", "all equal", "all odd",
    "all even", "odd*2^k", "huge 4e18", "sorted", "powers of two"
};

static uint32_t gen(int64_t *a, int *type_out)
{
    uint32_t n;
    uint64_t r = rr(0, 99);
    if (rr(0, 199) == 0) n = 0;
    else if (r < 40) n = (uint32_t)rr(1, 6);
    else if (r < 70) n = (uint32_t)rr(7, 30);
    else if (r < 92) n = (uint32_t)rr(31, 200);
    else n = (uint32_t)rr(201, STRESS_CAP);
    int type = (int)rr(0, 9);
    *type_out = type;
    uint64_t eq = rr(1, 1000000);
    for (uint32_t i = 0; i < n; i++) {
        uint64_t v;
        switch (type) {
        case 0: v = rr(1, 10); break;
        case 1: v = rr(1, 1000); break;
        case 2: v = rr(1, 1000000000ULL); break;
        case 3: v = eq; break;
        case 4: v = 2 * rr(0, 500000000ULL) + 1; break;
        case 5: v = 2 * rr(1, 500000000ULL); break;
        case 6: { uint64_t o = 2 * rr(0, 1000000) + 1; v = o << rr(0, 40); } break;
        case 7: v = rr(1, 4000000000000000000ULL); break;
        case 8: v = i + 1; break;
        default: v = 1ULL << rr(0, 61); break;
        }
        if (v < 1) v = 1;
        if (v > (uint64_t)MAX_VAL) v = (uint64_t)MAX_VAL;
        a[i] = (int64_t)v;
    }
    return n;
}

static int stress(uint64_t N, uint64_t seed)
{
    if (seed) rs = seed;
    int64_t *a = malloc(STRESS_CAP * sizeof *a);
    int64_t *h = malloc(STRESS_CAP * sizeof *h);
    if (!a || !h) return 2;
    uint64_t pass = 0, fail = 0, tc[10] = {0}, tiny = 0, big = 0, zero = 0, maxsteps = 0;
    uint32_t maxn = 0;
    for (uint64_t t = 1; t <= N; t++) {
        int type;
        uint32_t n = gen(a, &type);
        memcpy(h, a, (size_t)n * sizeof *a);
        uint64_t steps;
        int64_t res = min_deviation(n, h, &steps);
        const char *err = validate(n, a, res);
        tc[type]++;
        if (n <= 5) tiny++;
        if (n >= 200) big++;
        if (res == 0) zero++;
        if (n > maxn) maxn = n;
        if (steps > maxsteps) maxsteps = steps;
        if (err) {
            fail++;
            if (fail <= 5) {
                printf("FAIL #%llu type=%s n=%u result=%lld : %s\n  input:", (unsigned long long)t,
                       type_name[type], n, (long long)res, err);
                for (uint32_t i = 0; i < n && i < 20; i++) printf(" %lld", (long long)a[i]);
                printf("\n");
            }
        } else pass++;
    }
    printf("Stress result: %llu / %llu passed, %llu failed\n", (unsigned long long)pass,
           (unsigned long long)N, (unsigned long long)fail);
    printf("  n<=5 (brute-force checked): %llu | n>=200: %llu | answer 0: %llu | largest n=%u | most heap steps=%llu\n",
           (unsigned long long)tiny, (unsigned long long)big, (unsigned long long)zero, maxn,
           (unsigned long long)maxsteps);
    printf("  types:");
    for (int i = 0; i < 10; i++) printf(" [%s]=%llu", type_name[i], (unsigned long long)tc[i]);
    printf("\n");
    free(a); free(h);
    return fail ? 1 : 0;
}

/* ------------------------------------------------------------------ */
/*  Input                                                             */
/* ------------------------------------------------------------------ */
static int rd_char(void)
{
    static unsigned char buf[1 << 16];
    static size_t len = 0, pos = 0;
    if (pos == len) {
        len = fread(buf, 1, sizeof buf, stdin);
        pos = 0;
        if (len == 0) return -1;
    }
    return buf[pos++];
}

static int rd_u64(uint64_t *out)
{
    int c;
    do c = rd_char(); while (c == ' ' || c == '\n' || c == '\r' || c == '\t');
    if (c < '0' || c > '9') return 0;
    uint64_t v = 0;
    while (c >= '0' && c <= '9') {
        if (v > (UINT64_MAX - (uint64_t)(c - '0')) / 10) return 0;
        v = v * 10 + (uint64_t)(c - '0');
        c = rd_char();
    }
    *out = v;
    return 1;
}

int main(int argc, char **argv)
{
    if (argc >= 3 && strcmp(argv[1], "--stress") == 0)
        return stress(strtoull(argv[2], NULL, 10), argc >= 4 ? strtoull(argv[3], NULL, 10) : 0);
    int verify = (argc >= 2 && strcmp(argv[1], "-v") == 0);

    uint64_t n64;
    if (!rd_u64(&n64) || n64 >= (1ULL << 31)) { fprintf(stderr, "error: bad n\n"); return 2; }
    uint32_t n = (uint32_t)n64;

    int64_t *h = malloc((n ? n : 1) * sizeof *h);
    int64_t *orig = verify ? malloc((n ? n : 1) * sizeof *orig) : NULL;
    if (!h || (verify && !orig)) { fprintf(stderr, "error: out of memory\n"); return 2; }

    for (uint32_t i = 0; i < n; i++) {
        uint64_t v;
        if (!rd_u64(&v)) { fprintf(stderr, "error: expected %u integers\n", n); return 2; }
        if (v < 1 || v > (uint64_t)MAX_VAL) { fprintf(stderr, "error: values must be in 1..4e18\n"); return 2; }
        h[i] = (int64_t)v;
    }
    if (verify) memcpy(orig, h, (size_t)n * sizeof *h);

    uint64_t steps;
    int64_t res = min_deviation(n, h, &steps);
    printf("%lld\n", (long long)res);

    if (verify) {
        const char *err = validate(n, orig, res);
        if (err) { fprintf(stderr, "[VALIDATION] FAIL: %s\n", err); return 1; }
        fprintf(stderr, "[VALIDATION] PASS (heap steps: %llu)\n", (unsigned long long)steps);
    }
    free(h); free(orig);
    return 0;
}