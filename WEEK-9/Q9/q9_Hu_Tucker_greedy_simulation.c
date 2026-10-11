/*
 * hu_tucker.c  --  Optimal alphabetic binary tree (Hu-Tucker, Garsia-Wachs form)
 *
 * Given weights w[1..n] in a fixed order, build the binary tree whose leaves
 * are 1..n IN THIS ORDER (in-order sequence preserved) and which minimises
 *                    sum  w[i] * depth(i)          (root has depth 0).
 *
 * Build : gcc -O2 -std=c11 -Wall -Wextra -o hu_tucker hu_tucker.c
 *
 * Usage :
 *   hu_tucker                   read stdin, print cost, depths, codes, tree
 *   hu_tucker -v                same + run all validators (verdict on stderr)
 *   hu_tucker --stress N [S]    self-test on N random instances (seed S)
 *
 * INPUT    n
 *          w[1] w[2] ... w[n]          non-negative integers, total <= 1e17
 * OUTPUT   Minimum cost, one row per leaf (index weight depth code), and the
 *          tree as a parenthesised expression whose leaf order is 1..n.
 *
 * ALGORITHM  (Hu-Tucker's three phases, in Garsia-Wachs form)
 *   Phase 1  COMBINATION.  Work on the list  [INF, w1..wn, INF].
 *            Find the LEFTMOST index i with  x[i-1] <= x[i+1]  (a "local min pair"),
 *            merge x[i-1],x[i] into s = x[i-1]+x[i], delete both and re-insert s
 *            immediately to the right of the nearest element on its left that is
 *            >= s.  (Hu-Tucker's "minimum compatible pair" rule is equivalent.)
 *            Each merge adds s to the cost.  n-1 merges build a tree T' whose
 *            leaves are NOT yet in order.
 *   Phase 2  LEVEL ASSIGNMENT.  depth(i) = depth of leaf i in T' (parent array,
 *            one in-place sweep).
 *   Phase 3  RECOMBINATION.  Scan leaves left to right with a stack of
 *            (depth,node); whenever the two top entries have equal depth, join
 *            them into a node of depth-1.  This yields the unique ordered tree
 *            with exactly those leaf depths; the theorem guarantees it exists
 *            and is optimal.
 *
 * DATA STRUCTURE  (this is what makes Phase 1 O(n log n))
 *   The list lives in a randomised balanced tree (treap, implicit order) that
 *   stores per node: subtree size, subtree MAX weight, subtree "has violator"
 *   flag.  A node c is a violator when  w[prev(prev(c))] <= w[c].
 *     - leftmost violator ........ descend using the flags            O(log n)
 *     - insertion point .......... descend using subtree max           O(log n)
 *     - delete 2 / insert 1 ...... split / merge                       O(log n)
 *   A doubly linked list beside the treap gives neighbours in O(1), so only the
 *   <= 5 nodes whose prev(prev()) changed get their flag recomputed.
 *
 * ZERO WEIGHTS  A zero weight is treated as an infinitesimal epsilon (each weight is
 *   the pair (w, number of zero leaves inside)), compared lexicographically.  The
 *   cost is unchanged, but among all optimal trees the one that also balances the
 *   zero leaves is chosen, so depths stay O(log n) instead of degenerating into a chain.
 *
 * COMPLEXITY   time O(n log n) expected,   space O(n).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_TOTAL 100000000000000000ULL /* 1e17: keeps every sum/cost exact in 64 bits */
#define INFW UINT64_MAX

/* ================================================================== */
/*  Treap with implicit order (index 0 is the NIL node)                */
/* ================================================================== */
typedef struct {
    uint64_t w, mx;
    uint32_t z, mz;                 /* zero-leaf count of this node / subtree max (tie component) */
    uint32_t l, r, pri, sz;
    uint32_t tid, prv, nxt;
    uint8_t flag, any;
} TNode;

/* lexicographic order on (weight, zero-leaf count) */
#define LT(w1, z1, w2, z2) ((w1) < (w2) || ((w1) == (w2) && (z1) < (z2)))
#define LE(w1, z1, w2, z2) (!LT(w2, z2, w1, z1))

static TNode *T;
static uint64_t trs = 0x9E3779B97F4A7C15ULL;
static uint32_t trnd(void) { trs ^= trs << 13; trs ^= trs >> 7; trs ^= trs << 17; return (uint32_t)(trs >> 32); }

static inline void pull(uint32_t x)
{
    TNode *t = &T[x], *L = &T[t->l], *R = &T[t->r];
    t->sz = L->sz + R->sz + 1;
    uint64_t m = t->w;
    uint32_t mz = t->z;
    if (LT(m, mz, L->mx, L->mz)) { m = L->mx; mz = L->mz; }
    if (LT(m, mz, R->mx, R->mz)) { m = R->mx; mz = R->mz; }
    t->mx = m; t->mz = mz;
    t->any = (uint8_t)(t->flag | L->any | R->any);
}

static void split(uint32_t x, uint32_t k, uint32_t *a, uint32_t *b)
{
    if (!x) { *a = *b = 0; return; }
    if (T[T[x].l].sz >= k) {
        split(T[x].l, k, a, &T[x].l);
        *b = x;
    } else {
        split(T[x].r, k - T[T[x].l].sz - 1, &T[x].r, b);
        *a = x;
    }
    pull(x);
}

static uint32_t merge(uint32_t a, uint32_t b)
{
    if (!a) return b;
    if (!b) return a;
    if (T[a].pri > T[b].pri) { T[a].r = merge(T[a].r, b); pull(a); return a; }
    T[b].l = merge(a, T[b].l); pull(b); return b;
}

static void pull_all(uint32_t x)
{
    if (!x) return;
    pull_all(T[x].l);
    pull_all(T[x].r);
    pull(x);
}

static void pull_small(uint32_t x) /* piece of <= 2 nodes */
{
    if (!x) return;
    if (T[x].l) pull(T[x].l);
    if (T[x].r) pull(T[x].r);
    pull(x);
}

static void set_flag(uint32_t x)
{
    uint32_t p = T[x].prv, pp = p ? T[p].prv : 0;
    T[x].flag = (uint8_t)(pp && LE(T[pp].w, T[pp].z, T[x].w, T[x].z));
}

/* ================================================================== */
/*  Phases 1 + 2: optimal leaf depths in O(n log n)                    */
/*  depth[i] = depth of leaf i in the optimal alphabetic tree          */
/* ================================================================== */
static int gw_depths(uint32_t n, const uint64_t *w, uint32_t *depth, uint64_t *cost_out)
{
    *cost_out = 0;
    if (n == 0) return 0;
    if (n == 1) { depth[0] = 0; return 0; }

    uint32_t N = n + 2; /* slot 1 = left sentinel, slots 2..n+1 = leaves, slot N = right sentinel */
    T = calloc((size_t)N + 1, sizeof *T);
    uint32_t *par = malloc((2 * (size_t)n - 1) * sizeof *par);
    uint32_t *stk = malloc(((size_t)N + 1) * sizeof *stk);
    if (!T || !par || !stk) { free(T); free(par); free(stk); return -1; }

    for (uint32_t id = 1; id <= N; id++) {
        T[id].w = (id == 1 || id == N) ? INFW : w[id - 2];
        T[id].z = (id >= 2 && id <= n + 1 && w[id - 2] == 0) ? 1 : 0;
        T[id].tid = (id >= 2 && id <= n + 1) ? id - 2 : 0;
        T[id].prv = id - 1;                 /* 0 for the first node */
        T[id].nxt = (id < N) ? id + 1 : 0;
        T[id].pri = trnd();
    }
    for (uint32_t id = 1; id <= N; id++) set_flag(id);

    uint32_t sp = 0; /* linear-time Cartesian-tree build (max-heap on pri) */
    for (uint32_t id = 1; id <= N; id++) {
        uint32_t last = 0;
        while (sp && T[stk[sp - 1]].pri < T[id].pri) last = stk[--sp];
        T[id].l = last;
        if (sp) T[stk[sp - 1]].r = id;
        stk[sp++] = id;
    }
    uint32_t root = stk[0];
    pull_all(root);

    uint64_t cost = 0;
    uint32_t nextid = n;
    for (uint32_t step = 0; step + 1 < n; step++) {
        /* --- leftmost violator c at 0-based rank r ------------------- */
        uint32_t c = root, r = 0;
        for (;;) {
            if (T[T[c].l].any) { c = T[c].l; continue; }
            r += T[T[c].l].sz;
            if (T[c].flag) break;
            r++;
            c = T[c].r;
        }
        uint32_t b = T[c].prv, a = T[b].prv, p = T[a].prv;

        uint32_t A, rest, mid, C;
        split(root, r - 2, &A, &rest);
        split(rest, 2, &mid, &C);
        (void)mid;

        uint64_t s = T[a].w + T[b].w;
        uint32_t sz0 = T[a].z + T[b].z;
        cost += s;
        par[T[a].tid] = nextid;
        par[T[b].tid] = nextid;
        T[a].tid = nextid++;
        T[a].w = s; T[a].z = sz0;

        T[c].prv = p; T[p].nxt = c;          /* unlink a, b */

        /* --- rightmost element >= s inside A --------------------------- */
        uint32_t q = A, k = 0;
        for (;;) {
            uint32_t rc = T[q].r;
            if (rc && LE(s, sz0, T[rc].mx, T[rc].mz)) { k += T[T[q].l].sz + 1; q = rc; }
            else if (LE(s, sz0, T[q].w, T[q].z)) { k += T[T[q].l].sz; break; }
            else q = T[q].l;
        }
        uint32_t A1, A2;
        split(A, k + 1, &A1, &A2);

        uint32_t n2 = T[q].nxt;              /* link s after q */
        T[q].nxt = a; T[a].prv = q; T[a].nxt = n2; T[n2].prv = a;
        T[a].l = T[a].r = 0; T[a].any = 0;

        uint32_t A2f, A2r, Cf, Cr;
        split(A2, 2, &A2f, &A2r);
        split(C, 2, &Cf, &Cr);

        /* --- recompute flags of nodes whose prev(prev()) changed ------- */
        uint32_t x1 = T[a].nxt, x2 = x1 ? T[x1].nxt : 0, c2 = T[c].nxt;
        set_flag(a);
        if (x1) set_flag(x1);
        if (x2) set_flag(x2);
        set_flag(c);
        if (c2) set_flag(c2);
        pull(a);
        pull_small(A2f);
        pull_small(Cf);

        root = merge(merge(merge(merge(merge(A1, a), A2f), A2r), Cf), Cr);
    }

    /* Phase 2: parent index > child index, one downward sweep gives depths */
    size_t nodes = 2 * (size_t)n - 1;
    par[nodes - 1] = 0;
    for (size_t i = nodes - 1; i-- > 0;) par[i] = par[par[i]] + 1;
    for (uint32_t i = 0; i < n; i++) depth[i] = par[i];

    *cost_out = cost;
    free(T); free(par); free(stk);
    T = NULL;
    return 0;
}

/* ================================================================== */
/*  Phase 3: ordered tree from the leaf depths (stack recombination)   */
/*  leaves 0..n-1, internal nodes n..2n-2 ; lc/rc indexed by id-n      */
/* ================================================================== */
static const char *build_tree(uint32_t n, const uint32_t *depth, uint32_t *lc, uint32_t *rc, uint32_t *root)
{
    if (n == 0) { *root = 0; return NULL; }
    uint32_t *sd = malloc((size_t)n * sizeof *sd), *sid = malloc((size_t)n * sizeof *sid);
    if (!sd || !sid) { free(sd); free(sid); return "out of memory"; }
    uint32_t sp = 0, made = 0;
    const char *err = NULL;
    for (uint32_t i = 0; i < n && !err; i++) {
        sd[sp] = depth[i]; sid[sp] = i; sp++;
        while (sp >= 2 && sd[sp - 1] == sd[sp - 2]) {
            if (sd[sp - 1] == 0 || made + 1 >= n) { err = "invalid depth sequence"; break; }
            lc[made] = sid[sp - 2];
            rc[made] = sid[sp - 1];
            uint32_t d = sd[sp - 1] - 1;
            sp -= 2;
            sd[sp] = d; sid[sp] = n + made; sp++;
            made++;
        }
    }
    if (!err && (sp != 1 || sd[0] != 0 || made != n - 1)) err = "depth sequence is not a complete tree";
    if (!err) *root = sid[0];
    free(sd); free(sid);
    return err;
}

/* ================================================================== */
/*  Output                                                            */
/* ================================================================== */
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

static int print_result(uint32_t n, const uint64_t *w, const uint32_t *depth, const uint32_t *lc,
                        const uint32_t *rc, uint32_t root, uint64_t cost)
{
    oputs("n = "); oputu(n); oputc('\n');
    oputs("Minimum cost = "); oputu(cost); oputc('\n');
    if (n == 0) { oputs("Tree: (empty)\n"); oflush(); return 0; }

    /* codes: DFS, 0 = left edge, 1 = right edge */
    uint32_t maxd = 0;
    for (uint32_t i = 0; i < n; i++) if (depth[i] > maxd) maxd = depth[i];
    char *code = malloc((size_t)maxd + 2);
    uint32_t *sn = malloc(((size_t)2 * n + 4) * sizeof *sn);
    uint32_t *sdp = malloc(((size_t)2 * n + 4) * sizeof *sdp);
    char *sb = malloc(((size_t)2 * n + 4));
    if (!code || !sn || !sdp || !sb) return -1;
    oputs("Index Weight Depth Code\n");
    size_t sp = 0;
    sn[sp] = root; sdp[sp] = 0; sb[sp] = 0; sp++;
    while (sp) {
        sp--;
        uint32_t id = sn[sp], d = sdp[sp];
        if (d > 0) code[d - 1] = sb[sp];
        if (id < n) {
            oputu((uint64_t)id + 1); oputc(' '); oputu(w[id]); oputc(' '); oputu(d); oputc(' ');
            if (d == 0) oputc('-');
            else for (uint32_t j = 0; j < d; j++) oputc(code[j]);
            oputc('\n');
        } else {
            sn[sp] = rc[id - n]; sdp[sp] = d + 1; sb[sp] = '1'; sp++;
            sn[sp] = lc[id - n]; sdp[sp] = d + 1; sb[sp] = '0'; sp++;
        }
    }
    /* parenthesised tree: stack items >=0 expand a node, -1 = ')', -2 = ' ' */
    int64_t *ts = malloc(((size_t)3 * n + 8) * sizeof *ts);
    if (!ts) return -1;
    size_t tp = 0;
    ts[tp++] = root;
    oputs("Tree: ");
    while (tp) {
        int64_t it = ts[--tp];
        if (it == -1) oputc(')');
        else if (it == -2) oputc(' ');
        else if ((uint64_t)it < n) oputu((uint64_t)it + 1);
        else {
            oputc('(');
            ts[tp++] = -1;
            ts[tp++] = rc[(uint32_t)it - n];
            ts[tp++] = -2;
            ts[tp++] = lc[(uint32_t)it - n];
        }
    }
    oputc('\n');
    oflush();
    free(code); free(sn); free(sdp); free(sb); free(ts);
    return 0;
}

/* ================================================================== */
/*  Independent oracles (validation only)                             */
/* ================================================================== */

/* oracle A: naive Garsia-Wachs, same rules, O(n^2), array based */
static int gw_naive(uint32_t n, const uint64_t *w, uint32_t *depth, uint64_t *cost_out)
{
    *cost_out = 0;
    if (n == 0) return 0;
    if (n == 1) { depth[0] = 0; return 0; }
    uint64_t *iw = malloc(((size_t)n + 2) * sizeof *iw);
    uint32_t *iz = malloc(((size_t)n + 2) * sizeof *iz);
    uint32_t *it = malloc(((size_t)n + 2) * sizeof *it);
    uint32_t *par = malloc((2 * (size_t)n - 1) * sizeof *par);
    if (!iw || !iz || !it || !par) { free(iw); free(iz); free(it); free(par); return -1; }
    iw[0] = INFW; iw[n + 1] = INFW; iz[0] = iz[n + 1] = 0;
    for (uint32_t i = 0; i < n; i++) { iw[i + 1] = w[i]; iz[i + 1] = (w[i] == 0); it[i + 1] = i; }
    uint32_t m = n, nextid = n;
    uint64_t cost = 0;
    while (m > 1) {
        uint32_t i = 1;
        while (!LE(iw[i - 1], iz[i - 1], iw[i + 1], iz[i + 1])) i++;
        uint64_t s = iw[i - 1] + iw[i];
        uint32_t sz0 = iz[i - 1] + iz[i];
        cost += s;
        par[it[i - 1]] = nextid; par[it[i]] = nextid;
        uint32_t j = i - 2;
        while (j > 0 && LT(iw[j], iz[j], s, sz0)) j--;
        if (i >= 2 + j) {
            memmove(&iw[j + 2], &iw[j + 1], (size_t)(i - 2 - j) * sizeof *iw);
            memmove(&iz[j + 2], &iz[j + 1], (size_t)(i - 2 - j) * sizeof *iz);
            memmove(&it[j + 2], &it[j + 1], (size_t)(i - 2 - j) * sizeof *it);
        }
        iw[j + 1] = s; iz[j + 1] = sz0; it[j + 1] = nextid++;
        memmove(&iw[i], &iw[i + 1], (size_t)(m + 1 - i) * sizeof *iw);
        memmove(&iz[i], &iz[i + 1], (size_t)(m + 1 - i) * sizeof *iz);
        memmove(&it[i], &it[i + 1], (size_t)(m + 1 - i) * sizeof *it);
        m--;
    }
    size_t nodes = 2 * (size_t)n - 1;
    par[nodes - 1] = 0;
    for (size_t i = nodes - 1; i-- > 0;) par[i] = par[par[i]] + 1;
    for (uint32_t i = 0; i < n; i++) depth[i] = par[i];
    *cost_out = cost;
    free(iw); free(iz); free(it); free(par);
    return 0;
}

/* oracle B: original Hu-Tucker combination phase (minimum compatible pair) */
static uint64_t ht_cost(uint32_t n, const uint64_t *w)
{
    if (n < 2) return 0;
    uint64_t *x = malloc((size_t)n * sizeof *x);
    uint8_t *term = malloc(n);
    if (!x || !term) { free(x); free(term); return INFW; }
    for (uint32_t i = 0; i < n; i++) { x[i] = w[i]; term[i] = 1; }
    uint32_t m = n;
    uint64_t cost = 0;
    while (m > 1) {
        uint64_t bs = INFW;
        uint32_t bi = 0, bj = 0;
        for (uint32_t i = 0; i < m; i++)
            for (uint32_t j = i + 1; j < m; j++) {
                uint64_t s = x[i] + x[j];
                if (s < bs) { bs = s; bi = i; bj = j; }
                if (term[j]) break;      /* a terminal blocks every pair beyond it */
            }
        cost += bs;
        x[bi] = bs; term[bi] = 0;
        memmove(&x[bj], &x[bj + 1], (size_t)(m - bj - 1) * sizeof *x);
        memmove(&term[bj], &term[bj + 1], (size_t)(m - bj - 1));
        m--;
    }
    free(x); free(term);
    return cost;
}

/* oracle C: exact interval DP, O(n^3) */
static uint64_t dp_cost(uint32_t n, const uint64_t *w)
{
    if (n < 2) return 0;
    uint64_t *C = calloc((size_t)n * n, sizeof *C), *pre = malloc(((size_t)n + 1) * sizeof *pre);
    if (!C || !pre) { free(C); free(pre); return INFW; }
    pre[0] = 0;
    for (uint32_t i = 0; i < n; i++) pre[i + 1] = pre[i] + w[i];
    for (uint32_t len = 2; len <= n; len++)
        for (uint32_t i = 0; i + len <= n; i++) {
            uint32_t j = i + len - 1;
            uint64_t best = INFW;
            for (uint32_t k = i; k < j; k++) {
                uint64_t v = C[(size_t)i * n + k] + C[(size_t)(k + 1) * n + j];
                if (v < best) best = v;
            }
            C[(size_t)i * n + j] = best + pre[j + 1] - pre[i];
        }
    uint64_t r = C[n - 1];
    free(C); free(pre);
    return r;
}

/* structural check of the produced tree + cost identity */
static const char *check_tree(uint32_t n, const uint64_t *w, const uint32_t *depth, const uint32_t *lc,
                              const uint32_t *rc, uint32_t root, uint64_t cost)
{
    if (n == 0) return NULL;
    uint64_t c2 = 0;
    for (uint32_t i = 0; i < n; i++) c2 += w[i] * depth[i];
    if (c2 != cost) return "cost != sum w*depth";
    uint32_t *sn = malloc(((size_t)2 * n + 4) * sizeof *sn), *sdp = malloc(((size_t)2 * n + 4) * sizeof *sdp);
    if (!sn || !sdp) { free(sn); free(sdp); return "validator out of memory"; }
    const char *err = NULL;
    size_t sp = 0;
    uint32_t expect = 0, internal = 0;
    sn[sp] = root; sdp[sp] = 0; sp++;
    while (sp && !err) {
        sp--;
        uint32_t id = sn[sp], d = sdp[sp];
        if (id < n) {
            if (id != expect) err = "leaves not in input order";
            else if (depth[id] != d) err = "leaf depth mismatch";
            expect++;
        } else {
            if (id >= 2 * n - 1) { err = "bad node id"; break; }
            internal++;
            sn[sp] = rc[id - n]; sdp[sp] = d + 1; sp++;
            sn[sp] = lc[id - n]; sdp[sp] = d + 1; sp++;
        }
    }
    if (!err && (expect != n || internal != n - 1)) err = "wrong leaf / internal count";
    free(sn); free(sdp);
    return err;
}

#define DP_LIM 60
#define HT_LIM 250
#define NAIVE_LIM 4000

static char errbuf[160];

/* run the real algorithm + every check; returns NULL when all agree */
static const char *check_instance(uint32_t n, const uint64_t *w, int *oracle_mask)
{
    uint32_t *depth = malloc(((size_t)n + 1) * sizeof *depth);
    uint32_t *lc = malloc(((size_t)n + 1) * sizeof *lc), *rc = malloc(((size_t)n + 1) * sizeof *rc);
    uint32_t *d2 = malloc(((size_t)n + 1) * sizeof *d2);
    if (!depth || !lc || !rc || !d2) return "validator out of memory";
    const char *err = NULL;
    uint64_t cost, c2;
    uint32_t root = 0;
    if (gw_depths(n, w, depth, &cost)) { err = "algorithm out of memory"; goto done; }
    if ((err = build_tree(n, depth, lc, rc, &root))) { snprintf(errbuf, sizeof errbuf, "recombination: %s", err); err = errbuf; goto done; }
    if ((err = check_tree(n, w, depth, lc, rc, root, cost))) goto done;
    if (n <= NAIVE_LIM) {
        gw_naive(n, w, d2, &c2);
        if (c2 != cost || memcmp(depth, d2, (size_t)n * sizeof *depth)) { err = "treap result differs from naive Garsia-Wachs"; goto done; }
        if (oracle_mask) *oracle_mask |= 1;
    }
    if (n <= HT_LIM) {
        if (ht_cost(n, w) != cost) { err = "cost differs from original Hu-Tucker"; goto done; }
        if (oracle_mask) *oracle_mask |= 2;
    }
    if (n <= DP_LIM) {
        if (dp_cost(n, w) != cost) { err = "cost differs from exact DP optimum"; goto done; }
        if (oracle_mask) *oracle_mask |= 4;
    }
done:
    free(depth); free(lc); free(rc); free(d2);
    return err;
}

/* ================================================================== */
/*  Random stress test                                                */
/* ================================================================== */
static uint64_t rs = 88172645463325252ULL;
static uint64_t rnd(void) { rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; return rs; }
static uint64_t rr(uint64_t lo, uint64_t hi) { return lo + rnd() % (hi - lo + 1); }

#define STRESS_CAP 1500
#define NTYPES 13
static const char *type_name[NTYPES] = {
    "1..3", "1..10", "1..1000", "1..1e9", "all equal", "increasing", "decreasing", "powers of two",
    "fibonacci", "with zeros", "sawtooth", "bitonic", "valley"
};

static uint32_t gen(uint64_t *w, int *type_out)
{
    uint32_t n;
    uint64_t r = rr(0, 99);
    if (rr(0, 199) == 0) n = 0;
    else if (r < 40) n = (uint32_t)rr(1, 8);
    else if (r < 70) n = (uint32_t)rr(9, 40);
    else if (r < 90) n = (uint32_t)rr(41, 200);
    else n = (uint32_t)rr(201, STRESS_CAP);
    int type = (int)rr(0, NTYPES - 1);
    *type_out = type;
    uint64_t eq = rr(1, 100000), k = rr(2, 12);
    uint32_t peak = n ? (uint32_t)rr(0, n - 1) : 0;
    int desc = (int)rr(0, 1);
    if (type == 8) { /* Fibonacci weights, ascending / descending / shuffled */
        if (n > 70) n = 70;
        uint64_t a = 1, b = 1;
        for (uint32_t i = 0; i < n; i++) { w[i] = a; uint64_t t = a + b; a = b; b = t; }
        int mode = (int)rr(0, 2);
        if (mode == 1) for (uint32_t i = 0; i < n / 2; i++) { uint64_t t = w[i]; w[i] = w[n - 1 - i]; w[n - 1 - i] = t; }
        if (mode == 2) for (uint32_t i = n; i-- > 1;) { uint32_t j = (uint32_t)rr(0, i); uint64_t t = w[i]; w[i] = w[j]; w[j] = t; }
        return n;
    }
    for (uint32_t i = 0; i < n; i++) {
        uint64_t v;
        switch (type) {
        case 0: v = rr(1, 3); break;
        case 1: v = rr(1, 10); break;
        case 2: v = rr(1, 1000); break;
        case 3: v = rr(1, 1000000000ULL); break;
        case 4: v = eq; break;
        case 5: v = desc ? (n - i) : (i + 1); break;
        case 6: v = desc ? (i + 1) : (n - i); break;
        case 7: v = 1ULL << rr(0, 40); break;
        case 9: v = rr(0, 3); break;
        case 10: v = 1 + (i % k) * (desc ? 1 : 7); break;
        case 11: v = 1 + (i <= peak ? i : (n - 1 - i)); break; /* bitonic */
        default: v = 1 + (i > peak ? i - peak : peak - i); break; /* valley */
        }
        w[i] = v;
    }
    return n;
}

static int stress(uint64_t N, uint64_t seed)
{
    if (seed) rs = seed;
    uint64_t *w = malloc(STRESS_CAP * sizeof *w);
    if (!w) return 2;
    uint64_t pass = 0, fail = 0, tc[NTYPES] = {0}, small = 0, mid = 0, big = 0, tiny = 0;
    uint32_t maxn = 0;
    for (uint64_t t = 1; t <= N; t++) {
        int type;
        uint32_t n = gen(w, &type);
        int mask = 0;
        const char *err = check_instance(n, w, &mask);
        tc[type]++;
        if (n <= 1) tiny++;
        if (mask & 4) small++;
        if (mask & 2) mid++;
        if (n >= 201) big++;
        if (n > maxn) maxn = n;
        if (err) {
            fail++;
            if (fail <= 5) {
                printf("FAIL #%llu type=%s n=%u : %s\n  w:", (unsigned long long)t, type_name[type], n, err);
                for (uint32_t i = 0; i < n && i < 25; i++) printf(" %llu", (unsigned long long)w[i]);
                printf("\n");
            }
        } else pass++;
    }
    printf("Stress result: %llu / %llu passed, %llu failed\n", (unsigned long long)pass,
           (unsigned long long)N, (unsigned long long)fail);
    printf("  n<=1: %llu | exact-DP checked (n<=%d): %llu | original-HT checked (n<=%d): %llu | n>=201: %llu | largest n=%u\n",
           (unsigned long long)tiny, DP_LIM, (unsigned long long)small, HT_LIM, (unsigned long long)mid,
           (unsigned long long)big, maxn);
    printf("  types:");
    for (int i = 0; i < NTYPES; i++) printf(" [%s]=%llu", type_name[i], (unsigned long long)tc[i]);
    printf("\n");
    free(w);
    return fail ? 1 : 0;
}

/* ================================================================== */
/*  Input / main                                                      */
/* ================================================================== */
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
    if (!rd_u64(&n64) || n64 >= (1ULL << 29)) { fprintf(stderr, "error: bad n\n"); return 2; }
    uint32_t n = (uint32_t)n64;
    uint64_t *w = malloc((n ? n : 1) * sizeof *w);
    uint32_t *depth = malloc((n ? n : 1) * sizeof *depth);
    uint32_t *lc = malloc((n ? n : 1) * sizeof *lc), *rc = malloc((n ? n : 1) * sizeof *rc);
    if (!w || !depth || !lc || !rc) { fprintf(stderr, "error: out of memory\n"); return 2; }

    uint64_t total = 0;
    for (uint32_t i = 0; i < n; i++) {
        if (!rd_u64(&w[i])) { fprintf(stderr, "error: expected %u weights\n", n); return 2; }
        if (w[i] > MAX_TOTAL || total > MAX_TOTAL - w[i]) { fprintf(stderr, "error: total weight exceeds 1e17\n"); return 2; }
        total += w[i];
    }

    uint64_t cost;
    uint32_t root = 0;
    if (gw_depths(n, w, depth, &cost)) { fprintf(stderr, "error: out of memory\n"); return 2; }
    const char *err = build_tree(n, depth, lc, rc, &root);
    if (err) { fprintf(stderr, "internal error: %s\n", err); return 3; }
    if (print_result(n, w, depth, lc, rc, root, cost)) { fprintf(stderr, "error: out of memory\n"); return 2; }

    if (verify) {
        int mask = 0;
        const char *e = check_instance(n, w, &mask);
        if (e) { fprintf(stderr, "[VALIDATION] FAIL: %s\n", e); return 1; }
        fprintf(stderr, "[VALIDATION] PASS (tree structure ok%s%s%s)\n", (mask & 1) ? ", naive GW" : "",
                (mask & 2) ? ", original Hu-Tucker" : "", (mask & 4) ? ", exact DP" : "");
    }
    free(w); free(depth); free(lc); free(rc);
    return 0;
}