/* Bidirectional BFS on the implicit word graph (shortest transformation length).
 * Input : N L, then N dictionary words, then A B.
 *         (A need not be in D; B must be in D.)
 * Output: minimum number of mutations, or -1 if impossible.
 * Words that are not exactly L lowercase letters are ignored in D;
 * an invalid A or B gives -1.
 * Time O(N*L*26) expected (rolling hash, O(1) lookup), space O(N*L). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAXL 64
#define TOKMAX 4096
#define NLIMIT 100000000L

static int L;
static char (*W)[MAXL + 1];
static uint64_t PW[MAXL];
static int *tab;
static size_t tmask;

static int valid(const char *s) {
    if (strlen(s) != (size_t)L) return 0;
    for (int i = 0; i < L; i++)
        if (s[i] < 'a' || s[i] > 'z') return 0;
    return 1;
}
static uint64_t whash(const char *s) {
    uint64_t h = 0;
    for (int i = 0; i < L; i++) h += (uint64_t)(s[i] - 'a' + 1) * PW[i];
    return h;
}
static size_t slot(uint64_t h) {
    h ^= h >> 29; h *= 0xbf58476d1ce4e5b9ULL; h ^= h >> 32;
    return (size_t)h & tmask;
}
static int find(uint64_t h, const char *s) { /* node index or -1 */
    size_t p = slot(h);
    while (tab[p] >= 0) {
        if (memcmp(W[tab[p]], s, (size_t)L) == 0) return tab[p];
        p = (p + 1) & tmask;
    }
    return -1;
}
static void insert(uint64_t h, int id) {
    size_t p = slot(h);
    while (tab[p] >= 0) p = (p + 1) & tmask;
    tab[p] = id;
}

int main(void) {
    long nl;
    static char tok[TOKMAX], a[TOKMAX], b[TOKMAX];
    if (scanf("%ld %d", &nl, &L) != 2 || nl < 0 || nl > NLIMIT || L < 1 || L > MAXL) {
        fputs("bad header\n", stderr);
        return 1;
    }
    int n = (int)nl;
    size_t cap = 4;
    while (cap < 2 * ((size_t)n + 1)) cap <<= 1;
    tmask = cap - 1;
    W = malloc(((size_t)n + 1) * sizeof *W);
    tab = malloc(cap * sizeof *tab);
    int *side = calloc((size_t)n + 1, sizeof *side);
    int *dist = calloc((size_t)n + 1, sizeof *dist);
    int *fa = malloc(((size_t)n + 1) * sizeof *fa);
    int *fb = malloc(((size_t)n + 1) * sizeof *fb);
    int *nx = malloc(((size_t)n + 1) * sizeof *nx);
    if (!W || !tab || !side || !dist || !fa || !fb || !nx) {
        fputs("out of memory\n", stderr);
        return 1;
    }
    memset(tab, -1, cap * sizeof *tab);
    PW[0] = 1;
    for (int i = 1; i < MAXL; i++) PW[i] = PW[i - 1] * 1000003ULL;

    int cnt = 0;
    for (int i = 0; i < n; i++) {
        if (scanf("%4095s", tok) != 1) break; /* short input: A/B check below reports it */
        if (!valid(tok)) continue;
        uint64_t h = whash(tok);
        if (find(h, tok) >= 0) continue; /* duplicate */
        memcpy(W[cnt], tok, (size_t)L + 1);
        insert(h, cnt++);
    }
    int ans = -1, rc = 0;
    if (scanf("%4095s %4095s", a, b) != 2) {
        fputs("missing A/B\n", stderr);
        rc = 1;
    } else if (valid(a) && valid(b)) {
        int ib = find(whash(b), b);
        if (ib < 0) {
            ans = -1;
        } else if (strcmp(a, b) == 0) {
            ans = 0;
        } else {
            int ia = find(whash(a), a);
            if (ia < 0) { /* A not in D: extra start node */
                memcpy(W[cnt], a, (size_t)L + 1);
                ia = cnt++;
                insert(whash(a), ia);
            }
            int *F = fa, *B = fb, nf = 1, nb = 1;
            F[0] = ia; B[0] = ib;
            side[ia] = 1; side[ib] = 2;
            while (nf > 0 && nb > 0 && ans < 0) {
                int fwd = nf <= nb; /* expand the smaller frontier */
                int *cur = fwd ? F : B;
                int cn = fwd ? nf : nb, mine = fwd ? 1 : 2, other = 3 - mine;
                int m = 0, best = -1;
                for (int k = 0; k < cn; k++) {
                    int u = cur[k];
                    char s[MAXL + 1];
                    memcpy(s, W[u], (size_t)L + 1);
                    uint64_t h = whash(s);
                    for (int p = 0; p < L; p++) {
                        char o = s[p];
                        for (char c = 'a'; c <= 'z'; c++) {
                            if (c == o) continue;
                            s[p] = c;
                            int v = find(h + (uint64_t)(c - o) * PW[p], s);
                            if (v < 0) continue;
                            if (side[v] == other) { /* frontiers meet */
                                int t = dist[u] + 1 + dist[v];
                                if (best < 0 || t < best) best = t;
                            } else if (side[v] == 0) {
                                side[v] = mine;
                                dist[v] = dist[u] + 1;
                                nx[m++] = v;
                            }
                        }
                        s[p] = o;
                    }
                }
                if (best >= 0) { ans = best; break; } /* whole level scanned: minimum */
                memcpy(cur, nx, (size_t)m * sizeof *nx);
                if (fwd) nf = m; else nb = m;
            }
        }
    }
    if (rc == 0) printf("%d\n", ans);
    free(W); free(tab); free(side); free(dist); free(fa); free(fb); free(nx);
    return rc;
}