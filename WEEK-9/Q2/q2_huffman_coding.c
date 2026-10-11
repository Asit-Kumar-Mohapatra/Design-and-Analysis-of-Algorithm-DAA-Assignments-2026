/*
 * huffman.c  --  Optimal prefix-free code (Huffman) with canonical codebook
 *
 * Build : gcc -O2 -std=c11 -Wall -Wextra -o huffman huffman.c
 *
 * Usage :
 *   huffman                  read input from stdin, print canonical codebook
 *   huffman -v               same, and also run the built-in validator
 *   huffman --stress N [S]   self-test on N random test cases (seed S)
 *
 * INPUT REPRESENTATION (array indexed by symbol id)
 *   n
 *   f[0] f[1] ... f[n-1]      non-negative integers, symbol i has frequency f[i]
 *   (symbol id i can be an alphabet index, or an ASCII code if n = 256)
 *   Constraint: sum of all f[i] <= 1e17 (keeps every 64-bit quantity exact).
 *
 * OUTPUT
 *   Symbol Frequency Length Code      <- one row per symbol, rows ordered by
 *   ...                                  (code length, symbol id)  = canonical order
 *   Total frequency / weighted bits / expected length
 *
 * ALGORITHM (n >= 2)
 *   1. Sort symbols by (frequency, id)            : O(n log n), or O(n) radix
 *   2. Two-queue merge  (leaves | internal nodes) : O(n), no heap
 *      ties are broken in favour of LEAVES -> minimum-variance Huffman code
 *   3. Code lengths from the parent array, in place : O(n)
 *   4. Counting sort of symbols by (length, id)    : O(n + L)
 *   5. Canonical codes: code(next) = (code+1) << (len(next)-len(cur)) : O(total output)
 *   Space: O(n) words (+ O(L) for one code buffer), L = longest code <= n-1.
 *   Special cases: n = 0 -> empty codebook, n = 1 -> the single symbol gets "0".
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_TOTAL 100000000000000000ULL /* 1e17 */

/* ------------------------------------------------------------------ */
/*  Sorting symbols by (frequency, id)                                */
/* ------------------------------------------------------------------ */
static uint32_t g_radix_threshold = 1024; /* n >= threshold -> radix sort */
static const uint64_t *g_f;

static int cmp_idx(const void *a, const void *b)
{
    uint32_t x = *(const uint32_t *)a, y = *(const uint32_t *)b;
    if (g_f[x] != g_f[y]) return g_f[x] < g_f[y] ? -1 : 1;
    return (x > y) - (x < y);
}

/* stable LSD radix sort of indices 0..n-1 by 64-bit key (11-bit digits) */
static void radix_sort_idx(uint32_t n, const uint64_t *f, uint32_t *idx, uint32_t *tmp)
{
    enum { B = 11, R = 1 << B };
    static uint32_t cnt[R];
    uint32_t *src = idx, *dst = tmp;
    for (uint32_t i = 0; i < n; i++) src[i] = i;
    for (int shift = 0; shift < 64; shift += B) {
        memset(cnt, 0, sizeof cnt);
        for (uint32_t i = 0; i < n; i++) cnt[(f[src[i]] >> shift) & (R - 1)]++;
        if (cnt[(f[src[0]] >> shift) & (R - 1)] == n) continue; /* digit constant */
        uint32_t sum = 0;
        for (uint32_t b = 0; b < R; b++) { uint32_t t = cnt[b]; cnt[b] = sum; sum += t; }
        for (uint32_t i = 0; i < n; i++) dst[cnt[(f[src[i]] >> shift) & (R - 1)]++] = src[i];
        uint32_t *t = src; src = dst; dst = t;
    }
    if (src != idx) memcpy(idx, src, (size_t)n * sizeof *idx);
}

/* ------------------------------------------------------------------ */
/*  Huffman code lengths (two-queue method)                           */
/*  len[s] = code length of symbol s ; returns 0 on success           */
/* ------------------------------------------------------------------ */
static int huffman_lengths(uint32_t n, const uint64_t *freq, uint32_t *len,
                           uint64_t *cost_out, uint32_t *maxlen_out)
{
    *cost_out = 0;
    *maxlen_out = 0;
    if (n == 0) return 0;
    if (n == 1) { len[0] = 1; *cost_out = freq[0]; *maxlen_out = 1; return 0; }

    size_t nodes = 2 * (size_t)n - 1;
    uint32_t *idx = malloc((size_t)n * sizeof *idx);
    uint32_t *tmp = malloc((size_t)n * sizeof *tmp);
    uint64_t *sw  = malloc((size_t)n * sizeof *sw);        /* sorted leaf weights   */
    uint64_t *iw  = malloc((size_t)(n - 1) * sizeof *iw);  /* internal node weights */
    uint32_t *par = malloc(nodes * sizeof *par);           /* parent, then depth    */
    if (!idx || !tmp || !sw || !iw || !par) {
        free(idx); free(tmp); free(sw); free(iw); free(par);
        return -1;
    }

    if (n < g_radix_threshold) {
        for (uint32_t i = 0; i < n; i++) idx[i] = i;
        g_f = freq;
        qsort(idx, n, sizeof *idx, cmp_idx);
    } else {
        radix_sort_idx(n, freq, idx, tmp);
    }
    for (uint32_t r = 0; r < n; r++) sw[r] = freq[idx[r]];

    /* nodes 0..n-1 = leaves by rank, n..2n-2 = internal nodes in creation order */
    uint32_t li = 0, ii = 0;
    uint64_t cost = 0;
    for (uint32_t k = 0; k < n - 1; k++) {
        uint32_t node[2];
        uint64_t w[2];
        for (int t = 0; t < 2; t++) {
            if (li < n && (ii >= k || sw[li] <= iw[ii])) { /* tie -> leaf first */
                node[t] = li; w[t] = sw[li]; li++;
            } else {
                node[t] = n + ii; w[t] = iw[ii]; ii++;
            }
        }
        iw[k] = w[0] + w[1];
        par[node[0]] = par[node[1]] = n + k;
        cost += iw[k];
    }

    /* parent index > child index, so one downward sweep turns parents into depths */
    par[nodes - 1] = 0;
    for (size_t i = nodes - 1; i-- > 0;) par[i] = par[par[i]] + 1;

    uint32_t mx = 0;
    for (uint32_t r = 0; r < n; r++) {
        len[idx[r]] = par[r];
        if (par[r] > mx) mx = par[r];
    }
    *cost_out = cost;
    *maxlen_out = mx;
    free(idx); free(tmp); free(sw); free(iw); free(par);
    return 0;
}

/* ------------------------------------------------------------------ */
/*  Canonical order and canonical codes                               */
/* ------------------------------------------------------------------ */
static int canonical_order(uint32_t n, const uint32_t *len, uint32_t maxlen, uint32_t *order)
{
    uint32_t *pos = calloc((size_t)maxlen + 2, sizeof *pos);
    if (!pos) return -1;
    for (uint32_t i = 0; i < n; i++) pos[len[i]]++;
    uint32_t sum = 0;
    for (uint32_t l = 0; l <= maxlen; l++) { uint32_t t = pos[l]; pos[l] = sum; sum += t; }
    for (uint32_t i = 0; i < n; i++) order[pos[len[i]]++] = i; /* stable: ids ascending */
    free(pos);
    return 0;
}

typedef void (*code_cb)(void *ctx, uint32_t sym, uint32_t len, const char *code);

static int canonical_codes(uint32_t n, const uint32_t *len, const uint32_t *order,
                           uint32_t maxlen, code_cb cb, void *ctx)
{
    char *buf = malloc((size_t)maxlen + 2);
    if (!buf) return -1;
    uint32_t cur = 0;
    for (uint32_t k = 0; k < n; k++) {
        uint32_t s = order[k], l = len[s];
        if (k == 0) {
            memset(buf, '0', l);
            cur = l;
        } else {
            long p = (long)cur - 1;           /* code = code + 1 */
            while (p >= 0 && buf[p] == '1') { buf[p] = '0'; p--; }
            if (p < 0) { free(buf); return -2; } /* code space exhausted: invalid */
            buf[p] = '1';
            if (l > cur) { memset(buf + cur, '0', l - cur); cur = l; } /* shift left */
        }
        buf[cur] = '\0';
        cb(ctx, s, l, buf);
    }
    free(buf);
    return 0;
}

/* ------------------------------------------------------------------ */
/*  Buffered output                                                   */
/* ------------------------------------------------------------------ */
static char obuf[1 << 16];
static size_t olen;
static void oflush(void) { fwrite(obuf, 1, olen, stdout); olen = 0; }
static void oputc(char c) { if (olen == sizeof obuf) oflush(); obuf[olen++] = c; }
static void oputs(const char *s) { while (*s) oputc(*s++); }
static void oputu(uint64_t v)
{
    char t[24]; int k = 0;
    do { t[k++] = (char)('0' + v % 10); v /= 10; } while (v);
    while (k) oputc(t[--k]);
}

static void print_cb(void *ctx, uint32_t sym, uint32_t len, const char *code)
{
    const uint64_t *freq = ctx;
    oputu(sym); oputc(' ');
    oputu(freq[sym]); oputc(' ');
    oputu(len); oputc(' ');
    oputs(code); oputc('\n');
}

/* ------------------------------------------------------------------ */
/*  Validator (independent checks)                                    */
/* ------------------------------------------------------------------ */
static void sift(uint64_t *h, size_t sz, size_t i)
{
    for (;;) {
        size_t l = 2 * i + 1, r = l + 1, m = i;
        if (l < sz && h[l] < h[m]) m = l;
        if (r < sz && h[r] < h[m]) m = r;
        if (m == i) return;
        uint64_t t = h[i]; h[i] = h[m]; h[m] = t;
        i = m;
    }
}

/* textbook heap-based Huffman cost, used only as an oracle */
static uint64_t heap_huffman_cost(uint32_t n, const uint64_t *freq)
{
    if (n == 0) return 0;
    if (n == 1) return freq[0];
    uint64_t *h = malloc((size_t)n * sizeof *h);
    if (!h) return (uint64_t)-1;
    memcpy(h, freq, (size_t)n * sizeof *h);
    size_t sz = n;
    for (size_t i = sz / 2; i-- > 0;) sift(h, sz, i);
    uint64_t cost = 0;
    while (sz > 1) {
        uint64_t a = h[0];
        h[0] = h[--sz]; sift(h, sz, 0);
        uint64_t b = h[0];
        h[0] = a + b; sift(h, sz, 0);
        cost += a + b;
    }
    free(h);
    return cost;
}

/* exhaustive optimum over all length vectors with Kraft sum <= 1 (n <= 6) */
static uint64_t bf_best, bf_maxlen;
static void bf_rec(uint32_t i, uint32_t n, const uint64_t *f, uint32_t *L,
                   uint64_t kr, uint64_t cost, uint32_t mx)
{
    if (i == n) {
        if (cost < bf_best || (cost == bf_best && mx < bf_maxlen)) { bf_best = cost; bf_maxlen = mx; }
        return;
    }
    for (uint32_t l = 1; l <= n - 1; l++) {
        uint64_t k2 = kr + (1ULL << (n - 1 - l));
        if (k2 > (1ULL << (n - 1))) continue;
        L[i] = l;
        bf_rec(i + 1, n, f, L, k2, cost + f[i] * l, mx > l ? mx : l);
    }
}

typedef struct {
    uint32_t k;
    uint32_t *sym, *len;
    size_t *off;
    char *pool;
    size_t used;
} Collect;

static void collect_cb(void *c, uint32_t s, uint32_t l, const char *code)
{
    Collect *C = c;
    C->sym[C->k] = s; C->len[C->k] = l; C->off[C->k] = C->used;
    memcpy(C->pool + C->used, code, (size_t)l + 1);
    C->used += (size_t)l + 1;
    C->k++;
}

/* returns NULL if everything is correct, else a message */
static const char *validate(uint32_t n, const uint64_t *freq, const uint32_t *len,
                            const uint32_t *order, uint32_t maxlen, uint64_t cost)
{
    if (n == 0) return NULL;
    const char *err = NULL;
    Collect C = {0};
    size_t total = 0;
    for (uint32_t i = 0; i < n; i++) total += (size_t)len[i] + 1;
    C.sym = malloc((size_t)n * sizeof *C.sym);
    C.len = malloc((size_t)n * sizeof *C.len);
    C.off = malloc((size_t)n * sizeof *C.off);
    C.pool = malloc(total);
    unsigned char *seen = calloc(n, 1);
    uint32_t *cnt = calloc((size_t)maxlen + 2, sizeof *cnt);
    if (!C.sym || !C.len || !C.off || !C.pool || !seen || !cnt) { err = "validator out of memory"; goto done; }

    if (canonical_codes(n, len, order, maxlen, collect_cb, &C) != 0) { err = "canonical code space overflow"; goto done; }
    if (C.k != n) { err = "wrong number of codes"; goto done; }

    for (uint32_t k = 0; k < n; k++) {
        const char *cd = C.pool + C.off[k];
        uint32_t s = C.sym[k];
        if (s >= n || seen[s]) { err = "symbol repeated / out of range"; goto done; }
        seen[s] = 1;
        if (len[s] < 1 || len[s] > maxlen) { err = "length out of range"; goto done; }
        if (strlen(cd) != len[s] || C.len[k] != len[s]) { err = "code length mismatch"; goto done; }
        for (uint32_t j = 0; j < len[s]; j++)
            if (cd[j] != '0' && cd[j] != '1') { err = "non-binary code"; goto done; }
        if (k > 0) {
            const char *pv = C.pool + C.off[k - 1];
            uint32_t ps = C.sym[k - 1];
            if (!(len[ps] < len[s] || (len[ps] == len[s] && ps < s))) { err = "not canonical order"; goto done; }
            if (strcmp(pv, cd) >= 0) { err = "codes not lexicographically increasing"; goto done; }
            if (strncmp(pv, cd, strlen(pv)) == 0) { err = "prefix property violated"; goto done; }
        }
        cnt[len[s]]++;
    }

    if (n >= 2) { /* Kraft equality: the code tree is full, nothing is wasted */
        long long need = 1;
        for (uint32_t l = 1; l <= maxlen; l++) {
            need = need * 2 - (long long)cnt[l];
            if (need < 0 || need > (long long)n) { err = "Kraft sum violated"; goto done; }
        }
        if (need != 0) { err = "Kraft sum < 1 (code not complete)"; goto done; }
    }

    {
        uint64_t c2 = 0;
        for (uint32_t i = 0; i < n; i++) c2 += freq[i] * len[i];
        if (c2 != cost) { err = "reported cost != sum f*len"; goto done; }
        if (heap_huffman_cost(n, freq) != cost) { err = "cost differs from heap-based Huffman"; goto done; }
    }

    if (n >= 2 && n <= 6) {
        uint32_t L[8];
        bf_best = (uint64_t)-1; bf_maxlen = 99;
        bf_rec(0, n, freq, L, 0, 0, 0);
        if (bf_best != cost) { err = "cost differs from brute-force optimum"; goto done; }
        if (bf_maxlen != maxlen) { err = "max length not minimal among optimal codes"; goto done; }
    }
done:
    free(C.sym); free(C.len); free(C.off); free(C.pool); free(seen); free(cnt);
    return err;
}

/* ------------------------------------------------------------------ */
/*  Random stress test                                                */
/* ------------------------------------------------------------------ */
static uint64_t rs = 88172645463325252ULL;
static uint64_t rnd(void) { rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; return rs; }
static uint64_t rr(uint64_t lo, uint64_t hi) { return lo + rnd() % (hi - lo + 1); }

#define STRESS_CAP 5000
static const char *type_name[10] = {
    "small 1..10", "medium 1..1e6", "all equal", "powers of two", "fibonacci",
    "with zeros", "one huge", "geometric", "sorted asc/desc", "large values"
};

static uint32_t gen(uint64_t *f, int *type_out)
{
    uint32_t n;
    uint64_t r = rr(0, 99);
    if (rr(0, 199) == 0) n = 0;
    else if (r < 35) n = (uint32_t)rr(1, 8);
    else if (r < 65) n = (uint32_t)rr(9, 40);
    else if (r < 85) n = (uint32_t)rr(41, 300);
    else if (r < 95) n = (uint32_t)rr(301, 1500);
    else n = (uint32_t)rr(1501, STRESS_CAP);
    int type = (int)rr(0, 9);
    *type_out = type;
    if (n == 0) return 0;
    uint64_t cap = MAX_TOTAL / n;
    if (cap > 1000000000000ULL) cap = 1000000000000ULL;

    if (type == 4) { /* Fibonacci weights: deepest possible tree */
        if (n > 80) n = 80;
        uint64_t a = 1, b = 1;
        for (uint32_t i = 0; i < n; i++) { f[i] = a; uint64_t t = a + b; a = b; b = t; }
        for (uint32_t i = n; i-- > 1;) { uint32_t j = (uint32_t)rr(0, i); uint64_t t = f[i]; f[i] = f[j]; f[j] = t; }
        return n;
    }
    uint32_t big = (uint32_t)rr(0, n - 1);
    int desc = (int)rr(0, 1);
    uint64_t eq = rr(1, 1000);
    for (uint32_t i = 0; i < n; i++) {
        uint64_t v;
        switch (type) {
        case 0: v = rr(1, 10); break;
        case 1: v = rr(1, 1000000); break;
        case 2: v = eq; break;
        case 3: v = 1ULL << rr(0, 40); break;
        case 5: v = rr(0, 3); break;
        case 6: v = (i == big) ? rr(1000000000ULL, 1000000000000ULL) : rr(1, 5); break;
        case 7: v = 1 + (rnd() >> rr(0, 60)); break;
        case 8: v = desc ? (n - i) : (i + 1); break;
        default: v = rr(1, cap); break;
        }
        if (v > cap) v = 1 + v % cap;
        f[i] = v;
    }
    return n;
}

static int stress(uint64_t N, uint64_t seed)
{
    if (seed) rs = seed;
    uint64_t *f = malloc(STRESS_CAP * sizeof *f);
    uint32_t *len = malloc(STRESS_CAP * sizeof *len);
    uint32_t *len2 = malloc(STRESS_CAP * sizeof *len2);
    uint32_t *order = malloc(STRESS_CAP * sizeof *order);
    if (!f || !len || !len2 || !order) return 2;
    uint64_t pass = 0, fail = 0, tcount[10] = {0}, big = 0, tiny = 0, brute = 0;
    uint32_t maxn = 0, maxL = 0;

    for (uint64_t t = 1; t <= N; t++) {
        int type;
        uint32_t n = gen(f, &type);
        uint64_t cost, cost2;
        uint32_t mx, mx2;
        const char *err = NULL;

        g_radix_threshold = 0xFFFFFFFFu;               /* force qsort path  */
        if (huffman_lengths(n, f, len, &cost, &mx)) return 2;
        g_radix_threshold = 0;                         /* force radix path  */
        if (huffman_lengths(n, f, len2, &cost2, &mx2)) return 2;
        if (cost != cost2 || mx != mx2 || memcmp(len, len2, (size_t)n * sizeof *len))
            err = "qsort and radix paths disagree";
        if (!err && n && canonical_order(n, len, mx, order)) return 2;
        if (!err) err = validate(n, f, len, order, mx, cost);

        tcount[type]++;
        if (n >= 1024) big++;
        if (n <= 6) brute++;
        if (n <= 1) tiny++;
        if (n > maxn) maxn = n;
        if (mx > maxL) maxL = mx;
        if (err) {
            fail++;
            if (fail <= 5) {
                printf("FAIL #%llu type=%s n=%u : %s\n  freq:", (unsigned long long)t, type_name[type], n, err);
                for (uint32_t i = 0; i < n && i < 20; i++) printf(" %llu", (unsigned long long)f[i]);
                printf("\n");
            }
        } else pass++;
    }
    printf("Stress result: %llu / %llu passed, %llu failed\n",
           (unsigned long long)pass, (unsigned long long)N, (unsigned long long)fail);
    printf("  n<=1: %llu tests | n<=6 (brute-force checked): %llu | n>=1024: %llu | largest n=%u | longest code=%u\n",
           (unsigned long long)tiny, (unsigned long long)brute, (unsigned long long)big, maxn, maxL);
    printf("  distribution types:");
    for (int i = 0; i < 10; i++) printf(" [%s]=%llu", type_name[i], (unsigned long long)tcount[i]);
    printf("\n");
    free(f); free(len); free(len2); free(order);
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
    if (!rd_u64(&n64) || n64 >= (1ULL << 30)) { fprintf(stderr, "error: bad n\n"); return 2; }
    uint32_t n = (uint32_t)n64;

    uint64_t *freq = malloc((n ? n : 1) * sizeof *freq);
    uint32_t *len = malloc((n ? n : 1) * sizeof *len);
    uint32_t *order = malloc((n ? n : 1) * sizeof *order);
    if (!freq || !len || !order) { fprintf(stderr, "error: out of memory\n"); return 2; }

    uint64_t total = 0;
    for (uint32_t i = 0; i < n; i++) {
        if (!rd_u64(&freq[i])) { fprintf(stderr, "error: expected %u frequencies\n", n); return 2; }
        if (freq[i] > MAX_TOTAL || total > MAX_TOTAL - freq[i]) {
            fprintf(stderr, "error: total frequency exceeds 1e17\n");
            return 2;
        }
        total += freq[i];
    }

    uint64_t cost;
    uint32_t maxlen;
    if (huffman_lengths(n, freq, len, &cost, &maxlen)) { fprintf(stderr, "error: out of memory\n"); return 2; }
    if (n && canonical_order(n, len, maxlen, order)) { fprintf(stderr, "error: out of memory\n"); return 2; }

    oputs("Symbol Frequency Length Code\n");
    if (n && canonical_codes(n, len, order, maxlen, print_cb, freq)) { oflush(); fprintf(stderr, "error: internal\n"); return 3; }

    oputs("Total frequency      : "); oputu(total); oputc('\n');
    oputs("Total weighted bits  : "); oputu(cost); oputc('\n');
    oflush();
    printf("Expected code length: %.6f bits/symbol\n", total ? (double)cost / (double)total : 0.0);

    if (verify) {
        const char *err = n ? validate(n, freq, len, order, maxlen, cost) : NULL;
        fprintf(stderr, err ? "[VALIDATION] FAIL: %s\n" : "[VALIDATION] PASS\n", err);
        if (err) return 1;
    }
    free(freq); free(len); free(order);
    return 0;
}