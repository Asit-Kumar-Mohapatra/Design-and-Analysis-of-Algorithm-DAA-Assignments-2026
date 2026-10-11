/*
 * Minimum Number of Meeting Rooms
 * -------------------------------
 * Input representation: the n intervals [s_i, e_i) are split into TWO flat arrays of 64-bit keys,
 *   S[i] = key(start_i)   and   E[i] = key(end_i),
 * which are sorted independently (LSD radix sort, O(n)) and then swept with two pointers.
 * We never need to know which start belongs to which end: a room is busy at time t exactly when
 * (#starts <= t) - (#ends <= t) meetings are running.
 *
 * Semantics (stated so there is no ambiguity):
 *   - a meeting occupies the half-open interval [s, e): a meeting ending at 5 and one starting at 5
 *     CAN share a room;
 *   - a zero-length meeting (s == e) occupies the single instant s (it needs a room, and clashes with
 *     any meeting whose [s',e') contains s, including one that starts at s);
 *   - times are integers with |t| <= 1e18; s > e is rejected as invalid input.
 *
 * Key encoding (makes every tie-break a plain integer comparison):
 *   start key = 2*s + 1
 *   end   key = 2*e      if s <  e      (a real end comes BEFORE a start at the same time)
 *             = 2*e + 1  if s == e      (a point meeting ends AFTER starts at the same time)
 *   Sweep: for each start in sorted order, retire every end whose key is strictly smaller,
 *          then ++busy; answer = max busy.
 *
 * Time  O(n)  for the sweep + O(n) radix sort (8 passes, empty passes skipped); comparison-sort
 *       model lower bound is Omega(n log n), radix sort beats it because keys are bounded integers.
 * Space O(n)  (S, E and one scratch array).
 *
 * Usage:   ./rooms [-a] < input     input is either   n  then n lines "s e"
 *                                   or the bracket form  [[s1,e1],[s2,e2],...]
 *                                   -a also prints one valid room assignment
 *          ./rooms test [N [seed]]  self-validation on N random/structured cases (default 10000)
 *          ./rooms bench N          timing on N random meetings
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef long long ll;
typedef unsigned long long ull;
#define TLIM 1000000000000000000LL

/* ------------------------------------------------------------------ keys */
static inline ull enc(ll k) { return (ull)k ^ 0x8000000000000000ULL; }   /* signed order -> unsigned order */
static inline ull start_key(ll s) { return enc(2 * s + 1); }
static inline ull end_key(ll s, ll e) { return enc(s < e ? 2 * e : 2 * e + 1); }

/* ------------------------------------------------------------ radix sort */
static void insertion_sort(ull *a, size_t n) {
    for (size_t i = 1; i < n; i++) {
        ull v = a[i]; size_t j = i;
        while (j > 0 && a[j - 1] > v) { a[j] = a[j - 1]; j--; }
        a[j] = v;
    }
}
static void radix_sort(ull *a, ull *tmp, size_t n) {
    if (n < 2) return;
    if (n <= 64) { insertion_sort(a, n); return; }
    size_t cnt[8][256];
    memset(cnt, 0, sizeof cnt);
    for (size_t i = 0; i < n; i++) {
        ull v = a[i];
        for (int b = 0; b < 8; b++) cnt[b][(v >> (8 * b)) & 255]++;
    }
    ull *src = a, *dst = tmp;
    for (int b = 0; b < 8; b++) {
        size_t *c = cnt[b];
        if (c[(src[0] >> (8 * b)) & 255] == n) continue;           /* all keys share this byte */
        size_t sum = 0;
        for (int k = 0; k < 256; k++) { size_t t = c[k]; c[k] = sum; sum += t; }
        for (size_t i = 0; i < n; i++) { ull v = src[i]; dst[c[(v >> (8 * b)) & 255]++] = v; }
        ull *t = src; src = dst; dst = t;
    }
    if (src != a) memcpy(a, src, n * sizeof(ull));
}

/* ------------------------------------------------------------- main algo */
/* precondition: s[i] <= e[i], |values| <= TLIM. Returns minimum number of rooms. */
static size_t min_rooms(const ll *s, const ll *e, size_t n) {
    if (n == 0) return 0;
    ull *S = (ull *)malloc(n * sizeof(ull));
    ull *E = (ull *)malloc(n * sizeof(ull));
    ull *T = (ull *)malloc(n * sizeof(ull));
    if (!S || !E || !T) { fprintf(stderr, "out of memory\n"); exit(3); }
    for (size_t i = 0; i < n; i++) { S[i] = start_key(s[i]); E[i] = end_key(s[i], e[i]); }
    radix_sort(S, T, n);
    radix_sort(E, T, n);
    size_t j = 0, busy = 0, best = 0;
    for (size_t i = 0; i < n; i++) {
        while (j < n && E[j] < S[i]) { j++; busy--; }
        busy++;
        if (busy > best) best = busy;
    }
    free(S); free(E); free(T);
    return best;
}

/* ---------------------- room assignment (min-heap greedy), used by -a and by the tests */
typedef struct { ull key; size_t idx; } KI;
static int cmp_ki(const void *a, const void *b) {
    const KI *p = a, *q = b;
    if (p->key != q->key) return p->key < q->key ? -1 : 1;
    return (p->idx > q->idx) - (p->idx < q->idx);
}
/* room[i] in [0, *nrooms). */
static void assign_rooms(const ll *s, const ll *e, size_t n, size_t *room, size_t *nrooms) {
    *nrooms = 0;
    if (n == 0) return;
    KI *ord = (KI *)malloc(n * sizeof(KI));
    ull *hk = (ull *)malloc(n * sizeof(ull));
    size_t *hr = (size_t *)malloc(n * sizeof(size_t));
    if (!ord || !hk || !hr) { fprintf(stderr, "out of memory\n"); exit(3); }
    for (size_t i = 0; i < n; i++) { ord[i].key = start_key(s[i]); ord[i].idx = i; }
    qsort(ord, n, sizeof(KI), cmp_ki);
    size_t m = 0;
    for (size_t t = 0; t < n; t++) {
        size_t i = ord[t].idx;
        ull nk = end_key(s[i], e[i]);
        size_t r;
        if (m > 0 && hk[0] < ord[t].key) {           /* earliest-finishing room is free: reuse it */
            r = hr[0];
            size_t p = 0;                             /* replace root, sift down */
            for (;;) {
                size_t l = 2 * p + 1, rr = l + 1, c = p;
                ull ck = nk;
                if (l < m && hk[l] < ck) { c = l; ck = hk[l]; }
                if (rr < m && hk[rr] < ck) { c = rr; ck = hk[rr]; }
                if (c == p) break;
                hk[p] = hk[c]; hr[p] = hr[c]; p = c;
            }
            hk[p] = nk; hr[p] = r;
        } else {                                      /* open a new room */
            r = (*nrooms)++;
            size_t p = m++;
            while (p > 0 && hk[(p - 1) / 2] > nk) { hk[p] = hk[(p - 1) / 2]; hr[p] = hr[(p - 1) / 2]; p = (p - 1) / 2; }
            hk[p] = nk; hr[p] = r;
        }
        room[i] = r;
    }
    free(ord); free(hk); free(hr);
}

/* ------------------------------------------------------------- input parsing */
static char *read_all(size_t *len) {
    size_t cap = 1 << 20, n = 0;
    char *buf = (char *)malloc(cap + 1);
    if (!buf) { fprintf(stderr, "out of memory\n"); exit(3); }
    for (;;) {
        size_t r = fread(buf + n, 1, cap - n, stdin);
        n += r;
        if (n < cap) break;
        cap *= 2;
        buf = (char *)realloc(buf, cap + 1);
        if (!buf) { fprintf(stderr, "out of memory\n"); exit(3); }
    }
    buf[n] = 0; *len = n; return buf;
}
/* returns 1 = got int, 0 = no more, -1 = bad (non-integer / out of range) */
static int next_int(const char **pp, const char *end, ll *out) {
    const char *p = *pp;
    while (p < end) {
        if (*p >= '0' && *p <= '9') break;
        if (*p == '-' && p + 1 < end && p[1] >= '0' && p[1] <= '9') break;
        p++;
    }
    if (p >= end) { *pp = p; return 0; }
    int neg = 0;
    if (*p == '-') { neg = 1; p++; }
    ull v = 0; int big = 0;
    while (p < end && *p >= '0' && *p <= '9') {
        if (v > (ull)TLIM) big = 1; else v = v * 10 + (ull)(*p - '0');
        p++;
    }
    *pp = p;
    if (p < end && (*p == '.' || *p == 'e' || *p == 'E')) return -1;   /* decimals unsupported */
    if (big || v > (ull)TLIM) return -1;
    *out = neg ? -(ll)v : (ll)v;
    return 1;
}
/* 0 ok, else error code (message printed) */
static int parse_input(const char *buf, size_t len, ll **ps, ll **pe, size_t *pn) {
    const char *p = buf, *end = buf + len;
    while (p < end && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) p++;
    int bracket = (p < end && *p == '[');
    size_t cap = 1024, cnt = 0;
    ll *vals = (ll *)malloc(cap * sizeof(ll));
    ll n_hdr = -1;
    int rc;
    if (!vals) { fprintf(stderr, "out of memory\n"); exit(3); }
    if (!bracket) {
        rc = next_int(&p, end, &n_hdr);
        if (rc == 0) { *ps = *pe = NULL; *pn = 0; free(vals); return 0; }     /* empty input */
        if (rc < 0 || n_hdr < 0) { fprintf(stderr, "error: invalid meeting count\n"); return 1; }
    }
    ll v;
    while ((rc = next_int(&p, end, &v)) != 0) {
        if (rc < 0) { fprintf(stderr, "error: times must be integers with |t| <= 1e18\n"); return 1; }
        if (cnt == cap) { cap *= 2; vals = (ll *)realloc(vals, cap * sizeof(ll)); if (!vals) { fprintf(stderr, "out of memory\n"); exit(3); } }
        vals[cnt++] = v;
        if (!bracket && cnt == 2 * (size_t)n_hdr) break;
    }
    size_t n;
    if (bracket) {
        if (cnt % 2) { fprintf(stderr, "error: odd number of values\n"); return 1; }
        n = cnt / 2;
    } else {
        if (cnt < 2 * (size_t)n_hdr) { fprintf(stderr, "error: expected %lld intervals, found %zu\n", n_hdr, cnt / 2); return 1; }
        n = (size_t)n_hdr;
    }
    ll *s = (ll *)malloc((n ? n : 1) * sizeof(ll)), *e = (ll *)malloc((n ? n : 1) * sizeof(ll));
    if (!s || !e) { fprintf(stderr, "out of memory\n"); exit(3); }
    for (size_t i = 0; i < n; i++) {
        s[i] = vals[2 * i]; e[i] = vals[2 * i + 1];
        if (s[i] > e[i]) { fprintf(stderr, "error: interval %zu has start > end (%lld > %lld)\n", i + 1, s[i], e[i]); return 1; }
    }
    free(vals);
    *ps = s; *pe = e; *pn = n;
    return 0;
}

/* ===================================================================== tests */
static ull rs = 88172645463325252ULL;
static ull rnd(void) { rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; return rs; }
static ll rr_(ll lo, ll hi) {
    ull span = (ull)hi - (ull)lo + 1;
    if (span == 0) return (ll)rnd();
    return lo + (ll)(rnd() % span);
}

/* independent method 1: O(n^2) from the definition (max over start points of #sets containing it) */
static size_t oracle(const ll *s, const ll *e, size_t n) {
    size_t best = 0;
    for (size_t a = 0; a < n; a++) {
        ll t = s[a]; size_t c = 0;
        for (size_t i = 0; i < n; i++) {
            int in = (s[i] < e[i]) ? (s[i] <= t && t < e[i]) : (t == s[i]);
            c += in;
        }
        if (c > best) best = c;
    }
    return best;
}
/* independent method 2: classic event sweep with a comparison sort (qsort) */
typedef struct { ll t; int ord; } Ev;
static int cmp_ev(const void *a, const void *b) {
    const Ev *p = a, *q = b;
    if (p->t != q->t) return p->t < q->t ? -1 : 1;
    return p->ord - q->ord;
}
static size_t event_sweep(const ll *s, const ll *e, size_t n) {
    Ev *ev = (Ev *)malloc((2 * n + 1) * sizeof(Ev));
    size_t m = 0;
    for (size_t i = 0; i < n; i++) {
        ev[m].t = s[i]; ev[m++].ord = 1;                          /* start */
        ev[m].t = e[i]; ev[m++].ord = (s[i] < e[i]) ? 0 : 2;      /* real end first, point end last */
    }
    qsort(ev, m, sizeof(Ev), cmp_ev);
    long cur = 0, best = 0;
    for (size_t i = 0; i < m; i++) {
        cur += (ev[i].ord == 1) ? 1 : -1;
        if (cur > best) best = cur;
    }
    free(ev);
    return (size_t)best;
}
/* do meetings a and b clash (their occupied sets intersect)? */
static int conflict(ll s1, ll e1, ll s2, ll e2) {
    if (s1 < e1 && s2 < e2) return (s1 > s2 ? s1 : s2) < (e1 < e2 ? e1 : e2);
    if (s1 == e1 && s2 == e2) return s1 == s2;
    if (s1 == e1) return s2 <= s1 && s1 < e2;
    return s1 <= s2 && s2 < e1;
}
static const ll *gs_, *ge_; static const size_t *gr_;
static int cmp_room(const void *a, const void *b) {
    size_t x = *(const size_t *)a, y = *(const size_t *)b;
    if (gr_[x] != gr_[y]) return gr_[x] < gr_[y] ? -1 : 1;
    if (gs_[x] != gs_[y]) return gs_[x] < gs_[y] ? -1 : 1;
    if (ge_[x] != ge_[y]) return ge_[x] < ge_[y] ? -1 : 1;
    return (x > y) - (x < y);
}
static int valid_assignment(const ll *s, const ll *e, size_t n, const size_t *room, size_t k) {
    for (size_t i = 0; i < n; i++) if (room[i] >= k) return 0;
    if (n <= 400) {                                              /* full pairwise check */
        for (size_t i = 0; i < n; i++)
            for (size_t j = i + 1; j < n; j++)
                if (room[i] == room[j] && conflict(s[i], e[i], s[j], e[j])) return 0;
        return 1;
    }
    size_t *ix = (size_t *)malloc(n * sizeof(size_t));
    for (size_t i = 0; i < n; i++) ix[i] = i;
    gs_ = s; ge_ = e; gr_ = room;
    qsort(ix, n, sizeof(size_t), cmp_room);
    int ok = 1;
    for (size_t t = 0; t + 1 < n && ok; t++) {
        size_t a = ix[t], b = ix[t + 1];
        if (room[a] == room[b] && conflict(s[a], e[a], s[b], e[b])) ok = 0;
    }
    free(ix);
    return ok;
}

static void shuffle_pairs(ll *s, ll *e, size_t n) {
    for (size_t i = n; i > 1; i--) {
        size_t j = (size_t)(rnd() % i);
        ll t = s[i - 1]; s[i - 1] = s[j]; s[j] = t;
        t = e[i - 1]; e[i - 1] = e[j]; e[j] = t;
    }
}

static int check_case(const ll *s, const ll *e, size_t n, int use_oracle, const char *tag) {
    size_t k = min_rooms(s, e, n), kq = event_sweep(s, e, n), kh = 0, ko = 0;
    size_t *room = (size_t *)malloc((n ? n : 1) * sizeof(size_t));
    assign_rooms(s, e, n, room, &kh);
    int ok = 1;
    const char *why = "";
    if (k != kq) { ok = 0; why = "radix-sweep != event-sweep"; }
    else if (k != kh) { ok = 0; why = "answer != heap room count"; }
    else if (!valid_assignment(s, e, n, room, kh)) { ok = 0; why = "heap assignment has a clash"; }
    else if (use_oracle && (ko = oracle(s, e, n)) != k) { ok = 0; why = "answer != O(n^2) oracle"; }
    else if (n >= 1 && (k < 1 || k > n)) { ok = 0; why = "answer out of [1,n]"; }
    else if (n == 0 && k != 0) { ok = 0; why = "empty input must give 0"; }
    if (ok && n > 1) {                                           /* order independence */
        ll *s2 = (ll *)malloc(n * sizeof(ll)), *e2 = (ll *)malloc(n * sizeof(ll));
        memcpy(s2, s, n * sizeof(ll)); memcpy(e2, e, n * sizeof(ll));
        shuffle_pairs(s2, e2, n);
        if (min_rooms(s2, e2, n) != k) { ok = 0; why = "not permutation invariant"; }
        free(s2); free(e2);
    }
    if (!ok) {
        printf("FAIL [%s] n=%zu fast=%zu events=%zu heap=%zu oracle=%zu : %s\n", tag, n, k, kq, kh, ko, why);
        if (n <= 12) for (size_t i = 0; i < n; i++) printf("   [%lld,%lld]\n", s[i], e[i]);
    }
    free(room);
    return ok;
}

/* instance generators -------------------------------------------------------- */
static const char *gen_case(int mode, size_t n, ll *s, ll *e) {
    static const ll special[] = { -TLIM, -TLIM + 1, -1, 0, 1, TLIM - 1, TLIM, -TLIM / 2, TLIM / 2 };
    ll R = 1 + (ll)(rnd() % 40), L = 1 + (ll)(rnd() % 12);
    switch (mode) {
    case 0: for (size_t i = 0; i < n; i++) { s[i] = rr_(0, R); e[i] = s[i] + rr_(0, L); } return "ties+zero-len";
    case 1: for (size_t i = 0; i < n; i++) { s[i] = rr_(0, R); e[i] = s[i] + 1 + rr_(0, L); } return "positive";
    case 2: { ll a = rr_(-50, 50), b = a + 1 + rr_(0, 50); for (size_t i = 0; i < n; i++) { s[i] = a; e[i] = b; } return "identical"; }
    case 3: { ll d = 1 + rr_(0, 9), o = rr_(-100, 100); for (size_t i = 0; i < n; i++) { s[i] = o + (ll)i * d; e[i] = o + (ll)(i + 1) * d; } return "touching chain"; }
    case 4: for (size_t i = 0; i < n; i++) { s[i] = (ll)i; e[i] = (ll)(2 * n - i); } return "nested";
    case 5: for (size_t i = 0; i < n; i++) { s[i] = e[i] = rr_(0, 3); } return "all zero-length";
    case 6: for (size_t i = 0; i < n; i++) { ll a = rr_(-1000, 1000), b = rr_(-1000, 1000); s[i] = a < b ? a : b; e[i] = a < b ? b : a; } return "random endpoints";
    case 7: for (size_t i = 0; i < n; i++) { ll a = special[rnd() % 9], b = special[rnd() % 9]; s[i] = a < b ? a : b; e[i] = a < b ? b : a; } return "extreme values";
    case 8: for (size_t i = 0; i < n; i++) { ll a = rr_(-TLIM, TLIM), b = rr_(-TLIM, TLIM); s[i] = a < b ? a : b; e[i] = a < b ? b : a; } return "huge random";
    case 9: for (size_t i = 0; i < n; i++) { s[i] = (ll)(i / 3); e[i] = (ll)(i / 3) + 1 + (ll)(i % 3); } return "staggered";
    case 10: for (size_t i = 0; i < n; i++) { s[i] = rr_(0, 5); e[i] = s[i] + (rnd() % 3 == 0 ? 0 : rr_(1, 3)); } return "mixed point/real";
    default: { ll sh = rr_(-TLIM / 4, TLIM / 4); for (size_t i = 0; i < n; i++) { s[i] = sh + rr_(0, R); e[i] = s[i] + rr_(0, L); } return "shifted"; }
    }
}

static int test_radix(void) {
    int bad = 0;
    for (int t = 0; t < 400; t++) {
        size_t n = (t < 50) ? (size_t)t : (size_t)(rnd() % 6000);
        ull *a = malloc((n + 1) * sizeof(ull)), *b = malloc((n + 1) * sizeof(ull)), *tmp = malloc((n + 1) * sizeof(ull));
        int kind = t % 5;
        for (size_t i = 0; i < n; i++)
            a[i] = kind == 0 ? rnd() : kind == 1 ? rnd() % 7 : kind == 2 ? 42 : kind == 3 ? (rnd() & 0xFF) << 56 : ~0ULL - rnd() % 3;
        memcpy(b, a, n * sizeof(ull));
        radix_sort(a, tmp, n);
        for (size_t i = 0; i + 1 < n; i++) if (a[i] > a[i + 1]) bad++;
        ull sa = 0, sb = 0, xa = 0, xb = 0;                       /* multiset preserved */
        for (size_t i = 0; i < n; i++) { sa += a[i]; sb += b[i]; xa ^= a[i] * 0x9E3779B97F4A7C15ULL; xb ^= b[i] * 0x9E3779B97F4A7C15ULL; }
        if (sa != sb || xa != xb) bad++;
        free(a); free(b); free(tmp);
    }
    return bad;
}

static int run_tests(long N) {
    int fails = 0;
    long counts[6] = {0};
    fails += test_radix() ? 1 : 0;
    if (fails) printf("FAIL radix sort unit test\n");
    for (long t = 0; t < N; t++) {
        int cat; size_t n;
        double f = (double)t / (double)N;
        if (f < 0.35) { cat = 0; n = (size_t)(rnd() % 13); }
        else if (f < 0.60) { cat = 1; n = 1 + (size_t)(rnd() % 60); }
        else if (f < 0.75) { cat = 2; n = 1 + (size_t)(rnd() % 500); }
        else if (f < 0.85) { cat = 3; n = 1 + (size_t)(rnd() % 40); }
        else if (f < 0.95) { cat = 4; n = 200 + (size_t)(rnd() % 1800); }
        else { cat = 5; n = 10000 + (size_t)(rnd() % 190000); }
        counts[cat]++;
        int mode = (int)(rnd() % 12);
        if (cat == 3) mode = (rnd() & 1) ? 7 : 8;
        ll *s = (ll *)malloc((n + 1) * sizeof(ll)), *e = (ll *)malloc((n + 1) * sizeof(ll));
        const char *tag = gen_case(mode, n, s, e);
        if (mode == 8 || mode == 7) { if (cat != 3) { /* keep ordering variety */ } }
        shuffle_pairs(s, e, n);
        if (!check_case(s, e, n, n <= 2000, tag)) fails++;
        free(s); free(e);
    }
    printf("tests: %ld  (tiny %ld, small %ld, structured %ld, extremes %ld, medium+oracle %ld, large %ld)\n",
           N, counts[0], counts[1], counts[2], counts[3], counts[4], counts[5]);
    printf("failures: %d\n", fails);
    return fails;
}

static int run_bench(size_t n) {
    ll *s = (ll *)malloc(n * sizeof(ll)), *e = (ll *)malloc(n * sizeof(ll));
    for (size_t i = 0; i < n; i++) { s[i] = rr_(0, 1000000000LL); e[i] = s[i] + rr_(1, 1000000); }
    clock_t t0 = clock(); size_t k = min_rooms(s, e, n); double t1 = (double)(clock() - t0) / CLOCKS_PER_SEC;
    t0 = clock(); size_t k2 = event_sweep(s, e, n); double t2 = (double)(clock() - t0) / CLOCKS_PER_SEC;
    printf("n=%zu rooms=%zu (radix two-pointer: %.3fs)   qsort event sweep: rooms=%zu (%.3fs)\n", n, k, t1, k2, t2);
    free(s); free(e);
    return k != k2;
}

/* ===================================================================== main */
int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "test") == 0) {
        long N = argc > 2 ? atol(argv[2]) : 10000;
        rs = 88172645463325252ULL + (argc > 3 ? strtoull(argv[3], 0, 10) * 7919ULL : 0ULL);
        return run_tests(N) ? 1 : 0;
    }
    if (argc > 2 && strcmp(argv[1], "bench") == 0) return run_bench((size_t)atoll(argv[2]));
    int want_assign = (argc > 1 && strcmp(argv[1], "-a") == 0);
    size_t len; char *buf = read_all(&len);
    ll *s = NULL, *e = NULL; size_t n = 0;
    if (parse_input(buf, len, &s, &e, &n)) return 2;
    free(buf);
    size_t k = min_rooms(s, e, n);
    printf("%zu\n", k);
    if (want_assign && n) {
        size_t *room = (size_t *)malloc(n * sizeof(size_t)), kk;
        assign_rooms(s, e, n, room, &kk);
        for (size_t i = 0; i < n; i++) printf("[%lld, %lld] -> room %zu\n", s[i], e[i], room[i] + 1);
        free(room);
    }
    free(s); free(e);
    return 0;
}