/*
 * Shortest Common Superstring (SCS) -- Greedy algorithm, exact solver and validation harness
 * Lab question: "Greedy Superstring Conjecture" (open problem).
 *
 * PROBLEM   Given strings s_1..s_n find the shortest string containing every s_i as a substring.
 *           SCS is NP-hard, so no polynomial-time exact algorithm is known (unless P = NP).
 *           The greedy algorithm below is the object of the conjecture "greedy <= 2 * OPT".
 *
 * PREPROCESSING  Remove duplicates and strings that are substrings of other strings (they are
 *           covered for free). The remaining set is "substring-free", which makes the overlap
 *           graph well defined: ov(a,b) = longest suffix of a that is a prefix of b (< min length).
 *
 * GREEDY (standard formulation, Tarhio-Ukkonen / Blum et al.)
 *           Repeatedly merge the two current strings with the maximum overlap. Equivalent form on
 *           the overlap graph of the original strings: scan all ordered pairs (i,j) by decreasing
 *           ov(i,j); accept (i,j) iff i has no successor, j has no predecessor and i, j lie in
 *           different chains (no cycle). Remaining chains are concatenated (overlap 0).
 *           (Ties are broken by index order, or randomly with a seed -- the conjecture is about
 *           ANY tie-breaking.)
 *
 * EXACT     Held-Karp DP over subsets: dp[mask][last] = max total overlap of an ordering of the
 *           strings in mask ending at 'last'. OPT length = total length - max overlap.
 *
 * COMPLEXITY (n strings after preprocessing, M total length, L longest string)
 *           preprocessing  O(n^2 L) worst case
 *           overlaps       O(n M)   (one KMP failure table per string, every other string is scanned)
 *           greedy         O(n^2)   (counting sort of the <= n^2 pairs by overlap + union-find)
 *           => greedy total O(n M + n^2) time, O(n^2) space (overlap matrix).
 *           exact DP       O(2^n n^2) time, O(2^n n) space (n <= 20).
 *           (Gusfield / Ukkonen give O(M) greedy implementations using Aho-Corasick; not needed here.)
 *
 * USAGE
 *   ./a.out                       stdin: n, then n strings (whitespace separated)
 *                                 prints greedy superstring and, if n' <= 20, the optimal one
 *   ./a.out --test [N] [seed]     validate everything on N random instances (default 10000)
 *   ./a.out --paper [s] [t] [R]   build the instance family of arXiv:2609.01365 (k = 2s, s >= 5,
 *                                 t groups) and measure greedy vs a certified optimum
 *   ./a.out --bench               timing table
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

typedef unsigned char uc;
typedef long long ll;
typedef unsigned long long ull;

#define EXACT_LIMIT 20
#define MAX_STRINGS 40000

static void die(const char *m) { fprintf(stderr, "%s\n", m); exit(1); }

/* ------------------------------ random numbers ------------------------------ */
static ull rs_global = 88172645463325252ULL;
static ull rng(ull *st) {                       /* xorshift64* */
    ull x = *st;
    x ^= x >> 12; x ^= x << 25; x ^= x >> 27;
    *st = x;
    return x * 2685821657736338717ULL;
}
static ll rr(ll lo, ll hi) {                     /* uniform in [lo, hi] */
    if (hi <= lo) return lo;
    return lo + (ll)(rng(&rs_global) % (ull)(hi - lo + 1));
}

/* ------------------------------ string sets ------------------------------ */
typedef struct {
    int n;
    uc **s;
    int *len;
    uc *pool;
    size_t total;
} Set;

static void set_init(Set *S, int n, size_t total) {
    S->n = n; S->total = total;
    S->s = (uc **)malloc((n ? (size_t)n : 1) * sizeof(uc *));
    S->len = (int *)malloc((n ? (size_t)n : 1) * sizeof(int));
    S->pool = (uc *)malloc(total ? total : 1);
    if (!S->s || !S->len || !S->pool) die("out of memory");
}
static void set_free(Set *S) {
    free(S->s); free(S->len); free(S->pool);
    memset(S, 0, sizeof *S);
}

static int contains(const uc *h, size_t lh, const uc *nd, size_t ln) {
    size_t i;
    if (ln == 0) return 1;
    if (ln > lh) return 0;
    for (i = 0; i + ln <= lh; i++) {
        const uc *p = (const uc *)memchr(h + i, nd[0], lh - ln - i + 1);
        if (!p) return 0;
        i = (size_t)(p - h);
        if (memcmp(p, nd, ln) == 0) return 1;
    }
    return 0;
}

static int all_contained(const Set *orig, const uc *sup, size_t len) {
    int i;
    for (i = 0; i < orig->n; i++)
        if (!contains(sup, len, orig->s[i], (size_t)orig->len[i])) return 0;
    return 1;
}

static const Set *g_sort_set;
static int cmp_idx(const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    const Set *S = g_sort_set;
    if (S->len[x] != S->len[y]) return S->len[y] - S->len[x];   /* longer first */
    return memcmp(S->s[x], S->s[y], (size_t)S->len[x]);
}

/* remove duplicates and strings contained in another string */
static void make_substring_free(const Set *in, Set *out) {
    int n = in->n, a, b, nk = 0;
    int *ord = (int *)malloc((n ? (size_t)n : 1) * sizeof(int));
    int *kept = (int *)malloc((n ? (size_t)n : 1) * sizeof(int));
    size_t tot = 0;
    if (!ord || !kept) die("out of memory");
    for (a = 0; a < n; a++) ord[a] = a;
    g_sort_set = in;
    qsort(ord, (size_t)n, sizeof(int), cmp_idx);
    for (a = 0; a < n; a++) {
        int c = ord[a], skip = 0;
        for (b = nk - 1; b >= 0 && !skip; b--)
            if (contains(in->s[kept[b]], (size_t)in->len[kept[b]], in->s[c], (size_t)in->len[c])) skip = 1;
        if (!skip) { kept[nk++] = c; tot += (size_t)in->len[c]; }
    }
    set_init(out, nk, tot);
    tot = 0;
    for (a = 0; a < nk; a++) {
        int c = kept[a];
        out->s[a] = out->pool + tot;
        out->len[a] = in->len[c];
        memcpy(out->s[a], in->s[c], (size_t)in->len[c]);
        tot += (size_t)in->len[c];
    }
    free(ord); free(kept);
}

/* ------------------------------ overlaps ------------------------------ */
static size_t ix(int i, int j, int n) { return (size_t)i * (size_t)n + (size_t)j; }

static int naive_overlap(const uc *a, int la, const uc *b, int lb) {   /* reference, O(L^2) */
    int l, m = (la < lb ? la : lb) - 1;
    for (l = m; l >= 1; l--)
        if (memcmp(a + la - l, b, (size_t)l) == 0) return l;
    return 0;
}

/* ov[i*n+j] = longest suffix of s_i that is a prefix of s_j (KMP, O(n M)) */
static uint16_t *compute_overlaps(const Set *S) {
    int n = S->n, i, j, maxl = 0;
    uint16_t *ov;
    int *pi;
    if ((size_t)n > MAX_STRINGS) die("too many strings after preprocessing");
    for (i = 0; i < n; i++) if (S->len[i] > maxl) maxl = S->len[i];
    if (maxl > 65535) die("string longer than 65535 not supported");
    ov = (uint16_t *)calloc((size_t)n * (size_t)n + 1, sizeof(uint16_t));
    pi = (int *)malloc(((size_t)maxl + 1) * sizeof(int));
    if (!ov || !pi) die("out of memory");
    for (j = 0; j < n; j++) {
        const uc *t = S->s[j];
        int lt = S->len[j], q, k;
        pi[0] = 0;
        for (q = 1, k = 0; q < lt; q++) {
            while (k > 0 && t[k] != t[q]) k = pi[k - 1];
            if (t[k] == t[q]) k++;
            pi[q] = k;
        }
        for (i = 0; i < n; i++) {
            const uc *a = S->s[i];
            int la = S->len[i], p;
            if (i == j) continue;
            q = 0;
            for (p = 0; p < la; p++) {
                while (q > 0 && (q == lt || t[q] != a[p])) q = pi[q - 1];
                if (t[q] == a[p]) q++;
            }
            if (q >= lt || q >= la) q = (lt < la ? lt : la) - 1;   /* cannot happen when substring-free */
            ov[ix(i, j, n)] = (uint16_t)q;
        }
    }
    free(pi);
    return ov;
}

/* ------------------------------ greedy ------------------------------ */
typedef struct { int i, j; } Edge;

static int uf_find(int *par, int x) {
    while (par[x] != x) { par[x] = par[par[x]]; x = par[x]; }
    return x;
}

/* Greedy on the overlap graph. nxt/prv receive the chains. tie_seed == 0: index order among equal
 * overlaps, otherwise a random order. Returns the total overlap used. */
static ll greedy_links(int n, const uint16_t *ov, ull tie_seed, const unsigned char *prio,
                       Edge *trace, int *ntrace, int *nxt, int *prv) {
    size_t m = 0, e, k;
    int i, j, maxo = 0, accepted = 0;
    ll sum = 0;
    Edge *raw, *srt;
    size_t *pos;
    int *par, *sz;

    for (i = 0; i < n; i++) { nxt[i] = -1; prv[i] = -1; }
    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++)
            if (i != j && ov[ix(i, j, n)] > 0) { m++; if (ov[ix(i, j, n)] > maxo) maxo = ov[ix(i, j, n)]; }
    if (ntrace) *ntrace = 0;
    if (m == 0) return 0;
    raw = (Edge *)malloc(m * sizeof(Edge));
    srt = (Edge *)malloc(m * sizeof(Edge));
    pos = (size_t *)calloc((size_t)maxo + 2, sizeof(size_t));
    par = (int *)malloc((size_t)n * sizeof(int));
    sz = (int *)malloc((size_t)n * sizeof(int));
    if (!raw || !srt || !pos || !par || !sz) die("out of memory");
    for (k = 0, i = 0; i < n; i++)
        for (j = 0; j < n; j++)
            if (i != j && ov[ix(i, j, n)] > 0) { raw[k].i = i; raw[k].j = j; k++; }
    if (tie_seed) {                                         /* random tie order */
        ull st = tie_seed | 1ULL;
        for (k = m; k > 1; k--) {
            size_t r = (size_t)(rng(&st) % k);
            Edge t = raw[k - 1]; raw[k - 1] = raw[r]; raw[r] = t;
        }
    }
    if (prio) {                                             /* preferred pairs first among equal overlaps */
        size_t w = 0;
        for (k = 0; k < m; k++) if (prio[ix(raw[k].i, raw[k].j, n)] == 0) srt[w++] = raw[k];
        for (k = 0; k < m; k++) if (prio[ix(raw[k].i, raw[k].j, n)] != 0) srt[w++] = raw[k];
        memcpy(raw, srt, m * sizeof(Edge));
    }
    for (k = 0; k < m; k++) pos[ov[ix(raw[k].i, raw[k].j, n)]]++;     /* counting sort, descending */
    {
        size_t acc = 0;
        int o;
        for (o = maxo; o >= 1; o--) { size_t c = pos[o]; pos[o] = acc; acc += c; }
    }
    for (k = 0; k < m; k++) srt[pos[ov[ix(raw[k].i, raw[k].j, n)]]++] = raw[k];
    for (i = 0; i < n; i++) { par[i] = i; sz[i] = 1; }
    for (e = 0; e < m && accepted < n - 1; e++) {
        int a = srt[e].i, b = srt[e].j, ra, rb;
        if (nxt[a] >= 0 || prv[b] >= 0) continue;
        ra = uf_find(par, a); rb = uf_find(par, b);
        if (ra == rb) continue;                              /* would close a cycle */
        if (sz[ra] < sz[rb]) { int t = ra; ra = rb; rb = t; }
        par[rb] = ra; sz[ra] += sz[rb];
        nxt[a] = b; prv[b] = a;
        sum += ov[ix(a, b, n)];
        if (trace) { trace[accepted].i = a; trace[accepted].j = b; }
        accepted++;
    }
    if (ntrace) *ntrace = accepted;
    free(raw); free(srt); free(pos); free(par); free(sz);
    return sum;
}

static uc *build_from_links(const Set *S, const uint16_t *ov, const int *nxt, const int *prv,
                            ll sum, size_t *outlen) {
    int n = S->n, h;
    size_t L = S->total - (size_t)sum, w = 0;
    uc *buf = (uc *)malloc(L ? L : 1);
    if (!buf) die("out of memory");
    for (h = 0; h < n; h++) {
        int cur;
        if (prv[h] >= 0) continue;
        memcpy(buf + w, S->s[h], (size_t)S->len[h]);
        w += (size_t)S->len[h];
        cur = h;
        while (nxt[cur] >= 0) {
            int j = nxt[cur], o = ov[ix(cur, j, n)];
            memcpy(buf + w, S->s[j] + o, (size_t)(S->len[j] - o));
            w += (size_t)(S->len[j] - o);
            cur = j;
        }
    }
    *outlen = w;
    return buf;
}

/* ------------------------------ exact solver (Held-Karp) ------------------------------ */
static int exact_scs(const Set *S, const uint16_t *ov, uc **out, size_t *outlen) {
    int n = S->n, last, nx, pos, cur;
    size_t full, mask, w = 0;
    int *dp, *order, best = -1, bestlast = 0;
    uc *buf;
    if (n == 0) { *out = (uc *)malloc(1); *outlen = 0; return 0; }
    if (n > EXACT_LIMIT) return -1;
    full = ((size_t)1 << n) - 1;
    dp = (int *)malloc(((size_t)1 << n) * (size_t)n * sizeof(int));
    order = (int *)malloc((size_t)n * sizeof(int));
    if (!dp || !order) die("out of memory");
    for (mask = 0; mask <= full; mask++)
        for (last = 0; last < n; last++) dp[mask * (size_t)n + (size_t)last] = -1;
    for (last = 0; last < n; last++) dp[((size_t)1 << last) * (size_t)n + (size_t)last] = 0;
    for (mask = 1; mask <= full; mask++)
        for (last = 0; last < n; last++) {
            int v = dp[mask * (size_t)n + (size_t)last];
            if (v < 0) continue;
            for (nx = 0; nx < n; nx++) {
                size_t ni;
                int nv;
                if ((mask >> nx) & 1u) continue;
                nv = v + ov[ix(last, nx, n)];
                ni = (mask | ((size_t)1 << nx)) * (size_t)n + (size_t)nx;
                if (nv > dp[ni]) dp[ni] = nv;
            }
        }
    for (last = 0; last < n; last++)
        if (dp[full * (size_t)n + (size_t)last] > best) { best = dp[full * (size_t)n + (size_t)last]; bestlast = last; }
    mask = full; cur = bestlast;
    for (pos = n - 1; pos >= 0; pos--) {
        order[pos] = cur;
        if (pos == 0) break;
        {
            size_t pm = mask ^ ((size_t)1 << cur);
            int p, found = -1;
            for (p = 0; p < n && found < 0; p++)
                if (((pm >> p) & 1u) && dp[pm * (size_t)n + (size_t)p] >= 0 &&
                    dp[pm * (size_t)n + (size_t)p] + (int)ov[ix(p, cur, n)] == dp[mask * (size_t)n + (size_t)cur])
                    found = p;
            cur = found; mask = pm;
        }
    }
    buf = (uc *)malloc(S->total - (size_t)best ? S->total - (size_t)best : 1);
    if (!buf) die("out of memory");
    memcpy(buf, S->s[order[0]], (size_t)S->len[order[0]]);
    w = (size_t)S->len[order[0]];
    for (pos = 1; pos < n; pos++) {
        int o = ov[ix(order[pos - 1], order[pos], n)];
        memcpy(buf + w, S->s[order[pos]] + o, (size_t)(S->len[order[pos]] - o));
        w += (size_t)(S->len[order[pos]] - o);
    }
    *out = buf; *outlen = w;
    free(dp); free(order);
    return (int)(S->total) - best;
}

/* ------------------------------ independent references for tests ------------------------------ */
static int brute_n, brute_best;
static int brute_nov[8][8];
static void brute_rec(int used, int last, int sumov) {
    int j;
    if (used == (1 << brute_n) - 1) { if (sumov > brute_best) brute_best = sumov; return; }
    for (j = 0; j < brute_n; j++)
        if (!((used >> j) & 1)) brute_rec(used | (1 << j), j, sumov + (last < 0 ? 0 : brute_nov[last][j]));
}
/* optimal length by trying all permutations (n <= 7), overlaps computed naively */
static int brute_perm_scs(const Set *S) {
    int i, j;
    brute_n = S->n; brute_best = 0;
    for (i = 0; i < S->n; i++)
        for (j = 0; j < S->n; j++)
            brute_nov[i][j] = (i == j) ? 0 : naive_overlap(S->s[i], S->len[i], S->s[j], S->len[j]);
    brute_rec(0, -1, 0);
    return (int)S->total - brute_best;
}

typedef struct { int n; ll total, mn, mx, leaves, cap; int capped; } Explore;
/* explore EVERY tie-breaking of the greedy (n <= 8): min and max resulting length */
static void explore_rec(Explore *E, const uint16_t *ov, const int *nxt, const int *prv, const int *comp, ll sum) {
    int n = E->n, i, j, best = 0;
    if (E->leaves >= E->cap) { E->capped = 1; return; }
    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++)
            if (i != j && nxt[i] < 0 && prv[j] < 0 && comp[i] != comp[j] && (int)ov[ix(i, j, n)] > best)
                best = ov[ix(i, j, n)];
    if (best == 0) {
        ll len = E->total - sum;
        E->leaves++;
        if (len < E->mn) E->mn = len;
        if (len > E->mx) E->mx = len;
        return;
    }
    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++)
            if (i != j && nxt[i] < 0 && prv[j] < 0 && comp[i] != comp[j] && (int)ov[ix(i, j, n)] == best) {
                int nx[8], pv[8], cp[8], k, from = comp[j], to = comp[i];
                for (k = 0; k < n; k++) { nx[k] = nxt[k]; pv[k] = prv[k]; cp[k] = comp[k] == from ? to : comp[k]; }
                nx[i] = j; pv[j] = i;
                explore_rec(E, ov, nx, pv, cp, sum + best);
                if (E->capped) return;
            }
}
static void explore_ties(const Set *S, const uint16_t *ov, ll *mn, ll *mx, int *capped) {
    Explore E;
    int nx[8], pv[8], cp[8], k;
    E.n = S->n; E.total = (ll)S->total; E.mn = (ll)1 << 60; E.mx = 0; E.leaves = 0; E.cap = 20000; E.capped = 0;
    for (k = 0; k < S->n; k++) { nx[k] = -1; pv[k] = -1; cp[k] = k; }
    explore_rec(&E, ov, nx, pv, cp, 0);
    *mn = E.mn; *mx = E.mx; *capped = E.capped;
}

/* Independent check that a merge sequence is a legal execution of the greedy algorithm: at every step
 * the chosen pair is valid and has the maximum overlap among all valid pairs; at the end no pair with
 * positive overlap is left. O(n^3) -- for tests only. */
static int verify_trace(const Set *S, const uint16_t *ov, const Edge *trace, int nt) {
    int n = S->n, step, i, j, k, ok = 1;
    int *nxt = (int *)malloc((size_t)n * sizeof(int)), *prv = (int *)malloc((size_t)n * sizeof(int));
    int *comp = (int *)malloc((size_t)n * sizeof(int));
    if (!nxt || !prv || !comp) die("out of memory");
    for (k = 0; k < n; k++) { nxt[k] = -1; prv[k] = -1; comp[k] = k; }
    for (step = 0; step <= nt && ok; step++) {
        int best = 0;
        for (i = 0; i < n; i++)
            for (j = 0; j < n; j++)
                if (i != j && nxt[i] < 0 && prv[j] < 0 && comp[i] != comp[j] && (int)ov[ix(i, j, n)] > best)
                    best = ov[ix(i, j, n)];
        if (step == nt) { if (best != 0) ok = 0; break; }      /* nothing positive may remain */
        i = trace[step].i; j = trace[step].j;
        if (nxt[i] >= 0 || prv[j] >= 0 || comp[i] == comp[j] || (int)ov[ix(i, j, n)] != best || best == 0) { ok = 0; break; }
        {
            int from = comp[j], to = comp[i];
            nxt[i] = j; prv[j] = i;
            for (k = 0; k < n; k++) if (comp[k] == from) comp[k] = to;
        }
    }
    free(nxt); free(prv); free(comp);
    return ok;
}

/* ------------------------------ stdin mode ------------------------------ */
static int run_stdin(void) {
    size_t cap = 1u << 16, blen = 0, r, pos = 0;
    char *buf = (char *)malloc(cap);
    long n_in, i;
    Set raw, S;
    uint16_t *ov;
    int *nxt, *prv;
    ll sum;
    uc *g, *o = NULL;
    size_t glen, olen = 0;
    int optlen;

    if (!buf) die("out of memory");
    while ((r = fread(buf + blen, 1, cap - blen, stdin)) > 0) {
        blen += r;
        if (blen == cap) { cap <<= 1; buf = (char *)realloc(buf, cap); if (!buf) die("out of memory"); }
    }
    buf = (char *)realloc(buf, blen + 1);                 /* room for a terminator */
    if (!buf) die("out of memory");
    buf[blen] = '\0';
    {   /* first token: n */
        char *end;
        while (pos < blen && (buf[pos] == ' ' || buf[pos] == '\n' || buf[pos] == '\r' || buf[pos] == '\t')) pos++;
        if (pos >= blen) die("invalid input");
        n_in = strtol(buf + pos, &end, 10);
        if (end == buf + pos || n_in < 0) die("invalid input");
        pos = (size_t)(end - buf);
    }
    if ((size_t)n_in > blen) die("invalid input");
    {
        size_t total = 0, p2 = pos, *st = (size_t *)malloc(((size_t)n_in + 1) * sizeof(size_t)), *ln = (size_t *)malloc(((size_t)n_in + 1) * sizeof(size_t));
        if (!st || !ln) die("out of memory");
        for (i = 0; i < n_in; i++) {
            while (p2 < blen && (buf[p2] == ' ' || buf[p2] == '\n' || buf[p2] == '\r' || buf[p2] == '\t')) p2++;
            if (p2 >= blen) die("fewer strings than announced");
            st[i] = p2;
            while (p2 < blen && !(buf[p2] == ' ' || buf[p2] == '\n' || buf[p2] == '\r' || buf[p2] == '\t')) p2++;
            ln[i] = p2 - st[i];
            total += ln[i];
        }
        set_init(&raw, (int)n_in, total);
        total = 0;
        for (i = 0; i < n_in; i++) {
            raw.s[i] = raw.pool + total;
            raw.len[i] = (int)ln[i];
            memcpy(raw.s[i], buf + st[i], ln[i]);
            total += ln[i];
        }
        free(st); free(ln);
    }
    free(buf);
    make_substring_free(&raw, &S);
    ov = compute_overlaps(&S);
    nxt = (int *)malloc((S.n ? (size_t)S.n : 1) * sizeof(int));
    prv = (int *)malloc((S.n ? (size_t)S.n : 1) * sizeof(int));
    if (!nxt || !prv) die("out of memory");
    sum = greedy_links(S.n, ov, 0, NULL, NULL, NULL, nxt, prv);
    g = build_from_links(&S, ov, nxt, prv, sum, &glen);
    printf("strings: %d given, %d after removing duplicates / contained strings\n", raw.n, S.n);
    printf("GREEDY  length=%zu  ", glen); fwrite(g, 1, glen, stdout); printf("\n");
    optlen = exact_scs(&S, ov, &o, &olen);
    if (optlen >= 0) {
        printf("OPTIMAL length=%zu  ", olen); fwrite(o, 1, olen, stdout); printf("\n");
        printf("greedy/optimal = %.4f\n", olen ? (double)glen / (double)olen : 1.0);
    } else {
        printf("OPTIMAL not computed (needs <= %d strings after preprocessing)\n", EXACT_LIMIT);
    }
    free(g); free(o); free(nxt); free(prv); free(ov);
    set_free(&raw); set_free(&S);
    return 0;
}

/* ------------------------------ random instances for tests ------------------------------ */
static void set_from_buf(Set *S, uc (*buf)[64], const int *bl, int n) {
    size_t tot = 0;
    int i;
    for (i = 0; i < n; i++) tot += (size_t)bl[i];
    set_init(S, n, tot);
    tot = 0;
    for (i = 0; i < n; i++) {
        S->s[i] = S->pool + tot; S->len[i] = bl[i];
        memcpy(S->s[i], buf[i], (size_t)bl[i]);
        tot += (size_t)bl[i];
    }
}

static void gen_raw(int kind, Set *S) {
    static uc buf[128][64];
    int bl[128], n = 0, i, p, sigma = 2;
    uc src[256];
    int sl = 0, lmin = 2, lmax = 10;

    switch (kind) {
    case 0: sigma = 2; n = (int)rr(1, 8);
        for (i = 0; i < n; i++) { bl[i] = (int)rr(1, 6); for (p = 0; p < bl[i]; p++) buf[i][p] = (uc)('a' + rr(0, sigma - 1)); }
        break;
    case 1: sigma = 3; n = (int)rr(1, 10);
        for (i = 0; i < n; i++) { bl[i] = (int)rr(2, 8); for (p = 0; p < bl[i]; p++) buf[i][p] = (uc)('a' + rr(0, sigma - 1)); }
        break;
    case 2: sigma = 4; n = (int)rr(1, 12);
        for (i = 0; i < n; i++) { bl[i] = (int)rr(3, 10); for (p = 0; p < bl[i]; p++) buf[i][p] = (uc)('a' + rr(0, sigma - 1)); }
        break;
    case 5: {                                   /* a b^k, b^(k+1), b^k c plus noise: the tie-breaking trap */
        int k = (int)rr(2, 6), extra = (int)rr(0, 3);
        n = 3;
        bl[0] = k + 1; buf[0][0] = 'a'; for (p = 1; p <= k; p++) buf[0][p] = 'b';
        bl[1] = k + 1; for (p = 0; p <= k; p++) buf[1][p] = 'b';
        bl[2] = k + 1; for (p = 0; p < k; p++) buf[2][p] = 'b'; buf[2][k] = 'c';
        for (i = 0; i < extra; i++) {
            bl[n] = (int)rr(1, 4);
            for (p = 0; p < bl[n]; p++) buf[n][p] = (uc)('a' + rr(0, 2));
            n++;
        }
        break;
    }
    default: {                                  /* substrings of a hidden source string */
        if (kind == 3) { sigma = (int)rr(2, 4); sl = (int)rr(8, 40); n = (int)rr(2, 12); lmin = 2; lmax = 10; }
        else if (kind == 4) { sigma = 2; sl = (int)rr(10, 40); n = (int)rr(2, 12); lmin = 2; lmax = 10; }
        else if (kind == 6) { sigma = (int)rr(2, 3); sl = (int)rr(15, 40); n = (int)rr(9, 13); lmin = 4; lmax = 9; }
        else { sigma = (int)rr(2, 4); sl = (int)rr(60, 200); n = (int)rr(20, 80); lmin = 5; lmax = 20; }
        if (kind == 4) {                        /* periodic source: many ties and repeats */
            int per = (int)rr(1, 3), q;
            uc unit[3];
            for (q = 0; q < per; q++) unit[q] = (uc)('a' + rr(0, 1));
            for (p = 0; p < sl; p++) src[p] = unit[p % per];
        } else {
            for (p = 0; p < sl; p++) src[p] = (uc)('a' + rr(0, sigma - 1));
        }
        for (i = 0; i < n; i++) {
            int L = (int)rr(lmin, lmax < sl ? lmax : sl), at;
            at = (int)rr(0, sl - L);
            bl[i] = L;
            memcpy(buf[i], src + at, (size_t)L);
        }
        break;
    }
    }
    set_from_buf(S, buf, bl, n);
}

typedef struct { const char *name; int n; const char *s[6]; int opt; ll gmin, gmax; } Fixed;
static const Fixed fixed_cases[] = {
    {"single string",               1, {"hello"}, 5, 5, 5},
    {"cyclic ab bc ca",             3, {"ab", "bc", "ca"}, 4, 4, 4},
    {"chain abcd bcde cdef",        3, {"abcd", "bcde", "cdef"}, 6, 6, 6},
    {"duplicates and substrings",   5, {"abc", "abc", "b", "bc", "xyz"}, 6, 6, 6},
    {"aab aba baa",                 3, {"aab", "aba", "baa"}, 5, 5, 5},
    {"disjoint strings",            3, {"aaa", "bbb", "ccc"}, 9, 9, 9},
    {"tie trap k=5: ab^5 b^6 b^5c", 3, {"abbbbb", "bbbbbb", "bbbbbc"}, 8, 8, 13},
    {"tie trap k=2: abb bbb bbc",   3, {"abb", "bbb", "bbc"}, 5, 5, 7},
};

static int run_tests(ll N, ull seed) {
    ll t, bad = 0, fixed_bad = 0;
    ll n_ovbad = 0, n_greedybad = 0, n_exactbad = 0, n_dpvsbrute = 0, n_brutecases = 0, n_ties = 0, n_tiecapped = 0;
    ll n_tracebad = 0, n_exact = 0, n_subopt = 0, n_violate = 0, n_tiebad = 0, n_tiesubopt = 0;
    double maxr = 1.0, maxr_tie = 1.0;
    size_t kf;

    printf("== Fixed cases ==\n");
    for (kf = 0; kf < sizeof(fixed_cases) / sizeof(fixed_cases[0]); kf++) {
        const Fixed *c = &fixed_cases[kf];
        Set raw, S;
        uint16_t *ov;
        uc *o = NULL;
        size_t olen = 0;
        int i, opt, ok = 1;
        ll mn, mx;
        int capped;
        set_init(&raw, c->n, 64);
        {
            size_t tot = 0;
            for (i = 0; i < c->n; i++) {
                raw.s[i] = raw.pool + tot; raw.len[i] = (int)strlen(c->s[i]);
                memcpy(raw.s[i], c->s[i], (size_t)raw.len[i]); tot += (size_t)raw.len[i];
            }
        }
        make_substring_free(&raw, &S);
        ov = compute_overlaps(&S);
        opt = exact_scs(&S, ov, &o, &olen);
        explore_ties(&S, ov, &mn, &mx, &capped);
        if (opt != c->opt || (int)olen != opt || !all_contained(&raw, o, olen)) ok = 0;
        if (mn != c->gmin || mx != c->gmax || capped) ok = 0;
        printf("  [%s] %-30s optimal %2d (expect %2d)  greedy over all tie-breaks %2lld..%2lld (expect %2lld..%2lld)\n",
               ok ? "ok" : "FAIL", c->name, opt, c->opt, mn, mx, c->gmin, c->gmax);
        if (!ok) fixed_bad++;
        free(o); free(ov); set_free(&raw); set_free(&S);
    }

    printf("== Random validation: %lld instances, seed %llu ==\n", N, seed);
    rs_global = seed ? seed : 88172645463325252ULL;
    for (t = 0; t < N; t++) {
        Set raw, S;
        uint16_t *ov;
        int *nxt, *prv, i, j, n, run, ok = 1, opt = -1, nt = 0;
        Edge *trace;
        ll glen[3] = {0, 0, 0};
        uc *o = NULL;
        size_t olen = 0;

        gen_raw((int)(t % 8), &raw);
        make_substring_free(&raw, &S);
        n = S.n;
        ov = compute_overlaps(&S);
        for (i = 0; i < n; i++)
            for (j = 0; j < n; j++)
                if (i != j && ov[ix(i, j, n)] != naive_overlap(S.s[i], S.len[i], S.s[j], S.len[j])) { n_ovbad++; ok = 0; i = n; break; }
        nxt = (int *)malloc((size_t)n * sizeof(int));
        prv = (int *)malloc((size_t)n * sizeof(int));
        trace = (Edge *)malloc((size_t)n * sizeof(Edge));
        for (run = 0; run < 3; run++) {              /* default tie order + two random tie orders */
            ll sum = greedy_links(n, ov, run == 0 ? 0 : (rng(&rs_global) | 1ULL), NULL, trace, &nt, nxt, prv);
            if (!verify_trace(&S, ov, trace, nt)) { n_tracebad++; ok = 0; }
            size_t len;
            uc *g = build_from_links(&S, ov, nxt, prv, sum, &len);
            glen[run] = (ll)len;
            if (!all_contained(&raw, g, len) || len != S.total - (size_t)sum) { n_greedybad++; ok = 0; }
            free(g);
        }
        if (n <= 12) {
            opt = exact_scs(&S, ov, &o, &olen);
            n_exact++;
            if ((size_t)opt != olen || !all_contained(&raw, o, olen)) { n_exactbad++; ok = 0; }
            for (run = 0; run < 3; run++) {
                double r = (double)glen[run] / (double)opt;
                if (glen[run] < opt) { n_exactbad++; ok = 0; }
                if (r > maxr) maxr = r;
                if (r > 2.0) n_violate++;
            }
            if (glen[0] > opt) n_subopt++;
            if (n <= 7) {
                n_brutecases++;
                if (brute_perm_scs(&S) != opt) { n_dpvsbrute++; ok = 0; }
            }
            if (n <= 6) {
                ll mn, mx;
                int capped;
                explore_ties(&S, ov, &mn, &mx, &capped);
                if (!capped) {
                    n_ties++;
                    if (mn < opt) { n_tiebad++; ok = 0; }
                    for (run = 0; run < 3; run++) if (glen[run] < mn || glen[run] > mx) { n_tiebad++; ok = 0; }
                    if ((double)mx / (double)opt > maxr_tie) maxr_tie = (double)mx / (double)opt;
                    if ((double)mx / (double)opt > 2.0) n_violate++;
                    if (mx > opt) n_tiesubopt++;
                } else n_tiecapped++;
            }
        }
        if (!ok && ++bad <= 5) {
            printf("FAILED instance %lld (kind %lld): n=%d\n", t, t % 8, n);
            for (i = 0; i < raw.n && i < 12; i++) printf("   %.*s\n", raw.len[i], (const char *)raw.s[i]);
        }
        free(o); free(nxt); free(prv); free(trace); free(ov); set_free(&raw); set_free(&S);
    }
    printf("  KMP overlaps differ from naive overlaps ........ %lld\n", n_ovbad);
    printf("  greedy outputs invalid / inconsistent ........... %lld\n", n_greedybad);
    printf("  greedy merge sequences that are not legal greedy  %lld  (each step re-checked: max overlap among valid pairs)\n", n_tracebad);
    printf("  exact solver invalid or better than possible .... %lld\n", n_exactbad);
    printf("  exact DP != brute-force permutations ............ %lld  (of %lld instances with n <= 7)\n", n_dpvsbrute, n_brutecases);
    printf("  greedy outside [min,max] over all tie-breaks .... %lld  (of %lld instances with n <= 6, %lld skipped by cap)\n", n_tiebad, n_ties, n_tiecapped);
    printf("  -- insight --\n");
    printf("  instances solved exactly: %lld; default greedy not optimal on %lld (%.1f%%)\n", n_exact, n_subopt, n_exact ? 100.0 * (double)n_subopt / (double)n_exact : 0.0);
    printf("  worst tie-breaking is longer than OPT on %lld of %lld exhaustively explored instances\n", n_tiesubopt, n_ties);
    printf("  max greedy/OPT seen: %.4f (3 runs per instance), %.4f (worst tie-break, n <= 6)\n", maxr, maxr_tie);
    printf("  instances with greedy > 2*OPT: %lld\n", n_violate);
    if (bad || fixed_bad) { printf("RESULT: FAILED\n"); return 1; }
    printf("RESULT: ALL TESTS PASSED\n");
    return 0;
}

/* ------------------------------ the arXiv:2609.01365 family ------------------------------ */
static uc bsym(int i) {                          /* i-th symbol distinct from 0, a, x, y */
    int c, cnt = 0;
    for (c = 1; c < 256; c++) {
        if (c == 'a' || c == 'x' || c == 'y') continue;
        if (cnt == i) return (uc)c;
        cnt++;
    }
    die("t too large (max 252)");
    return 0;
}
static int put_X(uc *b, int m) {                 /* X_m = (yx)^(m/2) for even m, x(yx)^((m-1)/2) for odd m */
    int p = 0, r;
    if (m % 2 == 1) b[p++] = 'x';
    for (r = 0; r < m / 2; r++) { b[p++] = 'y'; b[p++] = 'x'; }
    return p;
}
static void add_windows(Set *S, int *w, const uc *base, int L, int k) {   /* all k-windows of the cyclic word */
    int r, p;
    for (r = 0; r < L; r++) {
        uc *dst = S->pool + (size_t)(*w) * (size_t)k;
        for (p = 0; p < k; p++) dst[p] = base[(r + p) % L];
        S->s[*w] = dst; S->len[*w] = k;
        (*w)++;
    }
}
static void build_paper(int s, int t, Set *S) {
    int k = 2 * s, per = 4 * s + 2, n = t * per, i, w = 0, p;
    uc base[512];
    set_init(S, n, (size_t)n * (size_t)k);
    for (i = 0; i < t; i++) {
        uc b = bsym(i);
        p = 0; base[p++] = 'x'; base[p++] = 'a'; p += put_X(base + p, s - 3); base[p++] = b;
        base[p++] = 'x'; base[p++] = 'a'; p += put_X(base + p, s - 3); base[p++] = 'y';
        if (p != k) die("internal error: |U| != k");
        add_windows(S, &w, base, k, k);                              /* U_i: k windows   */
        p = 0; base[p++] = 'x'; base[p++] = 'a'; p += put_X(base + p, s - 3); base[p++] = b;
        if (p != s) die("internal error: |V| != s");
        add_windows(S, &w, base, s, k);                              /* V_i: s windows   */
        p = 0; base[p++] = 'x'; base[p++] = 'a'; p += put_X(base + p, s - 1); base[p++] = b;
        if (p != s + 2) die("internal error: |W| != s+2");
        add_windows(S, &w, base, s + 2, k);                          /* W_i: s+2 windows */
    }
}

static size_t g_kml;
static int cmp_kmer(const void *a, const void *b) { return memcmp(*(const uc *const *)a, *(const uc *const *)b, g_kml); }
static int kmer_id(const uc **u, int nu, const uc *key, size_t kl) {
    int lo = 0, hi = nu - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2, c = memcmp(u[mid], key, kl);
        if (c == 0) return mid;
        if (c < 0) lo = mid + 1; else hi = mid - 1;
    }
    return -1;
}

/* Strings of equal length k: an Eulerian trail in the graph (nodes = (k-1)-mers, edges = strings)
 * gives a superstring of length n + k - 1, which is optimal because n distinct strings need n + k - 1.
 * Returns 1 and the superstring if such a trail exists. */
static int euler_superstring(const Set *S, uc **out, size_t *outlen) {
    int n = S->n, k = S->len[0], e, nu = 0, v, np = 0, sp = 0, plus = 0, minus = 0, sv = -1;
    const uc **pt, **u;
    int *from, *to, *outd, *ind, *start, *adj, *ptr, *stk_v, *stk_e, *path;
    uc *buf;
    for (e = 0; e < n; e++) if (S->len[e] != k) return 0;
    if (k < 2) return 0;
    pt = (const uc **)malloc((size_t)(2 * n) * sizeof(uc *));
    u = (const uc **)malloc((size_t)(2 * n) * sizeof(uc *));
    from = (int *)malloc((size_t)n * sizeof(int)); to = (int *)malloc((size_t)n * sizeof(int));
    if (!pt || !u || !from || !to) die("out of memory");
    for (e = 0; e < n; e++) { pt[2 * e] = S->s[e]; pt[2 * e + 1] = S->s[e] + 1; }
    g_kml = (size_t)(k - 1);
    qsort(pt, (size_t)(2 * n), sizeof(uc *), cmp_kmer);
    for (e = 0; e < 2 * n; e++)
        if (nu == 0 || memcmp(u[nu - 1], pt[e], g_kml) != 0) u[nu++] = pt[e];
    for (e = 0; e < n; e++) { from[e] = kmer_id(u, nu, S->s[e], g_kml); to[e] = kmer_id(u, nu, S->s[e] + 1, g_kml); }
    outd = (int *)calloc((size_t)nu, sizeof(int)); ind = (int *)calloc((size_t)nu, sizeof(int));
    start = (int *)calloc((size_t)nu + 1, sizeof(int)); ptr = (int *)malloc((size_t)nu * sizeof(int));
    adj = (int *)malloc((size_t)n * sizeof(int));
    stk_v = (int *)malloc(((size_t)n + 1) * sizeof(int)); stk_e = (int *)malloc(((size_t)n + 1) * sizeof(int));
    path = (int *)malloc((size_t)n * sizeof(int));
    if (!outd || !ind || !start || !ptr || !adj || !stk_v || !stk_e || !path) die("out of memory");
    for (e = 0; e < n; e++) { outd[from[e]]++; ind[to[e]]++; }
    for (v = 0; v < nu; v++) {
        int d = outd[v] - ind[v];
        if (d == 1) { plus++; sv = v; } else if (d == -1) minus++; else if (d != 0) plus += 1000;
    }
    {
        int ok = (plus <= 1 && minus <= 1 && plus == minus), r = 0;
        if (ok) {
            for (v = 0; v < nu; v++) { start[v] = r; r += outd[v]; ptr[v] = start[v]; }
            start[nu] = r;
            for (e = 0; e < n; e++) adj[ptr[from[e]]++] = e;
            for (v = 0; v < nu; v++) ptr[v] = start[v];
            if (sv < 0) sv = from[0];
            stk_v[0] = sv; stk_e[0] = -1; sp = 1;
            while (sp > 0) {
                int x = stk_v[sp - 1];
                if (ptr[x] < start[x + 1]) { int ed = adj[ptr[x]++]; stk_v[sp] = to[ed]; stk_e[sp] = ed; sp++; }
                else { int ed = stk_e[sp - 1]; sp--; if (ed >= 0) path[np++] = ed; }
            }
            if (np != n) ok = 0;
        }
        if (ok) {
            int a, b;
            for (a = 0, b = n - 1; a < b; a++, b--) { int tmp = path[a]; path[a] = path[b]; path[b] = tmp; }
            buf = (uc *)malloc((size_t)n + (size_t)k);
            if (!buf) die("out of memory");
            memcpy(buf, S->s[path[0]], (size_t)k);
            for (a = 1; a < n; a++) buf[k - 1 + a] = S->s[path[a]][k - 1];
            *out = buf; *outlen = (size_t)n + (size_t)k - 1;
        }
        free(pt); free(u); free(from); free(to); free(outd); free(ind); free(start); free(ptr);
        free(adj); free(stk_v); free(stk_e); free(path);
        return ok;
    }
}

static int run_paper(int s, int t, int R) {
    int k, n, nr;
    Set raw, S;
    uint16_t *ov;
    int *nxt, *prv;
    ll sum, claimed_g, claimed_o, gmin = (ll)1 << 60, gmax = 0;
    uc *g, *e = NULL;
    size_t glen, elen = 0;
    if (s < 5 || t < 1 || t > 252) die("need s >= 5 (k = 2s >= 10) and 1 <= t <= 252");
    k = 2 * s;
    build_paper(s, t, &raw);
    make_substring_free(&raw, &S);
    n = S.n;
    printf("Instance of arXiv:2609.01365: s=%d (k=%d), t=%d groups -> %d strings of length %d (%d distinct)\n",
           s, k, t, raw.n, k, n);
    ov = compute_overlaps(&S);
    nxt = (int *)malloc((size_t)n * sizeof(int)); prv = (int *)malloc((size_t)n * sizeof(int));
    sum = greedy_links(n, ov, 0, NULL, NULL, NULL, nxt, prv);
    g = build_from_links(&S, ov, nxt, prv, sum, &glen);
    printf("greedy (index tie order): length %zu, valid superstring: %s\n", glen, all_contained(&raw, g, glen) ? "yes" : "NO");
    free(g);
    {   /* The paper's tie-breaking: among equal overlaps prefer (a) k-1: windows of the same base word,
         * (b) s-3: V_i -> U_i of the same group, (c) 1: U_i -> V_j of different groups. */
        unsigned char *prio = (unsigned char *)malloc((size_t)n * (size_t)n + 1);
        int *grp = (int *)malloc((size_t)n * sizeof(int)), *typ = (int *)malloc((size_t)n * sizeof(int));
        int *base = (int *)malloc((size_t)n * sizeof(int)), a, b, per = 4 * s + 2;
        Edge *trace = (Edge *)malloc((size_t)n * sizeof(Edge));
        int nt = 0, legal = -1;
        ll sm;
        uc *g2;
        size_t g2len;
        if (!prio || !grp || !typ || !base || !trace) die("out of memory");
        for (a = 0; a < n; a++) {                    /* locate each string in the construction order */
            int r;
            for (r = 0; r < raw.n; r++)
                if (memcmp(raw.s[r], S.s[a], (size_t)k) == 0) break;
            if (r == raw.n) die("internal error: string not found");
            grp[a] = r / per;
            r %= per;
            typ[a] = r < k ? 0 : (r < k + s ? 1 : 2);                 /* 0 = U, 1 = V, 2 = W */
            base[a] = grp[a] * 3 + typ[a];
        }
        memset(prio, 1, (size_t)n * (size_t)n);
        for (a = 0; a < n; a++)
            for (b = 0; b < n; b++) {
                int o = ov[ix(a, b, n)];
                if (a == b || o == 0) continue;
                if (o == k - 1 && base[a] == base[b]) prio[ix(a, b, n)] = 0;
                else if (o == s - 3 && typ[a] == 1 && typ[b] == 0 && grp[a] == grp[b]) prio[ix(a, b, n)] = 0;
                else if (o == 1 && typ[a] == 0 && typ[b] == 1 && grp[a] != grp[b]) prio[ix(a, b, n)] = 0;
            }
        sm = greedy_links(n, ov, 0, prio, trace, &nt, nxt, prv);
        g2 = build_from_links(&S, ov, nxt, prv, sm, &g2len);
        if (n <= 700) legal = verify_trace(&S, ov, trace, nt);
        printf("greedy (the paper's tie-breaking): length %zu, valid superstring: %s, legal greedy run: %s\n",
               g2len, all_contained(&raw, g2, g2len) ? "yes" : "NO",
               legal < 0 ? "not re-checked (n too large)" : (legal ? "yes (re-checked step by step)" : "NO"));
        {
            ll h2[512];
            int z, heads = 0;
            memset(h2, 0, sizeof h2);
            for (z = 0; z < n; z++) { if (nxt[z] >= 0) h2[ov[ix(z, nxt[z], n)]]++; if (prv[z] < 0) heads++; }
            printf("  merges by overlap (overlap:count):");
            for (z = k; z >= 0; z--) if (h2[z]) printf(" %d:%lld", z, h2[z]);
            printf("  (%d chains left, joined with overlap 0)\n", heads);
        }
        glen = g2len;                                 /* ratio below uses this adversarial run */
        free(g2); free(prio); free(grp); free(typ); free(base); free(trace);
    }
    for (nr = 0; nr < R; nr++) {                     /* does the tie-breaking matter? */
        ll sm = greedy_links(n, ov, (ull)nr * 7919ULL + 12345ULL, NULL, NULL, NULL, nxt, prv);
        ll len = (ll)S.total - sm;
        if (len < gmin) gmin = len;
        if (len > gmax) gmax = len;
    }
    if (R > 0) printf("greedy over %d random tie-breakings: shortest %lld, longest %lld\n", R, gmin, gmax);
    claimed_g = (ll)t * (9LL * s + 1) + 1;
    claimed_o = (ll)t * (4LL * s + 2) + k - 1;
    printf("paper's claims: greedy length t(9s+1)+1 = %lld, optimum t(4s+2)+k-1 = %lld\n", claimed_g, claimed_o);
    printf("optimum lower bound (n distinct strings need n+k-1 characters): %d\n", n + k - 1);
    if (euler_superstring(&S, &e, &elen)) {
        int valid = all_contained(&raw, e, elen);
        printf("explicit Eulerian-trail superstring: length %zu, contains every string: %s\n", elen, valid ? "yes" : "NO");
        if (valid && (ll)elen == (ll)n + k - 1) {
            printf("=> OPT = %zu exactly (certified).  greedy/OPT = %.4f   [paper's limit (9k+2)/(4k+4) = %.4f]\n",
                   elen, (double)glen / (double)elen, (9.0 * k + 2) / (4.0 * k + 4));
            printf("=> greedy %s 2*OPT = %zu on this instance\n", (ll)glen > 2 * (ll)elen ? "EXCEEDS" : "does not exceed", 2 * elen);
        }
        free(e);
    } else {
        printf("no Eulerian trail over the (k-1)-mer graph: optimum not certified by this method\n");
    }
    free(nxt); free(prv); free(ov); set_free(&raw); set_free(&S);
    return 0;
}

/* ------------------------------ benchmark ------------------------------ */
static int run_bench(void) {
    int sizes[] = {500, 1000, 2000, 4000}, k;
    rs_global = 424242ULL;
    printf("%-8s %-10s %-12s %-12s %-12s\n", "n'", "M", "overlaps(s)", "greedy(s)", "ns/(n*M)");
    for (k = 0; k < 4; k++) {
        int n = sizes[k], i, p, sl = n * 4;
        uc *src = (uc *)malloc((size_t)sl);
        Set raw, S;
        clock_t t0, t1, t2;
        uint16_t *ov;
        int *nxt, *prv;
        ll sum;
        size_t tot = 0;
        for (p = 0; p < sl; p++) src[p] = (uc)('a' + rr(0, 3));
        set_init(&raw, n, (size_t)n * 40);
        for (i = 0; i < n; i++) {
            int L = (int)rr(15, 40), at = (int)rr(0, sl - L);
            raw.s[i] = raw.pool + tot; raw.len[i] = L;
            memcpy(raw.s[i], src + at, (size_t)L); tot += (size_t)L;
        }
        raw.total = tot;
        make_substring_free(&raw, &S);
        t0 = clock();
        ov = compute_overlaps(&S);
        t1 = clock();
        nxt = (int *)malloc((size_t)S.n * sizeof(int)); prv = (int *)malloc((size_t)S.n * sizeof(int));
        sum = greedy_links(S.n, ov, 0, NULL, NULL, NULL, nxt, prv);
        t2 = clock();
        printf("%-8d %-10zu %-12.3f %-12.3f %-12.2f\n", S.n, S.total, (double)(t1 - t0) / CLOCKS_PER_SEC,
               (double)(t2 - t1) / CLOCKS_PER_SEC, (double)(t1 - t0) / CLOCKS_PER_SEC * 1e9 / ((double)S.n * (double)S.total));
        (void)sum;
        free(src); free(nxt); free(prv); free(ov); set_free(&raw); set_free(&S);
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--test") == 0)
        return run_tests(argc > 2 ? atoll(argv[2]) : 10000, argc > 3 ? strtoull(argv[3], NULL, 10) : 20261006ULL);
    if (argc > 1 && strcmp(argv[1], "--paper") == 0)
        return run_paper(argc > 2 ? atoi(argv[2]) : 5, argc > 3 ? atoi(argv[3]) : 20, argc > 4 ? atoi(argv[4]) : 25);
    if (argc > 1 && strcmp(argv[1], "--bench") == 0) return run_bench();
    return run_stdin();
}