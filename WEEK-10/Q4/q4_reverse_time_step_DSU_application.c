/*
 * Reverse Time-Step DSU
 * ---------------------
 * Vertex-weighted undirected graph; vertices are deleted in the order
 * P = [p1..pV]. After each deletion print the maximum, over all remaining
 * connected components, of the sum of vertex weights (0 if nothing remains).
 *
 * Input  : V
 *          w1 ... wV                  (any 64-bit integers, may be negative)
 *          E
 *          u v      (E lines, 1-indexed; self-loops / parallel edges allowed)
 *          p1 ... pV                  (a permutation of 1..V)
 * Output : V lines, line i = answer after deleting p1..pi
 *
 * Self-check: ./a.out --selftest [tests] [seed]   (DSU vs brute force)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef long long ll;

/* ------------------------------ fast I/O ------------------------------ */
static char ibuf[1 << 16];
static size_t ilen = 0, ipos = 0;

static int gc(void) {
    if (ipos == ilen) {
        ilen = fread(ibuf, 1, sizeof ibuf, stdin);
        ipos = 0;
        if (ilen == 0) return -1;
    }
    return (unsigned char)ibuf[ipos++];
}

static int read_ll(ll *x) {               /* 1 on success, 0 on EOF/garbage */
    int c = gc();
    while (c != -1 && c != '-' && (c < '0' || c > '9')) c = gc();
    if (c == -1) return 0;
    int neg = 0;
    if (c == '-') { neg = 1; c = gc(); if (c < '0' || c > '9') return 0; }
    ll r = 0;
    while (c >= '0' && c <= '9') { r = r * 10 + (c - '0'); c = gc(); }
    *x = neg ? -r : r;
    return 1;
}

static char obuf[1 << 16];
static size_t olen = 0;

static void flush_out(void) { fwrite(obuf, 1, olen, stdout); olen = 0; }

static void write_ll(ll x) {
    char t[24]; int n = 0;
    unsigned long long u = x < 0 ? 0ULL - (unsigned long long)x : (unsigned long long)x;
    if (olen + 24 > sizeof obuf) flush_out();
    if (x < 0) obuf[olen++] = '-';
    do { t[n++] = (char)('0' + u % 10); u /= 10; } while (u);
    while (n) obuf[olen++] = t[--n];
    obuf[olen++] = '\n';
}

/* ------------------------- max-heap (lazy deletion) -------------------- */
typedef struct { ll key; int id; } HNode;
static HNode *hp;
static int hn;

static void hpush(ll key, int id) {
    int i = hn++;
    while (i > 0) {
        int q = (i - 1) / 2;
        if (hp[q].key >= key) break;
        hp[i] = hp[q];
        i = q;
    }
    hp[i].key = key; hp[i].id = id;
}

static void hpop(void) {
    HNode last = hp[--hn];
    int i = 0;
    for (;;) {
        int c = 2 * i + 1;
        if (c >= hn) break;
        if (c + 1 < hn && hp[c + 1].key > hp[c].key) c++;
        if (hp[c].key <= last.key) break;
        hp[i] = hp[c];
        i = c;
    }
    if (hn > 0) hp[i] = last;
}

/* ------------------------------- DSU ---------------------------------- */
static int find(int *par, int x) {        /* iterative, path halving */
    while (par[x] != x) { par[x] = par[par[x]]; x = par[x]; }
    return x;
}

/*
 * Core algorithm. Vertices are 0-indexed. p[i] = vertex deleted at step i.
 * ans[i] = max component sum after deleting p[0..i].
 * Returns 0 on success, 1 on allocation failure.
 *
 * Edge (a,b) becomes present, in reverse time, at the moment the endpoint
 * that is deleted EARLIER (= re-inserted LATER) comes back. So each edge is
 * stored once, in the bucket of that endpoint; at that moment the other
 * endpoint is guaranteed to be present already.
 */
static int solve(int V, const ll *w, int E, const int *eu, const int *ev,
                 const int *p, ll *ans) {
    int rc = 1, hasNeg = 0;
    size_t n = (size_t)V;
    int *pos = malloc(n * sizeof *pos);
    int *par = malloc(n * sizeof *par);
    int *sz = malloc(n * sizeof *sz);
    ll *sum = malloc(n * sizeof *sum);
    int *start = calloc(n + 1, sizeof *start);
    int *cur = malloc((n + 1) * sizeof *cur);
    int *adj = malloc((size_t)(E > 0 ? E : 1) * sizeof *adj);
    hp = NULL; hn = 0;
    if (!pos || !par || !sz || !sum || !start || !cur || !adj) goto done;

    for (int i = 0; i < V; i++) if (w[i] < 0) { hasNeg = 1; break; }
    if (hasNeg) {                          /* <= V pushes + <= V-1 merges */
        hp = malloc((2 * n + 1) * sizeof *hp);
        if (!hp) goto done;
    }

    for (int i = 0; i < V; i++) pos[p[i]] = i;

    for (int e = 0; e < E; e++) {          /* counting sort of edges by owner */
        int a = eu[e], b = ev[e];
        if (a == b) continue;              /* self-loop never joins components */
        start[(pos[a] < pos[b] ? a : b) + 1]++;
    }
    for (int i = 0; i < V; i++) start[i + 1] += start[i];
    memcpy(cur, start, (n + 1) * sizeof *cur);
    for (int e = 0; e < E; e++) {
        int a = eu[e], b = ev[e];
        if (a == b) continue;
        if (pos[a] < pos[b]) adj[cur[a]++] = b;
        else                 adj[cur[b]++] = a;
    }

    for (int i = 0; i < V; i++) { par[i] = i; sz[i] = 1; sum[i] = 0; }

    ll best = 0;                           /* used only when no weight < 0 */
    for (int i = V - 1; i >= 0; i--) {
        /* state now = graph after deleting p[0..i]  ==> record answer */
        if (i == V - 1) ans[i] = 0;                    /* empty graph */
        else if (!hasNeg) ans[i] = best;
        else {
            while (par[hp[0].id] != hp[0].id || sum[hp[0].id] != hp[0].key) hpop();
            ans[i] = hp[0].key;
        }

        int u = p[i];                      /* re-insert p[i] */
        sum[u] = w[u];
        if (!hasNeg) { if (sum[u] > best) best = sum[u]; }
        else hpush(sum[u], u);

        for (int k = start[u]; k < start[u + 1]; k++) {
            int a = find(par, u), b = find(par, adj[k]);
            if (a == b) continue;
            if (sz[a] < sz[b]) { int t = a; a = b; b = t; }
            par[b] = a;                    /* union by size */
            sz[a] += sz[b];
            sum[a] += sum[b];
            if (!hasNeg) { if (sum[a] > best) best = sum[a]; }
            else hpush(sum[a], a);
        }
    }
    rc = 0;
done:
    free(pos); free(par); free(sz); free(sum); free(start); free(cur); free(adj);
    free(hp); hp = NULL; hn = 0;
    return rc;
}

/* ----------------------- brute force (for validation) ------------------ */
static int brute(int V, const ll *w, int E, const int *eu, const int *ev,
                 const int *p, ll *ans) {
    size_t n = (size_t)V;
    char *m = calloc(n * n, 1), *alive = malloc(n), *seen = malloc(n);
    int *st = malloc(n * sizeof *st);
    if (!m || !alive || !seen || !st) { free(m); free(alive); free(seen); free(st); return 1; }
    for (int e = 0; e < E; e++) { m[(size_t)eu[e] * n + ev[e]] = 1; m[(size_t)ev[e] * n + eu[e]] = 1; }
    memset(alive, 1, n);
    for (int i = 0; i < V; i++) {
        alive[p[i]] = 0;
        memset(seen, 0, n);
        int any = 0; ll best = 0;
        for (int s = 0; s < V; s++) {
            if (!alive[s] || seen[s]) continue;
            ll tot = 0; int top = 0;
            st[top++] = s; seen[s] = 1;
            while (top) {
                int x = st[--top]; tot += w[x];
                for (int y = 0; y < V; y++)
                    if (alive[y] && !seen[y] && m[(size_t)x * n + y]) { seen[y] = 1; st[top++] = y; }
            }
            if (!any || tot > best) { best = tot; any = 1; }
        }
        ans[i] = any ? best : 0;
    }
    free(m); free(alive); free(seen); free(st);
    return 0;
}

/* ------------------------------ self-test ------------------------------ */
static unsigned long long rs = 88172645463325252ULL;
static unsigned rnd(unsigned m) {          /* xorshift64, value in [0,m) */
    rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17;
    return (unsigned)((rs >> 11) % m);
}

static int selftest(long tests, unsigned long long seed) {
    if (seed) rs ^= seed * 0x9E3779B97F4A7C15ULL;
    long bad = 0, neg = 0;
    for (long t = 0; t < tests; t++) {
        int V = 1 + (int)rnd(t % 50 == 0 ? 60 : 10);
        int E = (int)rnd((unsigned)(2 * V + 3));
        int allowNeg = (int)rnd(2);
        neg += allowNeg;
        ll *w = malloc((size_t)V * sizeof *w), *a1 = malloc((size_t)V * sizeof *a1), *a2 = malloc((size_t)V * sizeof *a2);
        int *eu = malloc((size_t)(E + 1) * sizeof *eu), *ev = malloc((size_t)(E + 1) * sizeof *ev), *p = malloc((size_t)V * sizeof *p);
        if (!w || !a1 || !a2 || !eu || !ev || !p) { fprintf(stderr, "allocation failed\n"); return 2; }
        for (int i = 0; i < V; i++) w[i] = allowNeg ? (ll)rnd(36) - 15 : (ll)rnd(21);
        for (int e = 0; e < E; e++) { eu[e] = (int)rnd((unsigned)V); ev[e] = (int)rnd((unsigned)V); }
        for (int i = 0; i < V; i++) p[i] = i;
        for (int i = V - 1; i > 0; i--) { int j = (int)rnd((unsigned)i + 1), x = p[i]; p[i] = p[j]; p[j] = x; }
        if (solve(V, w, E, eu, ev, p, a1) || brute(V, w, E, eu, ev, p, a2)) { fprintf(stderr, "allocation failed\n"); return 2; }
        if (memcmp(a1, a2, (size_t)V * sizeof *a1) != 0) {
            bad++;
            if (bad <= 3) fprintf(stderr, "MISMATCH on test %ld (V=%d E=%d)\n", t, V, E);
        }
        free(w); free(a1); free(a2); free(eu); free(ev); free(p);
    }
    printf("selftest: %ld tests (%ld with negative weights allowed), %ld mismatches\n", tests, neg, bad);
    return bad ? 1 : 0;
}

/* -------------------------------- main --------------------------------- */
static int fail(const char *msg) { fprintf(stderr, "error: %s\n", msg); return 1; }

int main(int argc, char **argv) {
    if (argc >= 2 && strcmp(argv[1], "--selftest") == 0)
        return selftest(argc >= 3 ? atol(argv[2]) : 20000,
                        argc >= 4 ? strtoull(argv[3], NULL, 10) : 1ULL);

    ll t;
    if (!read_ll(&t)) return 0;            /* empty input: nothing to do */
    if (t < 0 || t > INT_MAX - 1) return fail("bad V");
    int V = (int)t;
    if (V == 0) return 0;

    ll *w = malloc((size_t)V * sizeof *w), *ans = malloc((size_t)V * sizeof *ans);
    int *p = malloc((size_t)V * sizeof *p);
    char *seen = calloc((size_t)V, 1);
    if (!w || !ans || !p || !seen) return fail("allocation failed");
    for (int i = 0; i < V; i++) if (!read_ll(&w[i])) return fail("truncated weights");

    if (!read_ll(&t) || t < 0 || t > INT_MAX) return fail("bad E");
    int E = (int)t;
    int *eu = malloc((size_t)(E > 0 ? E : 1) * sizeof *eu);
    int *ev = malloc((size_t)(E > 0 ? E : 1) * sizeof *ev);
    if (!eu || !ev) return fail("allocation failed");
    for (int e = 0; e < E; e++) {
        ll a, b;
        if (!read_ll(&a) || !read_ll(&b)) return fail("truncated edges");
        if (a < 1 || a > V || b < 1 || b > V) return fail("edge endpoint out of range");
        eu[e] = (int)a - 1; ev[e] = (int)b - 1;
    }
    for (int i = 0; i < V; i++) {
        if (!read_ll(&t)) return fail("truncated deletion order");
        if (t < 1 || t > V || seen[t - 1]) return fail("deletion order is not a permutation of 1..V");
        seen[t - 1] = 1; p[i] = (int)t - 1;
    }

    if (solve(V, w, E, eu, ev, p, ans)) return fail("allocation failed");
    for (int i = 0; i < V; i++) write_ll(ans[i]);
    flush_out();

    free(w); free(ans); free(p); free(seen); free(eu); free(ev);
    return 0;
}