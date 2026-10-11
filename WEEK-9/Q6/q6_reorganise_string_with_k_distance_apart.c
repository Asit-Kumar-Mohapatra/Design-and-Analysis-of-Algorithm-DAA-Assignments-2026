/*
 * Reorganise String with K-Distance Apart
 * ---------------------------------------
 * Input  : string S, integer K
 * Output : a rearrangement of S in which equal characters are at least K
 *          positions apart (index difference >= K), or "" if impossible.
 *
 * Input representation: S is read as a dynamically sized byte string; character
 * frequencies are kept in a 256-entry table (alphabet size SIGMA = 256), and a
 * max-heap over the distinct characters (<= 256 entries) drives the greedy.
 *
 * Greedy: at position i choose, among characters not used in the last K-1
 * positions, the one with the largest remaining count. The character placed at
 * i-K becomes available again at i, so out[] itself acts as the "cool-down queue".
 *
 * Time  : O(n log SIGMA) = O(n)   (SIGMA is a constant, 256)
 * Space : O(SIGMA) auxiliary + O(n) for the output
 *
 * Usage : ./reorganize        reads  S (line 1), K (line 2); prints answer
 *         ./reorganize -v     same, plus validation of the answer
 *         ./reorganize -t     runs a built-in randomized self-test (validation
 *                             of the algorithm against an exact feasibility
 *                             condition and an arrangement checker)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIGMA 256

static int cnt[SIGMA];
static int heap[SIGMA], hsz;

/* "a is better than b": larger count first, ties -> smaller char code */
static int better(int a, int b) {
    return cnt[a] > cnt[b] || (cnt[a] == cnt[b] && a < b);
}
static void push(int c) {
    int i = hsz++;
    while (i > 0) {
        int p = (i - 1) / 2;
        if (!better(c, heap[p])) break;
        heap[i] = heap[p];
        i = p;
    }
    heap[i] = c;
}
static int pop(void) {
    int top = heap[0], last = heap[--hsz], i = 0;
    while (1) {
        int l = 2 * i + 1, r = l + 1, m = l;
        if (l >= hsz) break;
        if (r < hsz && better(heap[r], heap[l])) m = r;
        if (!better(heap[m], last)) break;
        heap[i] = heap[m];
        i = m;
    }
    if (hsz > 0) heap[i] = last;
    return top;
}

/* Returns malloc'ed string (empty string if impossible). Caller frees. */
char *reorganize(const char *s, long long K) {
    size_t n = strlen(s);
    char *out = (char *)malloc(n + 1);
    if (!out) return NULL;
    if (K <= 1 || n <= 1) {            /* no restriction */
        memcpy(out, s, n + 1);
        return out;
    }
    memset(cnt, 0, sizeof cnt);
    hsz = 0;
    for (size_t i = 0; i < n; i++) cnt[(unsigned char)s[i]]++;

    /* quick feasibility test: (maxfreq-1)*K + (#chars with maxfreq) <= n */
    int mx = 0, t = 0;
    for (int c = 0; c < SIGMA; c++) {
        if (cnt[c] > mx) { mx = cnt[c]; t = 1; }
        else if (cnt[c] == mx && mx > 0) t++;
    }
    if ((long long)(mx - 1) * K + t > (long long)n) { out[0] = '\0'; return out; }

    for (int c = 0; c < SIGMA; c++) if (cnt[c] > 0) push(c);
    for (size_t i = 0; i < n; i++) {
        if ((long long)i >= K) {
            int c = (unsigned char)out[i - (size_t)K];
            if (cnt[c] > 0) push(c);
        }
        if (hsz == 0) { out[0] = '\0'; return out; }   /* safety net */
        int c = pop();
        out[i] = (char)c;
        cnt[c]--;
    }
    out[n] = '\0';
    return out;
}


/* ---------------- validation ---------------- */
/* 1 if o is a permutation of s with equal chars at least K apart */
static int valid_arrangement(const char *s, const char *o, long long K) {
    size_t n = strlen(s);
    if (strlen(o) != n) return 0;
    int a[SIGMA] = {0}, b[SIGMA] = {0};
    long long last[SIGMA];
    for (size_t i = 0; i < n; i++) { a[(unsigned char)s[i]]++; b[(unsigned char)o[i]]++; }
    if (memcmp(a, b, sizeof a)) return 0;
    for (int i = 0; i < SIGMA; i++) last[i] = -(1LL << 60);
    for (size_t i = 0; i < n; i++) {
        int c = (unsigned char)o[i];
        if (K > 0 && (long long)i - last[c] < K) return 0;
        last[c] = (long long)i;
    }
    return 1;
}
/* exact feasibility condition, computed independently of reorganize() */
static int feasible(const char *s, long long K) {
    size_t n = strlen(s);
    int c[SIGMA] = {0}, mx = 0, t = 0;
    if (n == 0 || K <= 1) return 1;
    for (size_t i = 0; i < n; i++) c[(unsigned char)s[i]]++;
    for (int i = 0; i < SIGMA; i++) {
        if (c[i] > mx) { mx = c[i]; t = 1; }
        else if (c[i] == mx && mx > 0) t++;
    }
    return (long long)(mx - 1) * K + t <= (long long)n;
}
static int self_test(void) {
    long tests = 0, fail = 0;
    srand(12345);
    for (int it = 0; it < 20000; it++) {
        int n = rand() % 60, sig = 1 + rand() % 8;
        long long K = rand() % 12;
        char s[64];
        for (int i = 0; i < n; i++) s[i] = (char)('a' + rand() % sig);
        s[n] = '\0';
        char *o = reorganize(s, K);
        int got = (o && (o[0] || n == 0));
        int ok = (got == feasible(s, K)) && (!got || valid_arrangement(s, o, K));
        if (!ok) { fail++; printf("FAIL: S=\"%s\" K=%lld out=\"%s\"\n", s, K, o ? o : ""); }
        tests++; free(o);
    }
    printf("Self-test: %ld cases, %ld failures -> %s\n", tests, fail, fail ? "FAIL" : "PASS");
    return fail != 0;
}

#ifndef NO_MAIN
int main(int argc, char **argv) {
    int verbose = (argc > 1 && strcmp(argv[1], "-v") == 0);
    if (argc > 1 && strcmp(argv[1], "-t") == 0) return self_test();
    /* Input format: line 1 = string S, line 2 = integer K */
    size_t cap = 1 << 16, len = 0;
    char *buf = (char *)malloc(cap);
    if (!buf) return 1;
    buf[0] = '\0';
    while (fgets(buf + len, (int)(cap - len), stdin)) {
        len += strlen(buf + len);
        if (len > 0 && buf[len - 1] == '\n') break;
        if (cap - len < 2 || len + 1 >= cap) {
            cap *= 2;
            char *nb = (char *)realloc(buf, cap);
            if (!nb) { free(buf); return 1; }
            buf = nb;
        }
    }
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) buf[--len] = '\0';
    long long K = 0;
    if (scanf("%lld", &K) != 1) K = 0;
    char *res = reorganize(buf, K);
    if (!res) { free(buf); return 1; }
    if (res[0]) puts(res);
    else puts("\"\"");              /* empty string */
    if (verbose) {
        int got = (res[0] != '\0') || len == 0;
        int ok = (got == feasible(buf, K)) && (!got || valid_arrangement(buf, res, K));
        printf("Validation: %s\n", ok ? "PASS" : "FAIL");
    }
    free(res); free(buf);
    return 0;
}
#endif