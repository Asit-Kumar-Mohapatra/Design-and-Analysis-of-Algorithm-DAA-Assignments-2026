/*
 * Online bipartiteness testing.
 *
 * Representation: no adjacency structure at all. Edges are consumed as a
 * stream and only a union-find forest with a parity bit per vertex is kept.
 *   parent[x] < 0 : x is a root, -parent[x] = component size
 *   parent[x] >= 0: parent pointer
 *   par[x]        : colour(x) XOR colour(parent[x])
 * An edge (u,v) closes an odd cycle  <=>  u and v are in the same component
 * and have the same colour.
 *
 * Time  : O(V + E * alpha(V))  (union by size + path compression)
 * Memory: O(V)  (5 bytes per vertex)
 *
 * Input : V [E]            first line: vertex count, optional edge count
 *         u v              one edge per line, 0-indexed, until E edges or EOF
 * Output: "added" or "violates" per edge ("invalid" if a vertex is out of range)
 * Interactive (stdin is a terminal): each answer is flushed immediately.
 */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <io.h>
#define IS_TTY(fd) _isatty(fd)
#else
#include <unistd.h>
#define IS_TTY(fd) isatty(fd)
#endif

/* ------------------------------ DSU ------------------------------------ */
static int *parent;
static unsigned char *par;

/* Root of x; *p receives colour(x) XOR colour(root). Compresses the path. */
static int find(int x, int *p) {
    int root = x, acc = 0;
    while (parent[root] >= 0) { acc ^= par[root]; root = parent[root]; }
    for (int cur = x, cp = acc; cur != root && parent[cur] != root;) {
        int next = parent[cur];
        int next_cp = cp ^ par[cur];       /* parity of next relative to root */
        parent[cur] = root;
        par[cur] = (unsigned char)cp;
        cur = next;
        cp = next_cp;
    }
    *p = acc;
    return root;
}

/* Returns 1 if edge (u,v) keeps the graph bipartite (and inserts it), else 0. */
static int add_edge(int u, int v) {
    int pu, pv;
    int ru = find(u, &pu), rv = find(v, &pv);
    if (ru == rv) return pu != pv;         /* same side => odd cycle */
    if (parent[ru] > parent[rv]) { int t = ru; ru = rv; rv = t; } /* ru larger */
    parent[ru] += parent[rv];
    parent[rv] = ru;
    par[rv] = (unsigned char)(pu ^ pv ^ 1); /* put u and v on opposite sides */
    return 1;
}

/* ------------------------------ I/O ------------------------------------ */
static int interactive;
static int pushback = -2;
static char ibuf[1 << 16];
static size_t ilen, ipos;

static int gc(void) {
    if (pushback != -2) { int c = pushback; pushback = -2; return c; }
    if (interactive) return getchar();     /* never wait for a full buffer */
    if (ipos == ilen) {
        ilen = fread(ibuf, 1, sizeof ibuf, stdin);
        ipos = 0;
        if (ilen == 0) return EOF;
    }
    return (unsigned char)ibuf[ipos++];
}
static void ungc(int c) { pushback = c; }

/* Reads the next integer (optional '-'); returns 0 at EOF. Saturates on overflow. */
static int read_int(long long *out) {
    int c, neg = 0;
    for (;;) {
        c = gc();
        if (c == EOF) return 0;
        if (c >= '0' && c <= '9') break;
        if (c == '-') {
            int d = gc();
            if (d >= '0' && d <= '9') { neg = 1; c = d; break; }
            ungc(d);
        }
    }
    long long v = 0;
    for (; c >= '0' && c <= '9'; c = gc())
        v = v < (LLONG_MAX - 9) / 10 ? v * 10 + (c - '0') : LLONG_MAX / 2;
    ungc(c);                               /* keep the terminator for the caller */
    *out = neg ? -v : v;
    return 1;
}

/* True if another number follows on the current line (does not cross '\n'). */
static int more_on_line(void) {
    int c;
    while ((c = gc()) == ' ' || c == '\t' || c == '\r') {}
    ungc(c);
    return c == '-' || (c >= '0' && c <= '9');
}

static char obuf[1 << 16];
static size_t olen;
static void emit(const char *s, size_t n) {
    if (interactive) { fwrite(s, 1, n, stdout); fflush(stdout); return; }
    if (olen + n > sizeof obuf) { fwrite(obuf, 1, olen, stdout); olen = 0; }
    memcpy(obuf + olen, s, n);
    olen += n;
}
#define EMIT(lit) emit(lit, sizeof(lit) - 1)

/* ------------------------------ main ----------------------------------- */
int main(void) {
    interactive = IS_TTY(0);

    long long V, E = -1;                   /* E < 0: read edges until EOF */
    if (!read_int(&V)) return 0;
    if (more_on_line()) { long long e; if (read_int(&e)) E = e; }
    if (V <= 0 || V > INT_MAX) { fputs("invalid vertex count\n", stderr); return 1; }

    parent = malloc((size_t)V * sizeof *parent);
    par = calloc((size_t)V, sizeof *par);
    if (!parent || !par) {
        fputs("allocation failed\n", stderr);
        free(parent); free(par);
        return 1;
    }
    for (long long i = 0; i < V; i++) parent[i] = -1;

    long long a, b;
    for (long long k = 0; (E < 0 || k < E) && read_int(&a) && read_int(&b); k++) {
        if (a < 0 || b < 0 || a >= V || b >= V) EMIT("invalid\n");
        else if (add_edge((int)a, (int)b))      EMIT("added\n");
        else                                    EMIT("violates\n");
    }

    if (olen) fwrite(obuf, 1, olen, stdout);
    free(parent);
    free(par);
    return 0;
}