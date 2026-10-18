#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- fast input: read all of stdin once ---- */
static char *buf; static size_t blen, bpos;

static int read_all(void) {
    size_t cap = 1 << 16; blen = 0;
    buf = malloc(cap);
    if (!buf) return 0;
    for (;;) {
        size_t got = fread(buf + blen, 1, cap - blen, stdin);
        blen += got;
        if (blen < cap) break;                    /* EOF */
        cap <<= 1;
        char *t = realloc(buf, cap);
        if (!t) return 0;
        buf = t;
    }
    return 1;
}
/* parse next integer; returns 0 on failure */
static int next_int(long long *out) {
    while (bpos < blen && (buf[bpos] < '0' || buf[bpos] > '9') && buf[bpos] != '-') bpos++;
    if (bpos >= blen) return 0;
    int neg = 0;
    if (buf[bpos] == '-') { neg = 1; bpos++; }
    if (bpos >= blen || buf[bpos] < '0' || buf[bpos] > '9') return 0;
    long long x = 0;
    while (bpos < blen && buf[bpos] >= '0' && buf[bpos] <= '9') {
        x = x * 10 + (buf[bpos++] - '0');
        if (x > 4000000000LL) x = 4000000000LL;   /* clamp: avoids overflow */
    }
    *out = neg ? -x : x;
    return 1;
}

/* ---- DSU: path compression + union by size => O(alpha(n)) amortized ---- */
static int *par, *sz;
/* operation counters, used only by explain() for the complexity analysis */
static long long c_find = 0, c_steps = 0, c_union = 0, c_merge = 0;

static int find(int x) {
    int r = x;
    c_find++;
    while (par[r] != r) { r = par[r]; c_steps++; }                          /* walk to root */
    while (par[x] != r) { int nx = par[x]; par[x] = r; x = nx; c_steps++; } /* compress path */
    return r;
}
static void unite(int a, int b) {
    c_union++;
    a = find(a); b = find(b);
    if (a == b) return;
    c_merge++;
    if (sz[a] < sz[b]) { int t = a; a = b; b = t; }
    par[b] = a; sz[a] += sz[b];
}

/* ------------------------------------------------------------------
   explain(): optional analysis report printed to stderr (run with -e).
   orig[1..n] = input permutation, res[1..n] = computed answer.
   It (1) lists components, (2) shows how each is rearranged,
   (3) reports operation counts against the (n+m) bound, and
   (4) machine-checks the facts the correctness proof relies on.
   ------------------------------------------------------------------ */
static void explain(int n, long long m, const int *orig, const int *res, const int *posOf) {
    enum { MAXC = 8, MAXE = 12 };                 /* print limits for big inputs */
    /* snapshot now: the algorithm's own work, before explain() calls find() itself */
    const long long f0 = c_find, s0 = c_steps;
    int comps = 0, largest = 0;
    for (int i = 1; i <= n; i++)
        if (par[i] == i) { comps++; if (sz[i] > largest) largest = sz[i]; }

    fprintf(stderr, "==== ANALYSIS ====\n");
    fprintf(stderr, "n=%d  m=%lld  components=%d  largest component=%d\n", n, m, comps, largest);

    /* (1)+(2) per-component table (first MAXC components, first MAXE entries each) */
    int shown = 0;
    for (int r = 1; r <= n && shown < MAXC; r++) {
        if (par[r] != r) continue;
        shown++;
        const char *dots = sz[r] > MAXE ? "..." : "";
        fprintf(stderr, "component %d (size %d)\n  positions : ", shown, sz[r]);
        int k = 0;
        for (int i = 1; i <= n && k < MAXE; i++) if (find(i) == r) { fprintf(stderr, "%d ", i); k++; }
        fprintf(stderr, "%s\n  original  : ", dots);
        k = 0;
        for (int i = 1; i <= n && k < MAXE; i++) if (find(i) == r) { fprintf(stderr, "%d ", orig[i]); k++; }
        fprintf(stderr, "%s\n  rearranged: ", dots);
        k = 0;
        for (int i = 1; i <= n && k < MAXE; i++) if (find(i) == r) { fprintf(stderr, "%d ", res[i]); k++; }
        fprintf(stderr, "%s\n", dots);
    }
    if (comps > shown) fprintf(stderr, "(%d more components not shown)\n", comps - shown);

    /* (3) complexity evidence (uses the snapshot taken at the top) */
    double bound = (double)n + (double)m;
    fprintf(stderr, "-- complexity evidence --\n");
    fprintf(stderr, "unite calls=%lld, successful merges=%lld (must be <= n-1 = %d)\n", c_union, c_merge, n - 1);
    fprintf(stderr, "find calls=%lld, pointer steps=%lld, steps per find=%.3f\n",
            f0, s0, f0 ? (double)s0 / (double)f0 : 0.0);
    fprintf(stderr, "(finds+steps)/(n+m) = %.3f  (a small constant => near-linear)\n",
            bound > 0 ? (double)(f0 + s0) / bound : 0.0);

    /* (4) invariant checks, each O(n alpha) */
    int ok_perm = 1, ok_comp = 1, ok_sorted = 1;
    int *posRes = calloc((size_t)n + 1, sizeof *posRes);   /* posRes[v] = index of v in res */
    int *last   = calloc((size_t)n + 1, sizeof *last);     /* last value seen per component */
    if (!posRes || !last) {
        fprintf(stderr, "checks skipped: alloc failed\n");
        free(posRes); free(last);
        return;
    }
    for (int i = 1; i <= n; i++) {
        int v = res[i];
        if (v < 1 || v > n || posRes[v]) ok_perm = 0; else posRes[v] = i;
    }
    if (ok_perm) {
        for (int v = 1; v <= n; v++)              /* each value stays in its component */
            if (find(posOf[v]) != find(posRes[v])) { ok_comp = 0; break; }
        for (int i = 1; i <= n; i++) {            /* ascending positions => ascending values */
            int r = find(i);
            if (res[i] <= last[r]) { ok_sorted = 0; break; }
            last[r] = res[i];
        }
    }
    (void)orig;
    fprintf(stderr, "-- invariant checks --\n");
    fprintf(stderr, "[%s] result is a permutation of 1..n\n", ok_perm ? "PASS" : "FAIL");
    fprintf(stderr, "[%s] every value stays inside its original component\n", ok_comp ? "PASS" : "FAIL");
    fprintf(stderr, "[%s] inside each component values increase with position (minimal)\n", ok_sorted ? "PASS" : "FAIL");
    free(posRes); free(last);
}

int main(int argc, char **argv) {
    int want_explain = (argc > 1 && strcmp(argv[1], "-e") == 0);
    long long n_, m_;
    if (!read_all()) { fprintf(stderr, "alloc failed\n"); return 1; }
    if (!next_int(&n_) || !next_int(&m_) || n_ < 0 || m_ < 0 || n_ > 100000000LL) {
        fprintf(stderr, "invalid input\n"); free(buf); return 1;
    }
    int n = (int)n_; long long m = m_;
    if (n == 0) { putchar('\n'); free(buf); return 0; }

    int *posOf = calloc((size_t)n + 1, sizeof *posOf);   /* posOf[v]=index of v, 0=unseen */
    int *next  = malloc(((size_t)n + 1) * sizeof *next); /* ascending position list per comp */
    int *head  = malloc(((size_t)n + 1) * sizeof *head);
    int *res   = malloc(((size_t)n + 1) * sizeof *res);
    par        = malloc(((size_t)n + 1) * sizeof *par);
    sz         = malloc(((size_t)n + 1) * sizeof *sz);
    int *orig  = want_explain ? malloc(((size_t)n + 1) * sizeof *orig) : NULL;
    char *out  = malloc((size_t)n * 11 + 2);
    int rc = 0;
    if (!posOf || !next || !head || !res || !par || !sz || !out || (want_explain && !orig)) {
        fprintf(stderr, "alloc failed\n"); rc = 1; goto done;
    }

    /* read + VALIDATE that A is a permutation of 1..n */
    for (int i = 1; i <= n; i++) {
        long long v;
        if (!next_int(&v) || v < 1 || v > n || posOf[v] != 0) {
            fprintf(stderr, "invalid permutation\n"); rc = 1; goto done;
        }
        if (orig) orig[i] = (int)v;
        posOf[v] = i; par[i] = i; sz[i] = 1; head[i] = 0;
    }
    /* union every allowed swap; ignore out-of-range edges */
    for (long long i = 0; i < m; i++) {
        long long u, v;
        if (!next_int(&u) || !next_int(&v)) break;
        if (u < 1 || u > n || v < 1 || v > n) continue;
        unite((int)u, (int)v);
    }
    /* per component: positions in ascending order (insert n..1 at head) */
    for (int i = n; i >= 1; i--) {
        int r = find(i);
        next[i] = head[r]; head[r] = i;
    }
    /* values 1..n ascending -> smallest free position of their component */
    for (int v = 1; v <= n; v++) {
        int r = find(posOf[v]);
        int p = head[r];
        res[p] = v; head[r] = next[p];
    }
    /* fast output */
    size_t o = 0;
    for (int i = 1; i <= n; i++) {
        char t[12]; int l = 0, x = res[i];
        while (x) { t[l++] = (char)('0' + x % 10); x /= 10; }
        while (l) out[o++] = t[--l];
        out[o++] = (i == n) ? '\n' : ' ';
    }
    fwrite(out, 1, o, stdout);
    if (want_explain) explain(n, m, orig, res, posOf);
done:
    free(buf); free(posOf); free(next); free(head); free(res); free(par); free(sz); free(out); free(orig);
    return rc;
}