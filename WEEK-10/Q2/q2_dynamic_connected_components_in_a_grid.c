/* Dynamic Connected Components in a Grid  (Number of Islands II)
 *
 * Input representation
 *   Cell (r,c) is flattened to id = r*C + c and kept in a Disjoint Set Union
 *   (union by size + path halving).  Two storage layouts, chosen automatically:
 *     dense  : arrays indexed by id             (small / medium grids, fastest)
 *     sparse : open-addressing hash id -> slot  (huge grids, memory O(min(K,R*C)))
 *   Both run the identical algorithm; only "where is cell id stored" differs.
 *
 * Input  (stdin) : R C K, then K lines "r c"      (0-indexed; see --one-indexed)
 * Output (stdout): K lines, number of islands after each operation
 *
 * Usage
 *   ./q2 < input.txt            normal run
 *   ./q2 --one-indexed < in     cells are given as 1..R and 1..C
 *   ./q2 --dense | --sparse     force a layout (default: automatic)
 *   ./q2 --verify   < in        check DSU (dense AND sparse) against brute force on this input
 *   ./q2 --selftest [tests]     random tests, DSU (dense AND sparse) vs brute force
 * Build: gcc -std=c11 -Wall -Wextra -O2 q2_dynamic_islands.c -o q2
 * Complexity: O(K * alpha(n)) time after O(1) (sparse) or O(R*C) (dense) setup.
 */
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------------- fast input (portable, fread-based) ---------------- */
static char ibuf[1 << 16];
static size_t ilen = 0, ipos = 0;
static int gc(void) {
    if (ipos == ilen) {
        ilen = fread(ibuf, 1, sizeof ibuf, stdin);
        ipos = 0;
        if (ilen == 0) return EOF;
    }
    return (unsigned char)ibuf[ipos++];
}
static int read_ll(long long *out) {
    int c = gc();
    while (c != EOF && c != '-' && (c < '0' || c > '9')) c = gc();
    if (c == EOF) return 0;
    int neg = 0;
    if (c == '-') { neg = 1; c = gc(); }
    if (c < '0' || c > '9') return 0;
    long long v = 0;
    while (c >= '0' && c <= '9') {
        if (v > (long long)4e17) v = (long long)4e17;       /* clamp: never overflows */
        v = v * 10 + (c - '0');
        c = gc();
    }
    *out = neg ? -v : v;
    return 1;
}

/* Optional instrumentation for complexity experiments: gcc -DSTATS ... */
#ifdef STATS
static unsigned long long st_finds = 0, st_hops = 0;
#define ST_FIND() (st_finds++)
#define ST_HOP()  (st_hops++)
#else
#define ST_FIND() ((void)0)
#define ST_HOP()  ((void)0)
#endif

/* ---------------- grid + DSU ---------------- */
enum { MODE_AUTO = 0, MODE_DENSE = 1, MODE_SPARSE = 2 };

typedef struct {
    long long R, C, count;
    int sparse;
    int *par;             /* par[slot] < 0: slot is a root, -par[slot] = island size */
    unsigned char *land;  /* dense : land[id] == 1 if the cell is land              */
    long long *key;       /* sparse: key[slot] = id of the cell, -1 if slot is empty */
    size_t mask;          /* sparse: table size - 1 (size is a power of two)        */
} Grid;

static void grid_free(Grid *g) {
    free(g->par); free(g->land); free(g->key);
    g->par = NULL; g->land = NULL; g->key = NULL;
}

/* K = number of operations that will be applied (sizes the sparse table). */
static int grid_init(Grid *g, long long R, long long C, long long K, int mode) {
    memset(g, 0, sizeof *g);
    g->R = R; g->C = C;
    if (R <= 0 || C <= 0 || K < 0 || R > LLONG_MAX / C) return 0;
    long long N = R * C;
    int sparse = (mode == MODE_SPARSE) ||
                 (mode == MODE_AUTO && !(N <= INT_MAX && N <= 8 * K + (1LL << 20)));
    if (!sparse) {
        if (N > INT_MAX || (size_t)N > SIZE_MAX / sizeof *g->par) return 0;   /* ids must fit int */
        g->par  = (int *)malloc((size_t)N * sizeof *g->par);   /* water cells need no initialisation */
        g->land = (unsigned char *)calloc((size_t)N, 1);
        if (!g->par || !g->land) { grid_free(g); return 0; }
    } else {
        long long distinct = K < N ? K : N;                    /* at most this many land cells */
        size_t cap = 16;
        while ((long long)cap < 2 * distinct) {                /* load factor <= 1/2 */
            if (cap >= ((size_t)1 << 30)) return 0;            /* slots must fit int */
            cap <<= 1;
        }
        if (cap > SIZE_MAX / sizeof *g->key) return 0;
        g->key = (long long *)malloc(cap * sizeof *g->key);
        g->par = (int *)malloc(cap * sizeof *g->par);
        if (!g->key || !g->par) { grid_free(g); return 0; }
        memset(g->key, 0xFF, cap * sizeof *g->key);            /* every key = -1 (empty) */
        g->mask = cap - 1;
        g->sparse = 1;
    }
    return 1;
}

static size_t hash64(uint64_t x) {                              /* splitmix64 finaliser */
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return (size_t)(x ^ (x >> 31));
}

/* slot of cell id if it is land, else -1 */
static long long find_slot(const Grid *g, long long id) {
    if (!g->sparse) return g->land[id] ? id : -1;
    size_t h = hash64((uint64_t)id) & g->mask;
    while (g->key[h] != -1) {
        if (g->key[h] == id) return (long long)h;
        h = (h + 1) & g->mask;
    }
    return -1;
}

/* make cell id land: returns its slot, or -1 if it already was land */
static long long insert_slot(Grid *g, long long id) {
    if (!g->sparse) {
        if (g->land[id]) return -1;
        g->land[id] = 1;
        return id;
    }
    size_t h = hash64((uint64_t)id) & g->mask;
    while (g->key[h] != -1) {
        if (g->key[h] == id) return -1;
        h = (h + 1) & g->mask;
    }
    g->key[h] = id;
    return (long long)h;
}

/* iterative find with path halving */
static int find(int *par, int x) {
    ST_FIND();
    while (par[x] >= 0 && par[par[x]] >= 0) { par[x] = par[par[x]]; x = par[x]; ST_HOP(); }
    if (par[x] >= 0) { ST_HOP(); return par[x]; }
    return x;
}

/* union by size; returns 1 if two different islands were merged */
static int unite(int *par, int a, int b) {
    a = find(par, a); b = find(par, b);
    if (a == b) return 0;
    if (par[a] > par[b]) { int t = a; a = b; b = t; }          /* a = larger island */
    par[a] += par[b];
    par[b] = a;
    return 1;
}

/* add land at (r,c); returns the island count afterwards.
   Duplicate or out-of-range cells leave the count unchanged. */
static long long grid_add(Grid *g, long long r, long long c) {
    static const int dr[4] = {-1, 1, 0, 0}, dc[4] = {0, 0, -1, 1};
    if (r < 0 || r >= g->R || c < 0 || c >= g->C) return g->count;
    long long s = insert_slot(g, r * g->C + c);
    if (s < 0) return g->count;                                /* duplicate: nothing changes */
    g->par[s] = -1;
    g->count++;                                                /* a new island is born */
    for (int d = 0; d < 4; d++) {
        long long nr = r + dr[d], nc = c + dc[d];
        if (nr < 0 || nr >= g->R || nc < 0 || nc >= g->C) continue;
        long long t = find_slot(g, nr * g->C + nc);
        if (t >= 0 && unite(g->par, (int)s, (int)t)) g->count--;   /* two islands merged */
    }
    return g->count;
}

/* ---------------- brute force (validator): full DFS after every op ---------------- */
static int brute_run(long long R, long long C, size_t K, const long long *rs,
                     const long long *cs, long long *out) {
    if (R <= 0 || C <= 0 || R > 50000000LL / C) return 0;      /* too big to brute force */
    size_t N = (size_t)(R * C);
    unsigned char *land = (unsigned char *)calloc(N, 1), *seen = (unsigned char *)malloc(N);
    int *stack = (int *)malloc(N * sizeof *stack);
    if (!land || !seen || !stack) { free(land); free(seen); free(stack); return 0; }
    static const int dr[4] = {-1, 1, 0, 0}, dc[4] = {0, 0, -1, 1};
    for (size_t i = 0; i < K; i++) {
        if (rs[i] >= 0 && rs[i] < R && cs[i] >= 0 && cs[i] < C) land[rs[i] * C + cs[i]] = 1;
        memset(seen, 0, N);
        long long cnt = 0;
        for (size_t s = 0; s < N; s++) {
            if (!land[s] || seen[s]) continue;
            cnt++;
            size_t top = 0; stack[top++] = (int)s; seen[s] = 1;
            while (top) {
                int x = stack[--top]; long long xr = x / C, xc = x % C;
                for (int d = 0; d < 4; d++) {
                    long long nr = xr + dr[d], nc = xc + dc[d];
                    if (nr < 0 || nr >= R || nc < 0 || nc >= C) continue;
                    int y = (int)(nr * C + nc);
                    if (land[y] && !seen[y]) { seen[y] = 1; stack[top++] = y; }
                }
            }
        }
        out[i] = cnt;
    }
    free(land); free(seen); free(stack);
    return 1;
}

static int dsu_run(long long R, long long C, size_t K, const long long *rs,
                   const long long *cs, long long *out, int mode) {
    Grid g;
    if (!grid_init(&g, R, C, (long long)K, mode)) return 0;
    for (size_t i = 0; i < K; i++) out[i] = grid_add(&g, rs[i], cs[i]);
    grid_free(&g);
    return 1;
}

/* DSU (dense and sparse) vs brute force.
   Returns -1 on resource failure, 0 if all equal, else the 1-based index of the first mismatch
   (*bad_mode tells which layout disagreed). */
static long long compare_runs(long long R, long long C, size_t K, const long long *rs,
                              const long long *cs, long long *a, long long *b, int *bad_mode) {
    if (!brute_run(R, C, K, rs, cs, b)) return -1;
    for (int mode = MODE_DENSE; mode <= MODE_SPARSE; mode++) {
        if (!dsu_run(R, C, K, rs, cs, a, mode)) return -1;
        for (size_t i = 0; i < K; i++)
            if (a[i] != b[i]) { *bad_mode = mode; return (long long)i + 1; }
    }
    return 0;
}

static long long base_index = 0;   /* 1 with --one-indexed */

/* ---------------- --verify : check the given input ---------------- */
static int verify_mode(void) {
    long long R, C, K;
    if (!read_ll(&R) || !read_ll(&C) || !read_ll(&K) || K < 0 || K > 100000000LL) {
        fprintf(stderr, "bad input\n"); return 1;
    }
    size_t n = (size_t)K;
    long long *rs = (long long *)malloc((n + 1) * sizeof *rs), *cs = (long long *)malloc((n + 1) * sizeof *cs);
    long long *a = (long long *)malloc((n + 1) * sizeof *a), *b = (long long *)malloc((n + 1) * sizeof *b);
    int rc = 1;
    if (rs && cs && a && b) {
        size_t m = 0;
        while (m < n && read_ll(&rs[m]) && read_ll(&cs[m])) { rs[m] -= base_index; cs[m] -= base_index; m++; }
        int bad = 0;
        long long res = compare_runs(R, C, m, rs, cs, a, b, &bad);
        if (res == 0) {
            printf("PASS: dense and sparse DSU both match brute force on all %llu operations\n", (unsigned long long)m);
            rc = 0;
        } else if (res < 0) fprintf(stderr, "allocation failed or grid too large to brute-force\n");
        else printf("FAIL at operation %lld (%s layout): dsu=%lld brute=%lld\n", res,
                    bad == MODE_DENSE ? "dense" : "sparse", a[res - 1], b[res - 1]);
    } else fprintf(stderr, "allocation failed\n");
    free(rs); free(cs); free(a); free(b);
    return rc;
}

/* ---------------- --selftest : random tests ---------------- */
static uint64_t rng_state = 88172645463325252ULL;
static uint64_t rnd(void) {                                    /* xorshift64 */
    rng_state ^= rng_state << 13; rng_state ^= rng_state >> 7; rng_state ^= rng_state << 17;
    return rng_state;
}

static int selftest_mode(long long tests) {
    enum { MAXC = 10, MAXK = 2 * MAXC * MAXC + 8 };
    long long rs[MAXK], cs[MAXK], a[MAXK], b[MAXK];
    for (long long t = 1; t <= tests; t++) {
        long long R = 1 + (long long)(rnd() % MAXC), C = 1 + (long long)(rnd() % MAXC);
        size_t K;
        if (rnd() % 4 == 0) {                                  /* a full random permutation of all cells */
            K = (size_t)(R * C);
            for (size_t i = 0; i < K; i++) { rs[i] = (long long)i / C; cs[i] = (long long)i % C; }
            for (size_t i = K; i > 1; i--) {
                size_t j = (size_t)(rnd() % i);
                long long x = rs[i - 1]; rs[i - 1] = rs[j]; rs[j] = x;
                x = cs[i - 1]; cs[i - 1] = cs[j]; cs[j] = x;
            }
        } else {                                               /* random ops: duplicates + a few invalid */
            K = (size_t)(rnd() % (uint64_t)(2 * R * C + 6));
            for (size_t i = 0; i < K; i++) {
                rs[i] = (long long)(rnd() % (uint64_t)R); cs[i] = (long long)(rnd() % (uint64_t)C);
                if (rnd() % 16 == 0) rs[i] = (rnd() & 1) ? R : -1;
            }
        }
        int bad = 0;
        long long res = compare_runs(R, C, K, rs, cs, a, b, &bad);
        if (res != 0) {
            printf("FAIL on test %lld (R=%lld C=%lld, %s layout) at op %lld\n", t, R, C,
                   bad == MODE_DENSE ? "dense" : "sparse", res);
            return 1;
        }
    }
    printf("selftest: all %lld random tests passed (dense DSU == sparse DSU == brute force)\n", tests);
    return 0;
}

/* ---------------- main ---------------- */
int main(int argc, char **argv) {
    int mode = MODE_AUTO, verify = 0, selftest = 0;
    long long tests = 100000;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--verify")) verify = 1;
        else if (!strcmp(argv[i], "--selftest")) {
            selftest = 1;
            if (i + 1 < argc && argv[i + 1][0] >= '0' && argv[i + 1][0] <= '9') tests = atoll(argv[++i]);
        }
        else if (!strcmp(argv[i], "--one-indexed")) base_index = 1;
        else if (!strcmp(argv[i], "--dense"))  mode = MODE_DENSE;
        else if (!strcmp(argv[i], "--sparse")) mode = MODE_SPARSE;
        else { fprintf(stderr, "unknown option %s\n", argv[i]); return 2; }
    }
    if (selftest) return selftest_mode(tests);
    if (verify) return verify_mode();

    long long R, C, K;
    if (!read_ll(&R) || !read_ll(&C) || !read_ll(&K)) return 0;       /* empty input */
    Grid g;
    if (!grid_init(&g, R, C, K, mode)) { fprintf(stderr, "invalid dimensions or out of memory\n"); return 1; }

    static char obuf[1 << 16];                                 /* streamed output: memory independent of K */
    size_t op = 0;
    for (long long i = 0; i < K; i++) {
        long long r, c;
        if (!read_ll(&r) || !read_ll(&c)) break;               /* truncated input */
        long long cnt = grid_add(&g, r - base_index, c - base_index);
        if (op > sizeof obuf - 32) { fwrite(obuf, 1, op, stdout); op = 0; }
        op += (size_t)sprintf(obuf + op, "%lld\n", cnt);
    }
    fwrite(obuf, 1, op, stdout);
#ifdef STATS
    fprintf(stderr, "STATS %llu %llu\n", st_finds, st_hops);
#endif
    grid_free(&g);
    return 0;
}