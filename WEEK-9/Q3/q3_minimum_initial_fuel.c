/*
 * Minimum Refuelling Stops -- Reverse (Lazy) Greedy  +  built-in validation harness
 *
 * Problem : Start at 0 with F units of fuel (1 unit fuel = 1 unit distance, unlimited tank).
 *           Station i is at distance d_i and gives f_i fuel. Reach target D with the
 *           minimum number of refuelling stops.
 *
 * INPUT REPRESENTATION
 *   An array of structs  Station { d, f }  (distance from origin, refuel amount).
 *   The algorithm sorts it by d (skipped if already ordered) and keeps the fuel of every
 *   station already passed in a max-heap.
 *
 * ALGORITHM (reverse / lazy greedy)
 *   reach = F, stops = 0
 *   while reach < D:
 *       push fuel of every station with d <= reach into the max-heap   (stations passed)
 *       if heap empty: return -1                                      (stuck)
 *       reach += heap.pop(); stops++        <-- look BACK, refuel at the best passed station
 *   return stops
 *
 * CORRECTNESS (exchange argument)
 *   After k stops the greedy reach is the maximum possible with k stops: it always adds the
 *   largest fuel among the stations reachable at that moment, and any other set of k reachable
 *   stops gives a reach that is no larger. So the first k with reach >= D is the minimum.
 *
 * COMPLEXITY
 *   Sorting O(n log n); every station is pushed once and popped at most once, each heap
 *   operation O(log n)  =>  Time O(n log n) worst case (O(n) when F >= D or total fuel < D),
 *   Space O(n).
 *
 * USAGE
 *   ./a.out            read "F D n" then n lines "d f" from stdin, print the minimum stops
 *                      (or -1 if the target cannot be reached)
 *   ./a.out --test [N] [seed]   validate the greedy against two independent solvers
 *                      (O(n^2) DP and exhaustive subset search) on N random cases (default 10000)
 *   ./a.out --bench    time the algorithm for growing n to illustrate O(n log n)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef long long ll;
typedef unsigned long long ull;
typedef struct { ll d, f; } Station;

/* ====================== max-heap of fuel amounts ====================== */
static ll *heap;
static size_t hsize;

static void heap_push(ll x) {
    size_t c = hsize++;
    while (c > 0) {
        size_t p = (c - 1) / 2;
        if (heap[p] >= x) break;
        heap[c] = heap[p];
        c = p;
    }
    heap[c] = x;
}

static ll heap_pop(void) {
    ll top = heap[0], last = heap[--hsize];
    size_t p = 0;
    for (;;) {
        size_t c = 2 * p + 1;
        if (c >= hsize) break;
        if (c + 1 < hsize && heap[c + 1] > heap[c]) c++;
        if (heap[c] <= last) break;
        heap[p] = heap[c];
        p = c;
    }
    if (hsize > 0) heap[p] = last;
    return top;
}

static int cmp_station(const void *a, const void *b) {
    const Station *x = (const Station *)a, *y = (const Station *)b;
    return (x->d > y->d) - (x->d < y->d);
}

/* ====================== the algorithm under validation ====================== */
/* st[0..n_all) is reordered/compacted in place. Returns minimum stops or -1. */
static int min_refuel_stops(Station *st, size_t n_all, ll F, ll D) {
    size_t n = 0, i;
    int sorted = 1, stops = 0;
    ll tot = F, reach;

    if (F >= D) return 0;                                   /* already enough fuel */
    for (i = 0; i < n_all; i++) {
        ll d = st[i].d, f = st[i].f;
        if (d < 0 || d > D || f <= 0) continue;             /* can never help */
        if (n && st[n - 1].d > d) sorted = 0;
        st[n].d = d; st[n].f = f; n++;
        if (tot < D) tot = (f >= D - tot) ? D : tot + f;    /* saturating sum */
    }
    if (tot < D) return -1;                                 /* even all fuel is not enough */

    if (!sorted) qsort(st, n, sizeof(Station), cmp_station);
    heap = (ll *)malloc((n ? n : 1) * sizeof(ll));
    if (!heap) { fprintf(stderr, "out of memory\n"); exit(1); }

    reach = F;
    i = 0;
    hsize = 0;
    while (reach < D) {
        while (i < n && st[i].d <= reach) heap_push(st[i++].f);   /* stations passed */
        if (hsize == 0) { stops = -1; break; }                    /* stuck */
        {
            ll best = heap_pop();                                 /* look back: max fuel */
            reach = (best >= D - reach) ? D : reach + best;
            stops++;
        }
    }
    free(heap);
    heap = NULL;
    return stops;
}

/* ====================== fast input ====================== */
static char *buf;
static size_t blen, bpos;

static int load_input(void) {
    size_t cap = 1u << 16, r;
    blen = 0;
    buf = (char *)malloc(cap);
    if (!buf) return 0;
    while ((r = fread(buf + blen, 1, cap - blen, stdin)) > 0) {
        blen += r;
        if (blen == cap) {
            char *t;
            cap <<= 1;
            t = (char *)realloc(buf, cap);
            if (!t) return 0;
            buf = t;
        }
    }
    return 1;
}

static int read_ll(ll *out) {
    int neg = 0;
    ll v = 0;
    while (bpos < blen && buf[bpos] != '-' && (buf[bpos] < '0' || buf[bpos] > '9')) bpos++;
    if (bpos >= blen) return 0;
    if (buf[bpos] == '-') { neg = 1; bpos++; }
    if (bpos >= blen || buf[bpos] < '0' || buf[bpos] > '9') return 0;
    while (bpos < blen && buf[bpos] >= '0' && buf[bpos] <= '9') v = v * 10 + (buf[bpos++] - '0');
    *out = neg ? -v : v;
    return 1;
}

static int run_stdin(void) {
    ll F, D, n_in;
    size_t n_all, i;
    Station *st;
    int ans;

    if (!load_input() || !read_ll(&F) || !read_ll(&D) || !read_ll(&n_in) || n_in < 0 ||
        (size_t)n_in > blen) {
        fprintf(stderr, "invalid input\n");
        return 1;
    }
    n_all = (size_t)n_in;
    st = (Station *)malloc((n_all ? n_all : 1) * sizeof(Station));
    if (!st) return 1;
    for (i = 0; i < n_all; i++) {
        if (!read_ll(&st[i].d) || !read_ll(&st[i].f)) {
            fprintf(stderr, "invalid input\n");
            free(st); free(buf);
            return 1;
        }
    }
    free(buf);
    ans = min_refuel_stops(st, n_all, F, D);
    printf("%d\n", ans);
    free(st);
    return 0;
}

/* ====================== independent reference solvers ====================== */
/* O(n^2) DP: dp[j] = farthest distance reachable using exactly j stops. */
static int ref_dp(const Station *in, size_t n, ll F, ll D) {
    Station *s = (Station *)malloc((n ? n : 1) * sizeof(Station));
    ll *dp = (ll *)malloc((n + 1) * sizeof(ll));
    size_t i, j;
    int ans = -1;
    memcpy(s, in, n * sizeof(Station));
    qsort(s, n, sizeof(Station), cmp_station);
    for (j = 0; j <= n; j++) dp[j] = -1;
    dp[0] = F;
    for (i = 0; i < n; i++)
        for (j = i + 1; j-- > 0;)
            if (dp[j] >= 0 && dp[j] >= s[i].d && dp[j] + s[i].f > dp[j + 1])
                dp[j + 1] = dp[j] + s[i].f;
    for (j = 0; j <= n; j++)
        if (dp[j] >= D) { ans = (int)j; break; }
    free(s); free(dp);
    return ans;
}

/* Exhaustive: try every subset of stations (n <= 16), keep the smallest feasible one. */
static int ref_brute(const Station *in, size_t n, ll F, ll D) {
    Station s[16];
    unsigned mask;
    size_t i;
    int best = -1;
    memcpy(s, in, n * sizeof(Station));
    qsort(s, n, sizeof(Station), cmp_station);
    for (mask = 0; mask < (1u << n); mask++) {
        int pc = 0, ok = 1;
        ll reach = F;
        for (i = 0; i < n; i++) if ((mask >> i) & 1u) pc++;
        if (best != -1 && pc >= best) continue;
        for (i = 0; i < n && ok; i++)
            if ((mask >> i) & 1u) {
                if (s[i].d > reach) ok = 0; else reach += s[i].f;
            }
        if (ok && reach >= D) best = pc;
    }
    return best;
}

/* ====================== random test generation ====================== */
static ull rs;
static ull rnd(void) {                       /* xorshift64* */
    rs ^= rs >> 12; rs ^= rs << 25; rs ^= rs >> 27;
    return rs * 2685821657736338717ULL;
}
static ll rr(ll lo, ll hi) {                 /* uniform in [lo, hi] */
    if (hi <= lo) return lo;
    return lo + (ll)(rnd() % (ull)(hi - lo + 1));
}

#define MAXN 300
static size_t gen_case(int kind, ll *Fp, ll *Dp, Station *st) {
    ll F, D;
    size_t n, i;
    switch (kind) {
    case 0:  /* tiny and dense: many ties, zero-fuel stations */
        D = rr(0, 15); F = rr(0, 8); n = (size_t)rr(0, 8);
        for (i = 0; i < n; i++) { st[i].d = rr(0, D + 3); st[i].f = rr(0, 6); }
        break;
    case 1:  /* medium */
        D = rr(1, 100); F = rr(0, D); n = (size_t)rr(0, 12);
        for (i = 0; i < n; i++) { st[i].d = rr(0, D + 5); st[i].f = rr(0, 40); }
        break;
    case 2:  /* more stations */
        D = rr(1, 1000); F = rr(0, D / 5 > 1 ? D / 5 : 1); n = (size_t)rr(0, 60);
        for (i = 0; i < n; i++) { st[i].d = rr(0, D); st[i].f = rr(0, 150); }
        break;
    case 3:  /* huge values (overflow check) */
        D = rr(1000000000000LL, 100000000000000000LL); F = rr(0, D / 4); n = (size_t)rr(0, 40);
        for (i = 0; i < n; i++) { st[i].d = rr(0, D); st[i].f = rr(0, D / 3); }
        break;
    case 4: {  /* chain: every station exactly reachable */
        ll reach, extra[4] = {0, 0, 1, 5};
        n = (size_t)rr(1, 30); F = rr(1, 5); reach = F;
        for (i = 0; i < n; i++) {
            st[i].d = rr(reach - 3 > 0 ? reach - 3 : 0, reach);
            st[i].f = rr(1, 6);
            reach += st[i].f;
        }
        D = rr(F, reach + extra[rnd() % 4]);
        break;
    }
    case 5:  /* F >= D, tiny D, few or no stations */
        D = rr(0, 10); F = D + rr(0, 3); n = (size_t)rr(0, 5);
        for (i = 0; i < n; i++) { st[i].d = rr(0, D + 2); st[i].f = rr(0, 5); }
        break;
    case 6:  /* tight, often unreachable */
        D = rr(20, 200); F = rr(1, 10); n = (size_t)rr(0, 15);
        for (i = 0; i < n; i++) { st[i].d = rr(F, D); st[i].f = rr(1, 15); }
        break;
    default: /* large n, checked against the DP only */
        D = rr(1, 1000000); F = rr(0, D / 50 > 1 ? D / 50 : 1); n = (size_t)rr(0, MAXN);
        for (i = 0; i < n; i++) { st[i].d = rr(0, D); st[i].f = rr(0, 20000); }
        break;
    }
    for (i = n; i > 1; i--) {                /* shuffle input order */
        size_t j = (size_t)(rnd() % i);
        Station t = st[i - 1]; st[i - 1] = st[j]; st[j] = t;
    }
    if (rr(0, 9) < 3) qsort(st, n, sizeof(Station), cmp_station);   /* already-ordered input */
    *Fp = F; *Dp = D;
    return n;
}

typedef struct { const char *name; ll F, D; int n; Station s[6]; int expect; } Fixed;
static const Fixed fixed_cases[] = {
    {"sample from the write-up",      10, 100, 4, {{10, 60}, {20, 30}, {30, 30}, {60, 40}}, 2},
    {"F == D, no stations",           100, 100, 0, {{0, 0}}, 0},
    {"F > D",                         150, 100, 1, {{10, 5}}, 0},
    {"no stations, target too far",   1, 100, 0, {{0, 0}}, -1},
    {"first station out of reach",    50, 100, 1, {{60, 10}}, -1},
    {"station exactly at reach",      50, 100, 1, {{50, 50}}, 1},
    {"must use every station",        10, 100, 3, {{10, 30}, {40, 30}, {70, 30}}, 3},
    {"ties at same distance",         10, 60, 3, {{5, 10}, {5, 20}, {5, 30}}, 2},
    {"station at origin, F = 0",      0, 10, 1, {{0, 10}}, 1},
    {"station beyond target ignored", 20, 50, 2, {{20, 100}, {70, 100}}, 1},
    {"fuel gap: first station unreachable", 10, 50, 2, {{20, 100}, {70, 100}}, -1},
    {"huge values, no overflow",      1, 1000000000000000000LL, 2,
        {{1, 4000000000000000000LL}, {5, 9000000000000000000LL}}, 1},
};

static int run_tests(ll N, ull seed) {
    Station orig[MAXN + 1], work[MAXN + 1];
    ll t, bad = 0, cnt_unreach = 0, cnt_zero = 0, cnt_pos = 0, cnt_brute = 0;
    size_t k;
    int fixed_bad = 0;

    printf("== Fixed edge cases ==\n");
    for (k = 0; k < sizeof(fixed_cases) / sizeof(fixed_cases[0]); k++) {
        const Fixed *c = &fixed_cases[k];
        int got;
        memcpy(work, c->s, (size_t)c->n * sizeof(Station));
        got = min_refuel_stops(work, (size_t)c->n, c->F, c->D);
        printf("  [%s] %-30s expected %2d got %2d\n", got == c->expect ? "ok" : "FAIL",
               c->name, c->expect, got);
        if (got != c->expect) fixed_bad++;
    }

    printf("== Random validation: %lld cases, seed %llu ==\n", N, seed);
    rs = seed ? seed : 88172645463325252ULL;
    for (t = 0; t < N; t++) {
        ll F, D;
        size_t n = gen_case((int)(t % 8), &F, &D, orig);
        int got, dp, br;
        memcpy(work, orig, n * sizeof(Station));
        got = min_refuel_stops(work, n, F, D);
        dp = ref_dp(orig, n, F, D);
        if (n <= 12) { br = ref_brute(orig, n, F, D); cnt_brute++; } else br = dp;
        if (got != dp || got != br) {
            if (++bad <= 5) {
                size_t i;
                printf("MISMATCH case %lld: F=%lld D=%lld n=%zu greedy=%d dp=%d brute=%d\n",
                       t, F, D, n, got, dp, br);
                for (i = 0; i < n && i < 12; i++) printf("   (%lld,%lld)\n", orig[i].d, orig[i].f);
            }
        }
        if (dp == -1) cnt_unreach++; else if (dp == 0) cnt_zero++; else cnt_pos++;
    }
    printf("  answer -1: %lld   answer 0: %lld   answer >0: %lld   (exhaustive-checked: %lld)\n",
           cnt_unreach, cnt_zero, cnt_pos, cnt_brute);
    printf("  greedy vs DP vs exhaustive: %lld / %lld agree\n", N - bad, N);
    if (bad || fixed_bad) { printf("RESULT: FAILED\n"); return 1; }
    printf("RESULT: ALL TESTS PASSED\n");
    return 0;
}

static int run_bench(void) {
    size_t sizes[] = {100000, 200000, 400000, 800000, 1600000};
    size_t k;
    printf("%-10s %-10s %-10s %-22s\n", "n", "stops", "time(s)", "ns per (n*log2 n)");
    rs = 123456789ULL;
    for (k = 0; k < sizeof(sizes) / sizeof(sizes[0]); k++) {
        size_t n = sizes[k], i, m;
        double lg = 0, secs;
        ll D;
        clock_t t0, t1;
        int ans;
        Station *st = (Station *)malloc(n * sizeof(Station));
        if (!st) return 1;
        D = (ll)n * 1000;                    /* target scales with n so every size is reachable */
        for (i = 0; i < n; i++) { st[i].d = i ? rr(0, D) : 0; st[i].f = rr(1, 5000); }
        for (m = n; m > 1; m >>= 1) lg += 1;
        t0 = clock();
        ans = min_refuel_stops(st, n, 1000, D);
        t1 = clock();
        secs = (double)(t1 - t0) / CLOCKS_PER_SEC;
        printf("%-10zu %-10d %-10.3f %-22.2f\n", n, ans, secs, secs * 1e9 / ((double)n * lg));
        free(st);
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--test") == 0)
        return run_tests(argc > 2 ? atoll(argv[2]) : 10000, argc > 3 ? strtoull(argv[3], NULL, 10) : 20261006ULL);
    if (argc > 1 && strcmp(argv[1], "--bench") == 0) return run_bench();
    return run_stdin();
}