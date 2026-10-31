/* Parity-constrained shortest path via 0-1 BFS on states (v, parity of k1).
 * "Path" = walk (vertices/edges may repeat), the standard reading of this problem.
 * Input : n m S C, then m lines "u v w"   (vertices 1..n; compile -DZERO_INDEXED for 0..n-1)
 * Output: n lines: min Cost(P) from S to vertex i, or UNREACHABLE.
 * Build : gcc -std=c11 -O2 -Wall -Wextra [-fsanitize=address,undefined] sol.c -o sol */
#include <stdio.h>
#include <stdlib.h>

#ifdef ZERO_INDEXED
#define BASE 0
#else
#define BASE 1
#endif
#define INF (-1)

static char ibuf[1 << 16];
static size_t ilen = 0, ipos = 0;
static int gc(void) {
    if (ipos == ilen) { ilen = fread(ibuf, 1, sizeof ibuf, stdin); ipos = 0; if (!ilen) return -1; }
    return (unsigned char)ibuf[ipos++];
}
static int read_ll(long long *out) {          /* returns 1 on success, 0 on EOF/garbage */
    int c = gc(), neg = 0;
    while (c == ' ' || c == '\n' || c == '\r' || c == '\t') c = gc();
    if (c == '-') { neg = 1; c = gc(); }
    if (c < '0' || c > '9') return 0;
    long long x = 0;
    while (c >= '0' && c <= '9') {
        if (x > (long long)4e18 / 10) return 0;           /* overflow guard */
        x = x * 10 + (c - '0'); c = gc();
    }
    *out = neg ? -x : x;
    return 1;
}
static int bad(const char *msg) { fprintf(stderr, "Invalid input: %s\n", msg); return 1; }

int main(void) {
    long long n_, m_, s_, C;
    if (!read_ll(&n_) || !read_ll(&m_) || !read_ll(&s_) || !read_ll(&C)) return bad("header");
    if (n_ < 1 || n_ > 100000000LL || m_ < 0 || m_ > 400000000LL) return bad("n or m out of range");
    if (C < 0) return bad("C must be >= 0");
    if (s_ < BASE || s_ >= n_ + BASE) return bad("source out of range");
    int n = (int)n_, m = (int)m_, s = (int)s_ - BASE;

    size_t mm = m > 0 ? (size_t)m : 1;
    int *eu = malloc(mm * sizeof *eu), *ev = malloc(mm * sizeof *ev);
    char *ew = malloc(mm), *wt = malloc(mm);
    int *to = malloc(mm * sizeof *to);
    int *start = calloc((size_t)n + 2, sizeof *start);
    int *pos = malloc(((size_t)n + 2) * sizeof *pos);
    if (!eu || !ev || !ew || !wt || !to || !start || !pos) return bad("out of memory");

    for (int i = 0; i < m; i++) {
        long long u, v, w;
        if (!read_ll(&u) || !read_ll(&v) || !read_ll(&w)) return bad("truncated edge list");
        if (u < BASE || u >= n_ + BASE || v < BASE || v >= n_ + BASE) return bad("vertex out of range");
        if (w != 0 && w != 1) return bad("weight must be 0 or 1");
        eu[i] = (int)u - BASE; ev[i] = (int)v - BASE; ew[i] = (char)w;
        start[eu[i] + 1]++;
    }
    for (int i = 1; i <= n; i++) start[i] += start[i - 1];          /* CSR adjacency */
    for (int i = 0; i <= n; i++) pos[i] = start[i];
    for (int i = 0; i < m; i++) { int p = pos[eu[i]]++; to[p] = ev[i]; wt[p] = ew[i]; }

    size_t ns = 2 * (size_t)n;                    /* state = 2*v + parity; dist = min k1 of that parity */
    int *dist = malloc(ns * sizeof *dist);
    size_t cap = 2 * (size_t)m + 4, head = 0, tail = 0, cnt = 1;     /* circular deque; pushes <= 2m+1 */
    int *dq = malloc(cap * sizeof *dq);
    if (!dist || !dq) return bad("out of memory");
    for (size_t i = 0; i < ns; i++) dist[i] = INF;
    dist[2 * s] = 0; dq[tail++] = 2 * s;
    while (cnt) {
        int st = dq[head]; head = (head + 1) % cap; cnt--;
        int v = st >> 1, p = st & 1, d = dist[st];
        for (int e = start[v]; e < start[v + 1]; e++) {
            int w = wt[e], t = 2 * to[e] + (p ^ w), nd = d + w;
            if (dist[t] == INF || nd < dist[t]) {
                dist[t] = nd;
                if (w == 0) { head = (head + cap - 1) % cap; dq[head] = t; }
                else { dq[tail] = t; tail = (tail + 1) % cap; }
                cnt++;
            }
        }
    }
    for (int v = 0; v < n; v++) {
        long long best = -1;
        if (dist[2 * v] != INF) best = dist[2 * v];
        if (dist[2 * v + 1] != INF) {
            long long c = (long long)dist[2 * v + 1] + C;
            if (best < 0 || c < best) best = c;
        }
        if (best < 0) puts("UNREACHABLE"); else printf("%lld\n", best);
    }
    free(eu); free(ev); free(ew); free(wt); free(to); free(start); free(pos); free(dist); free(dq);
    return 0;
}

