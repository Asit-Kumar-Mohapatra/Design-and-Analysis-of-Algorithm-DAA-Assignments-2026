/*
 * Fractional Knapsack with Deterioration Rate
 * -------------------------------------------
 * Model: items are consumed one weight-unit per time-unit (time = weight consumed so far).
 * A unit of item i consumed at time t is worth  d_i - lambda_i * t   (d_i = v_i / w_i).
 * Total consumed weight <= W (stopping early is allowed).
 *
 * Algorithm (see write-up):
 *   1. Sort items by decay rate lambda DESCENDING  (exchange argument, independent of values).
 *   2. Blocks are contiguous; with c_i = end time of block i the objective becomes
 *          sum_i g_i(c_i),  g_i(t) = (d_i-d_{i+1}) t - (lam_i-lam_{i+1}) t^2/2   (concave)
 *      subject to 0=c_0 <= c_1 <= ... <= c_n <= W,  c_i - c_{i-1} <= w_i.
 *   3. DP over items keeping h_i'(c) (piecewise linear, non-increasing) in a lazy treap:
 *      "sliding-window max" = insert a zero-slope piece of length w_i at the peak and
 *      shift the right part by w_i; then add g_i' to everything (lazy tag).
 *   4. Back-track with c_{i-1} = clamp(peak_{i-1}, c_i - w_i, c_i).
 *
 * Time  O(n log n) expected, Space O(n).
 *
 * Usage:   ./knap < input          (n W, then n lines: v w lambda)
 *          ./knap test [N]         (self-validation on N random tests, default 10000)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

typedef struct { double d, w, lam, v; int id; } Item;

static int cmp_item(const void *a, const void *b) {
    const Item *p = a, *q = b;
    if (p->lam != q->lam) return (p->lam > q->lam) ? -1 : 1;
    if (p->d != q->d) return (p->d > q->d) ? -1 : 1;
    return p->id - q->id;
}

/* ===================== lazy treap on the pieces of h' ===================== */
typedef struct {
    double X, L, A, B0;        /* start, length, derivative at start, slope base (slope = B0 - Cb) */
    double tsh, tca, tce;      /* lazy: X += tsh ; A += tca + tce * X_old */
    int l, r;
    unsigned pr;
} Node;

static Node *T;
static int ncnt;
static double Cb;
static unsigned long long rs = 88172645463325252ULL;

static unsigned rnd(void) {
    rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17;
    return (unsigned)(rs >> 32);
}
static int newnode(double X, double L, double A, double B0) {
    Node *p = &T[++ncnt];
    p->X = X; p->L = L; p->A = A; p->B0 = B0;
    p->tsh = p->tca = p->tce = 0; p->l = p->r = 0; p->pr = rnd();
    return ncnt;
}
static void app(int t, double sh, double ca, double ce) {
    if (!t) return;
    Node *p = &T[t];
    p->A += ca + ce * p->X;
    p->X += sh;
    p->tca += ca + ce * p->tsh;
    p->tsh += sh;
    p->tce += ce;
}
static void push(int t) {
    Node *p = &T[t];
    if (p->tsh != 0 || p->tca != 0 || p->tce != 0) {
        app(p->l, p->tsh, p->tca, p->tce);
        app(p->r, p->tsh, p->tca, p->tce);
        p->tsh = p->tca = p->tce = 0;
    }
}
static int merge(int a, int b) {
    if (!a) return b;
    if (!b) return a;
    if (T[a].pr > T[b].pr) { push(a); T[a].r = merge(T[a].r, b); return a; }
    push(b); T[b].l = merge(a, T[b].l); return b;
}
/* left = pieces whose start-derivative A > 0 */
static void split_pos(int t, int *a, int *b) {
    if (!t) { *a = *b = 0; return; }
    push(t);
    if (T[t].A > 0) { *a = t; split_pos(T[t].r, &T[t].r, b); }
    else            { *b = t; split_pos(T[t].l, a, &T[t].l); }
}
/* left = pieces with start X < W */
static void split_x(int t, double W, int *a, int *b) {
    if (!t) { *a = *b = 0; return; }
    push(t);
    if (T[t].X < W) { *a = t; split_x(T[t].r, W, &T[t].r, b); }
    else            { *b = t; split_x(T[t].l, W, a, &T[t].l); }
}
static int pop_last(int t, int *last) {
    push(t);
    if (T[t].r) { T[t].r = pop_last(T[t].r, last); return t; }
    *last = t; int l = T[t].l; T[t].l = 0; return l;
}
/* locate peak of h (argmax); returns peak, splits tree into Lt | Rt around it */
static double peak_split(int root, int *Lt, int *Rt) {
    int a, b;
    split_pos(root, &a, &b);
    if (!a) { *Lt = 0; *Rt = b; return 0.0; }
    int P; a = pop_last(a, &P);
    Node *p = &T[P];
    double Bc = p->B0 - Cb;
    double E = p->A + Bc * p->L;
    if (E > 0) { *Lt = merge(a, P); *Rt = b; return p->X + p->L; }
    double len = (Bc < 0) ? p->A / (-Bc) : 0.0;
    if (len < 0) len = 0;
    if (len > p->L) len = p->L;
    double p0 = p->X + len;
    int Q = newnode(p0, p->L - len, 0.0, p->B0);
    p->L = len;
    *Lt = merge(a, P);
    *Rt = merge(Q, b);
    return p0;
}

/* items must be sorted by lambda descending. x[i] = amount consumed of sorted item i. */
static void solve_fast(const Item *it, int n, double W, double *x) {
    if (n == 0) return;
    T = (Node *)malloc((size_t)(2 * n + 6) * sizeof(Node));
    double *pk = (double *)malloc((size_t)(n + 1) * sizeof(double));
    ncnt = 0; Cb = 0.0;
    int root = 0;
    for (int i = 0; i < n; i++) {
        double w = it[i].w;
        int Lt, Rt;
        pk[i] = peak_split(root, &Lt, &Rt);
        app(Rt, w, 0, 0);                              /* shift right part by w */
        int Z = newnode(pk[i], w, 0.0, Cb);            /* zero-slope plateau   */
        root = merge(merge(Lt, Z), Rt);
        int a, b;                                      /* clip domain to [0,W] */
        split_x(root, W, &a, &b);
        root = a;
        if (a) {
            int t = a;
            while (T[t].r) { push(t); t = T[t].r; }
            if (T[t].X + T[t].L > W) T[t].L = W - T[t].X;
        }
        double alpha = it[i].d - (i + 1 < n ? it[i + 1].d : 0.0);
        double beta  = it[i].lam - (i + 1 < n ? it[i + 1].lam : 0.0);
        app(root, 0, alpha, -beta);                    /* add g_i' lazily      */
        Cb += beta;
    }
    int Lt, Rt;
    pk[n] = peak_split(root, &Lt, &Rt);
    double c = pk[n];
    for (int i = n - 1; i >= 0; i--) {
        double lo = c - it[i].w, ci = pk[i];
        if (ci < lo) ci = lo;
        if (ci > c) ci = c;
        if (ci < 0) ci = 0;
        x[i] = c - ci;
        c = ci;
    }
    free(T); free(pk);
}

/* ===================== O(n^2) reference implementation ===================== */
typedef struct { double X, L, A, B; } Pc;

static void solve_ref(const Item *it, int n, double W, double *x) {
    if (n == 0) return;
    Pc *a = (Pc *)malloc((size_t)(2 * n + 6) * sizeof(Pc));
    Pc *b = (Pc *)malloc((size_t)(2 * n + 6) * sizeof(Pc));
    double *pk = (double *)malloc((size_t)(n + 1) * sizeof(double));
    int m = 0;
    for (int i = 0; i < n; i++) {
        double w = it[i].w, p = 0;
        int k, inside = 0;
        for (k = 0; k < m; k++) {
            if (a[k].A <= 0) { p = a[k].X; break; }
            double E = a[k].A + a[k].B * a[k].L;
            if (E <= 0) {
                double len = (a[k].B < 0) ? a[k].A / (-a[k].B) : 0.0;
                if (len < 0) len = 0;
                if (len > a[k].L) len = a[k].L;
                p = a[k].X + len; inside = 1; break;
            }
        }
        if (k == m) p = m ? a[m - 1].X + a[m - 1].L : 0.0;
        pk[i] = p;
        int mb = 0;
        for (int j = 0; j < k; j++) b[mb++] = a[j];
        int from = k;
        Pc right; int hasRight = 0;
        if (inside) {
            double len = p - a[k].X;
            b[mb++] = (Pc){a[k].X, len, a[k].A, a[k].B};
            right = (Pc){p, a[k].L - len, 0.0, a[k].B}; hasRight = 1;
            from = k + 1;
        }
        b[mb++] = (Pc){p, w, 0.0, 0.0};
        if (hasRight) { right.X += w; b[mb++] = right; }
        for (int j = from; j < m; j++) { Pc q = a[j]; q.X += w; b[mb++] = q; }
        int mc = 0;
        for (int j = 0; j < mb; j++) {
            if (b[j].X >= W) break;
            if (b[j].X + b[j].L > W) b[j].L = W - b[j].X;
            b[mc++] = b[j];
        }
        double alpha = it[i].d - (i + 1 < n ? it[i + 1].d : 0.0);
        double beta  = it[i].lam - (i + 1 < n ? it[i + 1].lam : 0.0);
        for (int j = 0; j < mc; j++) { b[j].A += alpha - beta * b[j].X; b[j].B -= beta; }
        Pc *tmp = a; a = b; b = tmp; m = mc;
    }
    {   /* final peak */
        double p = 0; int k;
        for (k = 0; k < m; k++) {
            if (a[k].A <= 0) { p = a[k].X; break; }
            double E = a[k].A + a[k].B * a[k].L;
            if (E <= 0) {
                double len = (a[k].B < 0) ? a[k].A / (-a[k].B) : 0.0;
                if (len < 0) len = 0;
                if (len > a[k].L) len = a[k].L;
                p = a[k].X + len; break;
            }
        }
        if (k == m) p = m ? a[m - 1].X + a[m - 1].L : 0.0;
        pk[n] = p;
    }
    double c = pk[n];
    for (int i = n - 1; i >= 0; i--) {
        double lo = c - it[i].w, ci = pk[i];
        if (ci < lo) ci = lo;
        if (ci > c) ci = c;
        if (ci < 0) ci = 0;
        x[i] = c - ci;
        c = ci;
    }
    free(a); free(b); free(pk);
}

/* ===================== value / validation helpers ===================== */
static double total_value(const Item *it, int n, const double *x) {
    double s = 0, val = 0;
    for (int i = 0; i < n; i++) {
        val += it[i].d * x[i] - it[i].lam * (s * x[i] + 0.5 * x[i] * x[i]);
        s += x[i];
    }
    return val;
}
/* value of an arbitrary schedule given as order perm[] and amounts amt[] (by original index in 'it') */
static double schedule_value(const Item *it, const int *perm, const double *amt, int n) {
    double s = 0, val = 0;
    for (int k = 0; k < n; k++) {
        int i = perm[k]; double a = amt[i];
        val += it[i].d * a - it[i].lam * (s * a + 0.5 * a * a);
        s += a;
    }
    return val;
}
static int feasible(const Item *it, int n, double W, const double *x) {
    double tot = 0;
    for (int i = 0; i < n; i++) {
        if (x[i] < -1e-9 || x[i] > it[i].w * (1 + 1e-12) + 1e-9) return 0;
        tot += x[i];
    }
    return tot <= W * (1 + 1e-12) + 1e-9;
}
/* first-order (KKT) optimality certificate for the concave QP in the sorted order */
static int kkt_ok(const Item *it, int n, double W, const double *x, double *viol) {
    double *suf = (double *)calloc((size_t)n + 2, sizeof(double));
    for (int i = n - 1; i >= 0; i--) suf[i] = suf[i + 1] + it[i].lam * x[i];
    double S = 0, Lmax = -INFINITY, Umin = INFINITY, tot = 0, scale = 1;
    double dmax = 0, lmax = 0, wsum = 0;
    for (int i = 0; i < n; i++) {
        if (fabs(it[i].d) > dmax) dmax = fabs(it[i].d);
        if (it[i].lam > lmax) lmax = it[i].lam;
        wsum += it[i].w;
    }
    scale = 1 + dmax + lmax * (W < wsum ? W : wsum);
    for (int m = 0; m < n; m++) {
        double g = it[m].d - it[m].lam * (S + x[m]) - suf[m + 1];
        double tolx = 1e-9 * (1 + it[m].w);
        if (x[m] < it[m].w - tolx && g > Lmax) Lmax = g;
        if (x[m] > tolx && g < Umin) Umin = g;
        S += x[m];
    }
    tot = S;
    double tg = 1e-6 * scale, tw = 1e-9 * (1 + W);
    int ok;
    double v;
    if (tot < W - tw) { v = fmax(Lmax - 0.0, 0.0 - Umin); ok = (Lmax <= tg && Umin >= -tg); }
    else              { v = fmax(Lmax, 0.0) - Umin;       ok = (fmax(Lmax, 0.0) <= Umin + tg); }
    if (viol) *viol = v;
    free(suf);
    return ok;
}

/* ---- discretised exact oracle (min-cost flow): slots of width 1/q, exact slot integrals ---- */
#define MAXE 8192
static int eto[MAXE], ecap[MAXE], enx[MAXE], ehead[256], ecnt;
static long long ecost[MAXE];
static void add_edge(int u, int v, int cap, long long cost) {
    eto[ecnt] = v; ecap[ecnt] = cap; ecost[ecnt] = cost; enx[ecnt] = ehead[u]; ehead[u] = ecnt++;
    eto[ecnt] = u; ecap[ecnt] = 0; ecost[ecnt] = -cost; enx[ecnt] = ehead[v]; ehead[v] = ecnt++;
}
static double oracle_discrete(const Item *it, int n, int Wint, int q) {
    int S = Wint * q, V = n + S + 2, src = n + S, snk = n + S + 1;
    ecnt = 0; for (int i = 0; i < V; i++) ehead[i] = -1;
    double dl = 1.0 / q;
    for (int i = 0; i < n; i++) add_edge(src, i, (int)llround(it[i].w * q), 0);
    for (int i = 0; i < n; i++)
        for (int k = 0; k < S; k++)
            add_edge(i, n + k, 1, -llround((it[i].d - it[i].lam * ((k + 0.5) * dl)) * dl * 1e12));
    for (int k = 0; k < S; k++) add_edge(n + k, snk, 1, 0);
    long long total = 0;
    static long long dist[256]; static int inq[256], pe[256], qu[100000];
    for (;;) {
        for (int i = 0; i < V; i++) { dist[i] = (long long)4e18; inq[i] = 0; pe[i] = -1; }
        int qh = 0, qt = 0; dist[src] = 0; qu[qt++] = src; inq[src] = 1;
        while (qh != qt) {
            int u = qu[qh++ % 100000]; inq[u] = 0;
            for (int e = ehead[u]; e != -1; e = enx[e])
                if (ecap[e] > 0 && dist[u] + ecost[e] < dist[eto[e]]) {
                    dist[eto[e]] = dist[u] + ecost[e]; pe[eto[e]] = e;
                    if (!inq[eto[e]]) { inq[eto[e]] = 1; qu[qt++ % 100000] = eto[e]; }
                }
        }
        if (dist[snk] >= 0) break;
        int f = 1 << 30;
        for (int v = snk; v != src; v = eto[pe[v] ^ 1]) if (ecap[pe[v]] < f) f = ecap[pe[v]];
        for (int v = snk; v != src; v = eto[pe[v] ^ 1]) { ecap[pe[v]] -= f; ecap[pe[v] ^ 1] += f; }
        total += dist[snk] * f;
    }
    return -(double)total / 1e12;
}

/* ===================== self-test driver ===================== */
static double urand(void) { return (rnd() + 0.5) / 4294967296.0; }
static int irand(int lo, int hi) { return lo + (int)(rnd() % (unsigned)(hi - lo + 1)); }

static void fill_item(Item *p, int id, double v, double w, double lam) {
    p->v = v; p->w = w; p->lam = lam; p->d = v / w; p->id = id;
}

static int run_tests(int N) {
    int fails = 0;
    long long cA = 0, cB = 0, cC = 0, cD = 0;
    double worst_rel = 0, worst_gap = 0, sum_gap = 0; long long gap_cnt = 0;
    double worst_kkt = 0;
    for (int tcase = 0; tcase < N; tcase++) {
        int grp = (tcase < N * 0.40) ? 0 : (tcase < N * 0.78) ? 1 : (tcase < N * 0.97) ? 2 : 3;
        int n, Wint = 0; double W;
        Item *it;
        int mode = irand(0, 5);           /* flavour of the instance */
        if (grp == 0) { n = irand(1, 6); cA++; }
        else if (grp == 1) { n = irand(1, 300); cB++; }
        else if (grp == 2) { n = irand(1, 40); cC++; }
        else { n = irand(2000, 20000); cD++; }
        it = (Item *)malloc((size_t)n * sizeof(Item));
        double wsum = 0;
        for (int i = 0; i < n; i++) {
            double w, v, lam;
            if (grp == 0) { w = irand(1, 6); v = irand(0, 40) * 0.5 + (mode == 1 ? 0 : urand());
                            lam = (mode == 0) ? irand(1, 3) * 0.5 : 0.05 + 3 * urand(); }
            else if (grp == 1) { w = (mode & 1) ? irand(1, 20) : 0.1 + 10 * urand();
                                 v = 100 * urand() * w; lam = (mode == 2) ? irand(1, 5) : 0.001 + 2 * urand(); }
            else if (grp == 2) {
                switch (mode) {
                  case 0: w = 1; v = 5; lam = 1; break;                       /* all identical   */
                  case 1: w = 1e-3 + urand(); v = 1e3 * urand(); lam = 1e-6 + 1e-5 * urand(); break;
                  case 2: w = 1e3 * urand() + 1; v = urand() * 1e-3; lam = 1e3 * urand(); break;
                  case 3: w = irand(1, 3); v = 0; lam = 0.5 + urand(); break; /* zero values    */
                  case 4: w = 1 + 1e6 * urand(); v = 1e6 * urand() * w; lam = 1e-3 + urand(); break;
                  default: w = irand(1, 4); v = irand(0, 50); lam = irand(1, 2); break;
                }
            } else { w = 0.5 + 20 * urand(); v = 50 * urand() * w; lam = 0.0001 + urand(); }
            fill_item(&it[i], i, v, w, lam);
            wsum += w;
        }
        if (grp == 0) {
            Wint = irand(0, 14); W = Wint;
        } else {
            int wm = irand(0, 6);
            W = (wm == 0) ? 0.0 : (wm == 1) ? 1e-9 : (wm == 2) ? wsum * 5 : (wm == 3) ? wsum * (0.05 + 0.9 * urand())
                : (wm == 4) ? wsum : 1e-3 + wsum * urand();
            if (grp == 2 && irand(0, 9) == 0) W = 1e12;
        }
        qsort(it, (size_t)n, sizeof(Item), cmp_item);
        double *xf = (double *)calloc((size_t)n, sizeof(double));
        double *xr = (double *)calloc((size_t)n, sizeof(double));
        solve_fast(it, n, W, xf);
        double vf = total_value(it, n, xf);
        int bad = 0;
        if (!feasible(it, n, W, xf)) { bad = 1; printf("[%d] infeasible\n", tcase); }
        double kv; if (!kkt_ok(it, n, W, xf, &kv)) { bad = 1; printf("[%d] KKT violated (%g) n=%d W=%g grp=%d mode=%d\n", tcase, kv, n, W, grp, mode); }
        if (isfinite(kv) && kv > worst_kkt) worst_kkt = kv;
        if (n <= 3000) {
            solve_ref(it, n, W, xr);
            double vr = total_value(it, n, xr);
            double rel = fabs(vf - vr) / (1 + fabs(vf));
            if (rel > worst_rel) worst_rel = rel;
            if (rel > 1e-8) { bad = 1; printf("[%d] fast!=ref %.12g vs %.12g\n", tcase, vf, vr); }
        }
        if (grp == 0) {
            int q = irand(2, 4);
            double orc = oracle_discrete(it, n, Wint, q);
            double M = 0; for (int i = 0; i < n; i++) M += fabs(it[i].d) + it[i].lam * Wint;
            double gap = vf - orc;
            if (gap < -1e-9 * (1 + fabs(vf))) { bad = 1; printf("[%d] oracle BEATS us: %.10g > %.10g\n", tcase, orc, vf); }
            if (gap > 4.0 * n * (1.0 / q) * (M + 1) ) { bad = 1; printf("[%d] gap too big %g\n", tcase, gap); }
            if (gap > worst_gap) worst_gap = gap;
            sum_gap += gap; gap_cnt++;
            /* random arbitrary schedules (any order, any amounts) must not beat the optimum */
            int *perm = (int *)malloc((size_t)n * sizeof(int)); double *amt = (double *)malloc((size_t)n * sizeof(double));
            for (int rep = 0; rep < 60 && !bad; rep++) {
                for (int i = 0; i < n; i++) perm[i] = i;
                for (int i = n - 1; i > 0; i--) { int j = irand(0, i); int t = perm[i]; perm[i] = perm[j]; perm[j] = t; }
                double rem = W;
                for (int k = 0; k < n; k++) { int i = perm[k]; double a = it[i].w * urand(); if (irand(0,3)==0) a = it[i].w; if (irand(0,4)==0) a = 0; if (a > rem) a = rem; amt[i] = a; rem -= a; }
                double sv = schedule_value(it, perm, amt, n);
                if (sv > vf + 1e-9 * (1 + fabs(vf))) { bad = 1; printf("[%d] random schedule beats us\n", tcase); }
                /* optimal amounts, random order */
                for (int i = 0; i < n; i++) amt[i] = xf[i];
                sv = schedule_value(it, perm, amt, n);
                if (sv > vf + 1e-9 * (1 + fabs(vf))) { bad = 1; printf("[%d] reordering beats us\n", tcase); }
            }
            free(perm); free(amt);
        }
        if (bad) fails++;
        free(it); free(xf); free(xr);
    }
    printf("tests: %d  (small/oracle %lld, medium %lld, degenerate %lld, large %lld)\n", N, cA, cB, cC, cD);
    printf("failures: %d\n", fails);
    printf("max rel diff fast vs O(n^2) ref: %.3g\n", worst_rel);
    printf("max KKT violation: %.3g\n", worst_kkt);
    printf("oracle gap (continuous opt - discrete opt): max %.4g, mean %.4g  (never negative => nothing beats us)\n", worst_gap, gap_cnt ? sum_gap / gap_cnt : 0);
    return fails;
}

/* ===================== main ===================== */
int main(int argc, char **argv) {
    rs ^= (unsigned long long)time(NULL) * 2654435761ULL;
    if (argc > 1 && strcmp(argv[1], "test") == 0) {
        rs = 88172645463325252ULL + (argc > 3 ? strtoull(argv[3], 0, 10) : 0ULL);  /* reproducible; optional seed */
        int N = (argc > 2) ? atoi(argv[2]) : 10000;
        return run_tests(N) ? 1 : 0;
    }
    int n; double W;
    if (scanf("%d %lf", &n, &W) != 2) return 0;
    Item *it = (Item *)malloc((size_t)(n > 0 ? n : 1) * sizeof(Item));
    int m = 0;
    for (int i = 0; i < n; i++) {
        double v, w, lam;
        if (scanf("%lf %lf %lf", &v, &w, &lam) != 3) break;
        if (w <= 0) continue;                     /* weightless items are meaningless here */
        fill_item(&it[m], i + 1, v, w, lam); m++;
    }
    n = m;
    if (W < 0) W = 0;
    qsort(it, (size_t)n, sizeof(Item), cmp_item);
    double *x = (double *)calloc((size_t)(n > 0 ? n : 1), sizeof(double));
    solve_fast(it, n, W, x);
    printf("Maximum total value: %.9f\n", total_value(it, n, x));
    printf("Schedule (decay-rate descending; skipped items omitted):\n");
    printf("%-6s %-14s %-14s %-10s %-14s\n", "item", "start_time", "amount", "fraction", "value_gained");
    double s = 0, used = 0;
    for (int i = 0; i < n; i++) {
        if (x[i] <= 1e-12) continue;
        double gain = it[i].d * x[i] - it[i].lam * (s * x[i] + 0.5 * x[i] * x[i]);
        printf("%-6d %-14.9f %-14.9f %-10.6f %-14.9f\n", it[i].id, s, x[i], x[i] / it[i].w, gain);
        s += x[i]; used += x[i];
    }
    printf("Total weight used: %.9f / %.9f\n", used, W);
    return 0;
}