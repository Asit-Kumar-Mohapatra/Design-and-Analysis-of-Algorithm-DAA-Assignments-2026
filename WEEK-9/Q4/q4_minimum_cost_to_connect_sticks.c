/*
 * Minimum Cost to Connect Sticks
 * ------------------------------------------------------------------
 * Representation : sorted array + queue of merged sticks ("two-queue" Huffman)
 * Algorithm      : greedy - always merge the two currently smallest sticks.
 * Sorting        : LSD radix sort (11-bit digits) -> O(n * ceil(w/11)) = O(n)
 *                  for bounded integer lengths (insertion sort for tiny n).
 * Merging        : merged sums are produced in non-decreasing order, so a plain
 *                  FIFO queue replaces the heap -> O(n).
 * Time  : O(n)  (O(n log n) if lengths are unbounded / comparison model)
 * Space : O(n)  (radix buffer is reused as the FIFO queue)
 *
 * Usage:  ./sticks            reads  n  then n lengths from stdin
 *         ./sticks --test     validates against exhaustive brute force
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned long long ull;

#define RB 11
#define RS (1 << RB)

static void insertion_sort(ull *a, int n) {
    for (int i = 1; i < n; i++) {
        ull v = a[i]; int j = i - 1;
        while (j >= 0 && a[j] > v) { a[j + 1] = a[j]; j--; }
        a[j + 1] = v;
    }
}

/* sorts a[0..n-1] ascending; b is scratch of size n. Result ends in a. */
static void radix_sort(ull *a, ull *b, int n) {
    if (n < 64) { insertion_sort(a, n); return; }
    ull mx = 0;
    for (int i = 0; i < n; i++) if (a[i] > mx) mx = a[i];
    int passes = 0;
    for (ull t = mx; t; t >>= RB) passes++;
    ull *src = a, *dst = b;
    for (int p = 0; p < passes; p++) {
        int cnt[RS + 1];
        memset(cnt, 0, sizeof cnt);
        int sh = p * RB;
        for (int i = 0; i < n; i++) cnt[((src[i] >> sh) & (RS - 1)) + 1]++;
        for (int i = 0; i < RS; i++) cnt[i + 1] += cnt[i];
        for (int i = 0; i < n; i++) dst[cnt[(src[i] >> sh) & (RS - 1)]++] = src[i];
        ull *t = src; src = dst; dst = t;
    }
    if (src != a) memcpy(a, src, (size_t)n * sizeof(ull));
}

/* a: n lengths (destroyed), q: scratch of size n. Returns min total cost. */
ull min_cost_connect(ull *a, ull *q, int n) {
    if (n <= 1) return 0;
    radix_sort(a, q, n);
    int i = 0, qh = 0, qt = 0;
    ull total = 0;
    for (int k = 0; k < n - 1; k++) {
        ull x, y;
        if (i < n && (qh == qt || a[i] <= q[qh])) x = a[i++]; else x = q[qh++];
        if (i < n && (qh == qt || a[i] <= q[qh])) y = a[i++]; else y = q[qh++];
        ull s = x + y;
        total += s;
        q[qt++] = s;
    }
    return total;
}

/* ---------------- validation (brute force over every merge order) -------- */
static ull brute(ull *a, int n) {
    if (n <= 1) return 0;
    ull best = ~0ULL;
    for (int i = 0; i < n; i++) for (int j = i + 1; j < n; j++) {
        ull b[16]; int m = 0;
        for (int k = 0; k < n; k++) if (k != i && k != j) b[m++] = a[k];
        b[m++] = a[i] + a[j];
        ull c = a[i] + a[j] + brute(b, m);
        if (c < best) best = c;
    }
    return best;
}

/* independent O(n^2) greedy, used for large-n cross-check */
static ull naive_greedy(ull *a, int n) {
    ull tot = 0; int m = n;
    while (m > 1) {
        int i1 = 0; for (int i = 1; i < m; i++) if (a[i] < a[i1]) i1 = i;
        ull x = a[i1]; a[i1] = a[--m];
        int i2 = 0; for (int i = 1; i < m; i++) if (a[i] < a[i2]) i2 = i;
        a[i2] += x; tot += a[i2];
    }
    return tot;
}

static unsigned long long rs = 88172645463325252ULL;
static ull rnd(void) { rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; return rs; }

static ull gen(int mode) {
    switch (mode) {
        case 0: return 1;
        case 1: return rnd() % 5 + 1;
        case 2: return rnd() % 1000 + 1;
        case 3: return 1000000000ULL - rnd() % 3;
        case 4: return 1ULL << (rnd() % 30);
        case 5: return rnd() % 1000000000ULL + 1;
        default: return rnd() % 4000000000000ULL + 1;   /* up to ~4e12 */
    }
}

static int self_test(void) {
    const int T = 10000;
    for (int t = 0; t < T; t++) {
        int n = t < 20 ? t % 3 + 1 : (int)(rnd() % 8) + 1;
        ull a[16], b[16], q[16];
        int mode = t % 7;
        for (int i = 0; i < n; i++) a[i] = b[i] = gen(mode);
        ull e = brute(a, n), g = min_cost_connect(b, q, n);
        if (e != g) { printf("FAIL small t=%d exp=%llu got=%llu\n", t, e, g); return 1; }
    }
    printf("[1] exhaustive brute force (n<=8): %d/%d passed\n", T, T);

    int sizes[] = {1, 2, 3, 63, 64, 65, 255, 256, 1000, 3000};
    int ok = 0, tot = 0;
    for (int s = 0; s < 10; s++) for (int mode = 0; mode < 7; mode++) for (int rep = 0; rep < 12; rep++) {
        int n = sizes[s];
        ull *a = malloc(n * sizeof(ull)), *b = malloc(n * sizeof(ull)), *q = malloc(n * sizeof(ull));
        for (int i = 0; i < n; i++) a[i] = b[i] = gen(mode);
        tot++;
        if (naive_greedy(a, n) == min_cost_connect(b, q, n)) ok++;
        else { printf("FAIL large n=%d mode=%d\n", n, mode); return 1; }
        free(a); free(b); free(q);
    }
    printf("[2] radix path vs independent O(n^2) greedy: %d/%d passed\n", ok, tot);
    return 0;
}

/* ---------------- fast input ---------------- */
static char *buf; static size_t blen, bpos;
static int read_ull(ull *out) {
    while (bpos < blen && (buf[bpos] < '0' || buf[bpos] > '9')) bpos++;
    if (bpos >= blen) return 0;
    ull v = 0;
    while (bpos < blen && buf[bpos] >= '0' && buf[bpos] <= '9') v = v * 10 + (ull)(buf[bpos++] - '0');
    *out = v; return 1;
}

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--test") == 0) return self_test();
    size_t cap = 1 << 16; buf = malloc(cap);
    if (!buf) return 1;
    size_t r;
    while ((r = fread(buf + blen, 1, cap - blen, stdin)) > 0) {
        blen += r;
        if (blen == cap) { cap <<= 1; char *nb = realloc(buf, cap); if (!nb) return 1; buf = nb; }
    }
    ull n64;
    if (!read_ull(&n64) || n64 == 0) { printf("0\n"); return 0; }
    int n = (int)n64;
    ull *a = malloc((size_t)n * sizeof(ull)), *q = malloc((size_t)n * sizeof(ull));
    if (!a || !q) return 1;
    int cnt = 0;
    while (cnt < n && read_ull(&a[cnt])) cnt++;
    printf("%llu\n", min_cost_connect(a, q, cnt));
    return 0;
}