/*
 * Longest Common Subsequence (LCS): length + the actual subsequence
 * =================================================================
 * PORTABLE ANSI C (C89): no long long, no struct tricks, no compiler builtins.
 *
 * Input representation : each sequence is ONE LINE of characters (letters, digits,
 *                        symbols, spaces).  X has m characters, Y has n characters.
 *                        An empty line is a valid (empty) sequence.  '\r' is ignored.
 *                        Every other character (including spaces) is part of the sequence.
 *
 * Algorithm : Hirschberg's divide and conquer (linear-space version of the LCS DP)
 *   Recurrence   c[i][j] = c[i-1][j-1] + 1              if x_i == y_j
 *                          max(c[i-1][j], c[i][j-1])    otherwise
 *   Split X at its middle; a forward pass and a backward pass, each keeping ONE row,
 *   give the LCS lengths of both halves against every prefix/suffix of Y.  The best
 *   split point k of Y maximises F[k] + R[k].  Recurse on the two halves.
 *   Speed-up: a common prefix / suffix of X and Y always belongs to some LCS, so it
 *   is stripped first (O(m+n)) and only the differing middle is processed.
 *
 * Complexity (derivation)
 *   Time  : T(m,n) = mn/2 + mn/2 (two passes over both halves)  + T(m/2,k) + T(m/2,n-k)
 *           The two subproblems have total area  (m/2)*k + (m/2)*(n-k) = mn/2, so the
 *           work is  mn + mn/2 + mn/4 + ... <= 2mn   =>  O(m*n)  (about 2x the plain table).
 *   Space : two rows of length min(m,n)+2, the output buffer (<= min(m,n)), and a
 *           recursion depth of O(log m)  =>  O(min(m,n)) extra memory
 *           (+ O(m+n) to hold the input).  The classic table needs O(m*n) to rebuild the string.
 *
 * Output: length of the LCS, and one LCS string.
 *
 * Build : gcc -ansi -pedantic -O2 -Wall -Wextra -o lcs lcs.c
 * Test  : gcc -O2 -DTEST -o lcs_test lcs.c && ./lcs_test
 *
 * Output mode: prompts + labels when stdin is a terminal; otherwise (pipes / online
 * judges) prints only  "<length>\n<lcs>\n".  Force with -DPROMPTS=1 or -DPROMPTS=0.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__unix__) || defined(__unix) || defined(__APPLE__)
#include <unistd.h>
#define STDIN_IS_TTY() isatty(0)
#else
#define STDIN_IS_TTY() 1
#endif

static const char *A;   /* longer (or equal) sequence, split in halves       */
static const char *B;   /* shorter sequence, its length decides the row size */
static int *Lrow;       /* forward row  F[j]                                 */
static int *Rrow;       /* backward row R[j]                                 */
static char *out;       /* reconstructed LCS (middle part)                   */
static int outn;

/* F[j] = LCS( A[a0..mid) , B[b0..b0+j) ) for j = 0..nb */
static void forward(int a0, int mid, int b0, int nb)
{
    int i, j, diag, up;
    for (j = 0; j <= nb; j++) Lrow[j] = 0;
    for (i = a0; i < mid; i++) {
        diag = 0;
        for (j = 1; j <= nb; j++) {
            up = Lrow[j];
            if (A[i] == B[b0 + j - 1]) Lrow[j] = diag + 1;
            else if (Lrow[j - 1] > Lrow[j]) Lrow[j] = Lrow[j - 1];
            diag = up;
        }
    }
}

/* R[j] = LCS( A[mid..a1) , B[b0+j..b0+nb) ) for j = 0..nb */
static void backward(int mid, int a1, int b0, int nb)
{
    int i, j, diag, up;
    for (j = 0; j <= nb; j++) Rrow[j] = 0;
    for (i = a1 - 1; i >= mid; i--) {
        diag = 0;
        for (j = nb - 1; j >= 0; j--) {
            up = Rrow[j];
            if (A[i] == B[b0 + j]) Rrow[j] = diag + 1;
            else if (Rrow[j + 1] > Rrow[j]) Rrow[j] = Rrow[j + 1];
            diag = up;
        }
    }
}

/* appends an LCS of A[a0..a1) and B[b0..b1) to out */
static void hirschberg(int a0, int a1, int b0, int b1)
{
    int mid, k, best, bestk, j, nb;

    nb = b1 - b0;
    if (a1 == a0 || nb == 0) return;
    if (a1 - a0 == 1) {                       /* single character of A */
        for (j = b0; j < b1; j++)
            if (B[j] == A[a0]) { out[outn++] = A[a0]; return; }
        return;
    }
    mid = a0 + (a1 - a0) / 2;
    forward(a0, mid, b0, nb);
    backward(mid, a1, b0, nb);
    best = -1; bestk = 0;
    for (k = 0; k <= nb; k++)
        if (Lrow[k] + Rrow[k] > best) { best = Lrow[k] + Rrow[k]; bestk = k; }
    hirschberg(a0, mid, b0, b0 + bestk);
    hirschberg(mid, a1, b0 + bestk, b1);
}

/*
 * Computes one LCS of X[0..m) and Y[0..n).
 * result must have room for min(m,n)+1 chars; it is NUL-terminated.
 * Returns the LCS length, or -1 if memory could not be allocated.
 */
int lcs(const char *X, int m, const char *Y, int n, char *result)
{
    int p = 0, s = 0, la, lb, len;
    const char *MA, *MB;

    while (p < m && p < n && X[p] == Y[p]) p++;                       /* common prefix */
    while (s < m - p && s < n - p && X[m - 1 - s] == Y[n - 1 - s]) s++; /* common suffix */

    if (p > 0) memcpy(result, X, (size_t)p);

    if (m - p - s >= n - p - s) { MA = X + p; la = m - p - s; MB = Y + p; lb = n - p - s; }
    else                        { MA = Y + p; la = n - p - s; MB = X + p; lb = m - p - s; }
    A = MA; B = MB;

    Lrow = (int *)malloc((size_t)(lb + 2) * sizeof(int));
    Rrow = (int *)malloc((size_t)(lb + 2) * sizeof(int));
    if (!Lrow || !Rrow) { free(Lrow); free(Rrow); return -1; }
    out = result + p;
    outn = 0;
    hirschberg(0, la, 0, lb);
    free(Lrow); free(Rrow);

    if (s > 0) memcpy(result + p + outn, X + (m - s), (size_t)s);
    len = p + outn + s;
    result[len] = '\0';
    return len;
}

#ifdef TEST
/* ------------------------------ self-test ------------------------------ */
/* Reference: classic O(m*n) table, length only. */
static int ref_len(const char *x, int m, const char *y, int n)
{
    int *t = (int *)calloc((size_t)(m + 1) * (size_t)(n + 1), sizeof(int));
    int i, j, r, w = n + 1;
    for (i = 1; i <= m; i++)
        for (j = 1; j <= n; j++)
            t[i * w + j] = (x[i - 1] == y[j - 1]) ? t[(i - 1) * w + j - 1] + 1
                         : (t[(i - 1) * w + j] > t[i * w + j - 1] ? t[(i - 1) * w + j] : t[i * w + j - 1]);
    r = t[m * w + n];
    free(t);
    return r;
}

static int is_subseq(const char *s, const char *t)
{
    while (*s && *t) { if (*s == *t) s++; t++; }
    return *s == '\0';
}

static int fails = 0, total = 0;

static void check(const char *x, const char *y, const char *tag)
{
    int m = (int)strlen(x), n = (int)strlen(y);
    int mn = m < n ? m : n;
    char *res = (char *)malloc((size_t)mn + 1);
    int len = lcs(x, m, y, n, res);
    int exp = ref_len(x, m, y, n);
    int ok = (len == exp) && ((int)strlen(res) == len) && is_subseq(res, x) && is_subseq(res, y);
    total++;
    if (!ok) {
        fails++;
        if (fails <= 10) printf("FAIL [%s] X=\"%.40s\" Y=\"%.40s\" len=%d exp=%d\n", tag, x, y, len, exp);
    }
    free(res);
}

static void rnd_string(char *s, int len, const char *alpha)
{
    int i, a = (int)strlen(alpha);
    for (i = 0; i < len; i++) s[i] = alpha[rand() % a];
    s[len] = '\0';
}

int main(void)
{
    static char X[3001], Y[3001];
    static const char *alphas[] = { "a", "ab", "abc", "ACGT", "abcdefghijklmnopqrstuvwxyz", " a1!" };
    int t, i;

    srand(2024);

    /* 1. hand-made edge cases */
    check("", "", "empty-empty");
    check("", "abc", "empty-X");
    check("abc", "", "empty-Y");
    check("a", "a", "single-eq");
    check("a", "b", "single-ne");
    check("abc", "abc", "identical");
    check("abc", "def", "disjoint");
    check("abcbdab", "bdcaba", "CLRS");
    check("AGGTAB", "GXTXAYB", "classic");
    check("aaaa", "aa", "repeat");
    check("abcdef", "fedcba", "reverse");
    check("ab", "ba", "swap");
    check("a b c", "abc", "spaces");
    check("prefix_MIDDLE_suffix", "prefix_middle_suffix", "prefix-suffix");
    check("xxabyy", "abzz", "partial");

    /* 2. 20000 small random tests over varied alphabets and lengths */
    for (t = 0; t < 20000; t++) {
        int lx = rand() % 16, ly = rand() % 16;
        rnd_string(X, lx, alphas[rand() % 6]);
        rnd_string(Y, ly, alphas[rand() % 6]);
        check(X, Y, "small");
    }

    /* 3. medium / unbalanced / large random tests */
    for (t = 0; t < 300; t++) {
        int lx = rand() % 400, ly = (t % 3 == 0) ? rand() % 20 : rand() % 400;
        if (t % 5 == 0) { int tmp = lx; lx = ly; ly = tmp; }
        rnd_string(X, lx, alphas[rand() % 6]);
        rnd_string(Y, ly, alphas[rand() % 6]);
        check(X, Y, "medium");
    }
    for (t = 0; t < 6; t++) {
        rnd_string(X, 2000 + rand() % 1000, alphas[3]);
        rnd_string(Y, 2000 + rand() % 1000, alphas[3]);
        check(X, Y, "large");
    }

    /* 4. near-identical strings (exercise prefix/suffix stripping) */
    for (t = 0; t < 500; t++) {
        int len = 1 + rand() % 200;
        rnd_string(X, len, "abc");
        strcpy(Y, X);
        for (i = rand() % 4; i > 0; i--) Y[rand() % len] = "abcd"[rand() % 4];
        check(X, Y, "near-identical");
    }

    /* 5. long single-character runs */
    memset(X, 'a', 2500); X[2500] = '\0';
    memset(Y, 'a', 1700); Y[1700] = '\0';
    check(X, Y, "runs");

    printf("Total tests: %d | Failures: %d\n", total, fails);
    return fails != 0;
}
#else
/* ------------------------------- driver -------------------------------- */
/* reads one line (any length) without the trailing newline; NULL on memory error */
static char *read_line(int *len)
{
    int cap = 64, n = 0, ch;
    char *s = (char *)malloc((size_t)cap);
    if (!s) return NULL;
    while ((ch = getchar()) != EOF && ch != '\n') {
        if (ch == '\r') continue;
        if (n + 1 >= cap) {
            char *t;
            cap *= 2;
            t = (char *)realloc(s, (size_t)cap);
            if (!t) { free(s); return NULL; }
            s = t;
        }
        s[n++] = (char)ch;
    }
    s[n] = '\0';
    *len = n;
    return s;
}

int main(void)
{
    char *X, *Y, *res;
    int m, n, len, prompts;

#ifdef PROMPTS
    prompts = PROMPTS;
#else
    prompts = STDIN_IS_TTY();
#endif

    if (prompts) printf("Enter sequence X (one line, may be empty): ");
    X = read_line(&m);
    if (prompts) printf("Enter sequence Y (one line, may be empty): ");
    Y = read_line(&n);
    if (!X || !Y) { fprintf(stderr, "Out of memory\n"); return 1; }

    res = (char *)malloc((size_t)(m < n ? m : n) + 1);
    if (!res) { fprintf(stderr, "Out of memory\n"); return 1; }
    len = lcs(X, m, Y, n, res);
    if (len < 0) { fprintf(stderr, "Out of memory\n"); return 1; }

    if (prompts) {
        printf("Length of LCS = %d\n", len);
        printf("LCS           = %s\n", res);
    } else {
        printf("%d\n%s\n", len, res);
    }
    free(X); free(Y); free(res);
    return 0;
}
#endif