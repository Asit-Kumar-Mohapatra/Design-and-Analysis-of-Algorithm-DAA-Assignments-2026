/* =====================================================================
 * MAXIMUM WEIGHT COMPONENT WITH SIZE CONSTRAINT  (size <= K)
 * Edges are added one by one; after each edge we report the maximum total
 * vertex weight over all connected components having at most K vertices.
 *
 * Usage:  ./a.out < input     solve (normal mode)
 *         ./a.out --explain   print the algorithm + complexity explanation
 *         ./a.out --test      self-check against a brute force on random graphs
 *
 * Input : n m K
 *         w_1 ... w_n                (vertex weights, may be negative)
 *         u v c   (m lines)          (edge u-v, 1-indexed; c = edge weight, unused)
 * Output: m+1 lines: answer before any edge, then after each edge.
 *         "NONE" if no component has size <= K (only possible when K < 1).
 * ===================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef long long ll;
#define NONE_VAL LLONG_MIN          /* sentinel meaning "no valid component" */

/* ---------------------------------------------------------------------
 * EXPLANATION FUNCTION: prints the full reasoning (run with --explain)
 * --------------------------------------------------------------------- */
static void print_explanation(void) {
    puts(
"PROBLEM\n"
"  Edges arrive one at a time. After each edge, report the maximum total vertex\n"
"  weight among connected components with at most K vertices.\n"
"\n"
"INPUT REPRESENTATION\n"
"  Edge list. Edges arrive online, so we never need an adjacency matrix (O(n^2))\n"
"  or per-vertex lists. Components are tracked by Union-Find: for every root we\n"
"  store size and sum of vertex weights. Edge weights do not affect the objective.\n"
"\n"
"BRUTE FORCE\n"
"  After every edge rebuild all components and scan them: O(m * (n + m)).\n"
"\n"
"OPTIMAL APPROACH: Union-Find + max-heap with lazy deletion\n"
"  Key difficulty: the answer is NOT monotone. If the best component merges into a\n"
"  component larger than K, it disappears and the maximum can DROP.\n"
"  So we keep a max-heap of 'valid component versions' (sum, root, size).\n"
"  - When a component with size <= K is created, push it.\n"
"  - Union strictly increases the size of the surviving root, so (root,size)\n"
"    identifies one version of a component uniquely.\n"
"  - An entry is STALE if its root is no longer a root or its size changed.\n"
"  - Before answering, pop stale entries from the top; the top is the answer.\n"
"  - A component with size > K never becomes valid again (components only grow).\n"
"\n"
"INVARIANT\n"
"  After every step, each current component with size <= K has a fresh heap entry\n"
"  carrying its current sum, and every other entry is stale. Hence, after removing\n"
"  stale tops, the heap top equals the true maximum.\n"
"\n"
"COMPLEXITY\n"
"  Time : O((n + m) * alpha(n) + n log n)   (at most 2n-1 pushes, each popped once)\n"
"  Space: O(n + m)\n"
"  (No proof of a matching lower bound is claimed.)\n"
"\n"
"ASSUMPTIONS\n"
"  Vertices are 1-indexed; self-loops / duplicate edges are allowed and ignored;\n"
"  the initial state (before any edge) is printed as the first output line.");
}

/* ---------------------------------------------------------------------
 * HEAP: max-heap of Entry, ordered by component sum
 * --------------------------------------------------------------------- */
typedef struct { ll s; int r, z; } Entry;   /* sum, root, size when pushed */

typedef struct {
    int *par, *sz;      /* Union-Find: parent and component size            */
    ll  *sum;           /* Union-Find: total vertex weight of the component */
    Entry *h; int hn;   /* heap array and its current length                */
} State;

/* Insert an entry, sifting up to keep the max-heap property. O(log n) */
static void heap_push(State *st, Entry e) {
    int i = st->hn++;
    while (i > 0 && st->h[(i - 1) / 2].s < e.s) {
        st->h[i] = st->h[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    st->h[i] = e;
}

/* Remove the maximum entry, sifting down. O(log n) */
static void heap_pop(State *st) {
    Entry e = st->h[--st->hn];
    int i = 0;
    for (;;) {
        int c = 2 * i + 1;
        if (c >= st->hn) break;
        if (c + 1 < st->hn && st->h[c + 1].s > st->h[c].s) c++;
        if (st->h[c].s <= e.s) break;
        st->h[i] = st->h[c];
        i = c;
    }
    if (st->hn > 0) st->h[i] = e;
}

/* ---------------------------------------------------------------------
 * UNION-FIND
 * --------------------------------------------------------------------- */
/* Find the root of x, compressing the path (path halving). ~O(alpha(n)) */
static int dsu_find(State *st, int x) {
    while (st->par[x] != x) {
        st->par[x] = st->par[st->par[x]];
        x = st->par[x];
    }
    return x;
}

/* Merge the components of u and v (union by size). Returns the surviving root,
 * or -1 if they were already in the same component (nothing changes). */
static int dsu_union(State *st, int u, int v) {
    int a = dsu_find(st, u), b = dsu_find(st, v);
    if (a == b) return -1;                       /* self-loop / duplicate edge */
    if (st->sz[a] < st->sz[b]) { int t = a; a = b; b = t; }
    st->par[b] = a;                              /* attach smaller under larger */
    st->sz[a] += st->sz[b];
    st->sum[a] += st->sum[b];
    return a;
}

/* ---------------------------------------------------------------------
 * CURRENT ANSWER: discard stale heap entries, then return the maximum.
 * An entry is stale if its root was merged away or its component grew.
 * --------------------------------------------------------------------- */
static ll current_best(State *st) {
    while (st->hn > 0 &&
           (st->par[st->h[0].r] != st->h[0].r ||      /* root no longer a root */
            st->sz[st->h[0].r]  != st->h[0].z))       /* component size changed */
        heap_pop(st);
    return st->hn ? st->h[0].s : NONE_VAL;
}

static void state_free(State *st) {
    free(st->par); free(st->sz); free(st->sum); free(st->h);
}

/* ---------------------------------------------------------------------
 * FAST SOLVER: out[0..m] receives the answers. Returns 0 on success.
 * --------------------------------------------------------------------- */
static int solve_fast(int n, int m, ll K, const ll *w,
                      const int *U, const int *V, ll *out) {
    State st = {0};
    st.par = malloc(n * sizeof *st.par);
    st.sz  = malloc(n * sizeof *st.sz);
    st.sum = malloc(n * sizeof *st.sum);
    st.h   = malloc(((size_t)2 * n + 2) * sizeof *st.h);  /* <= 2n-1 pushes */
    if (!st.par || !st.sz || !st.sum || !st.h) { state_free(&st); return 1; }

    /* every vertex starts as its own component of size 1 */
    for (int i = 0; i < n; i++) {
        st.par[i] = i; st.sz[i] = 1; st.sum[i] = w[i];
        if (K >= 1) heap_push(&st, (Entry){w[i], i, 1});
    }
    out[0] = current_best(&st);                  /* state before any edge */

    for (int e = 0; e < m; e++) {
        int root = dsu_union(&st, U[e], V[e]);
        if (root >= 0 && st.sz[root] <= K)       /* new valid component version */
            heap_push(&st, (Entry){st.sum[root], root, st.sz[root]});
        out[e + 1] = current_best(&st);
    }
    state_free(&st);
    return 0;
}

/* ---------------------------------------------------------------------
 * BRUTE FORCE (used only for validation): O(m * (n + m)).
 * Rebuilds every component from scratch after each edge.
 * --------------------------------------------------------------------- */
static void solve_brute(int n, int m, ll K, const ll *w,
                        const int *U, const int *V, ll *out) {
    int *lab = malloc(n * sizeof *lab), *z = malloc(n * sizeof *z);
    ll  *s   = malloc(n * sizeof *s);
    for (int t = 0; t <= m; t++) {
        for (int i = 0; i < n; i++) lab[i] = i;
        for (int e = 0; e < t; e++) {            /* relabel-merge first t edges */
            int x = lab[U[e]], y = lab[V[e]];
            if (x != y) for (int i = 0; i < n; i++) if (lab[i] == y) lab[i] = x;
        }
        memset(s, 0, n * sizeof *s); memset(z, 0, n * sizeof *z);
        for (int i = 0; i < n; i++) { s[lab[i]] += w[i]; z[lab[i]]++; }
        ll best = NONE_VAL;
        for (int i = 0; i < n; i++)
            if (z[i] >= 1 && z[i] <= K && s[i] > best) best = s[i];
        out[t] = best;
    }
    free(lab); free(s); free(z);
}

/* ---------------------------------------------------------------------
 * SELF-VALIDATION: random graphs, fast solver vs brute force (--test)
 * Covers K=0, K>n, negative weights, self-loops and duplicate edges.
 * --------------------------------------------------------------------- */
static int run_self_test(void) {
    srand(12345);
    long tests = 20000, bad = 0;
    for (long t = 0; t < tests; t++) {
        int n = rand() % 10 + 1, m = rand() % 15;
        ll K = rand() % (n + 3);
        ll w[10], a[16], b[16]; int U[15], V[15];
        for (int i = 0; i < n; i++) w[i] = rand() % 21 - 8;
        for (int i = 0; i < m; i++) { U[i] = rand() % n; V[i] = rand() % n; }
        if (solve_fast(n, m, K, w, U, V, a)) return 1;
        solve_brute(n, m, K, w, U, V, b);
        if (memcmp(a, b, (m + 1) * sizeof(ll)) != 0) {
            bad++; printf("MISMATCH in test %ld\n", t);
        }
    }
    printf("%ld random tests, %ld mismatches\n", tests, bad);
    return bad != 0;
}

/* ---------------------------------------------------------------------
 * FAST INPUT: reads one signed integer; returns 0 on EOF
 * --------------------------------------------------------------------- */
static int read_ll(ll *x) {
    int ch = getchar(); ll sign = 1, v = 0;
    while (ch != EOF && ch != '-' && (ch < '0' || ch > '9')) ch = getchar();
    if (ch == EOF) return 0;
    if (ch == '-') { sign = -1; ch = getchar(); }
    for (; ch >= '0' && ch <= '9'; ch = getchar()) v = v * 10 + (ch - '0');
    *x = sign * v;
    return 1;
}

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--explain") == 0) { print_explanation(); return 0; }
    if (argc > 1 && strcmp(argv[1], "--test") == 0)    return run_self_test();

    ll n, m, K;
    if (!read_ll(&n) || !read_ll(&m) || !read_ll(&K) ||
        n < 1 || n > 250000000LL || m < 0 || m > 250000000LL) {
        fputs("invalid header\n", stderr); return 1;
    }
    ll *w   = malloc(n * sizeof *w);
    ll *out = malloc((m + 1) * sizeof *out);
    int *U  = malloc((m + 1) * sizeof *U), *V = malloc((m + 1) * sizeof *V);
    if (!w || !out || !U || !V) { fputs("alloc failed\n", stderr); return 1; }

    for (ll i = 0; i < n; i++)
        if (!read_ll(&w[i])) { fputs("bad input\n", stderr); return 1; }
    for (ll i = 0; i < m; i++) {
        ll u, v, c;                              /* c (edge weight) is unused */
        if (!read_ll(&u) || !read_ll(&v) || !read_ll(&c) ||
            u < 1 || u > n || v < 1 || v > n) { fputs("bad edge\n", stderr); return 1; }
        U[i] = (int)u - 1; V[i] = (int)v - 1;
    }
    if (solve_fast((int)n, (int)m, K, w, U, V, out)) {
        fputs("alloc failed\n", stderr); return 1;
    }
    for (ll i = 0; i <= m; i++) {
        if (out[i] == NONE_VAL) puts("NONE"); else printf("%lld\n", out[i]);
    }
    free(w); free(out); free(U); free(V);
    return 0;
}