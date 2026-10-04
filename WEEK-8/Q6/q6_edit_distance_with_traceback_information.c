/*
 * Edit Distance (Levenshtein) with Traceback
 * Operations: insert, delete, substitute (each cost 1); matching characters cost 0.
 *
 * PORTABLE ANSI C (C89): no type aliases, no struct, no long long, no compiler builtins.
 *
 * Input representation : A and B are each one line of characters (letters, digits,
 *                        symbols, spaces).  A has m characters, B has n characters.
 *                        An empty line is a valid (empty) string.
 *
 * Algorithm  : Hirschberg's divide and conquer applied to the edit-distance DP
 *   - Recurrence  d[i][j] = d[i-1][j-1]                         if a_i == b_j
 *                           1 + min(d[i-1][j-1] (substitute),
 *                                   d[i-1][j]   (delete a_i),
 *                                   d[i][j-1]   (insert b_j))   otherwise
 *   - Split the longer string at its middle; a forward pass and a backward pass (each
 *     keeping ONE row) give the cost of both halves against every split point of the
 *     other string.  The split point k minimising F[k] + R[k] lies on an optimal
 *     alignment; recurse on both halves.  The traceback is produced while unwinding.
 *
 * Time  : O(m*n)   (at most about 2*m*n cell updates: mn + mn/2 + mn/4 + ...)
 * Space : O(min(m,n)) working memory + O(m+n) for the strings and the operation list
 *         (the classic table needs O(m*n) to trace back).
 *
 * Output: minimum number of operations, the operation string
 *         (M = match, S = substitute, D = delete, I = insert), a step list and
 *         an aligned view of A and B.
 */
#include <stdio.h>
#include <stdlib.h>

#ifndef PROMPTS
#define PROMPTS 1   /* 1 = friendly prompts / full report, 0 = silent (online judge) */
#endif

#define WRAP 60     /* columns per block in the alignment view */

static const char *P;   /* string that is split in halves (the longer one)  */
static const char *Q;   /* string that indexes the row (the shorter one)    */
static int swapped;     /* 1 if P is really B and Q is really A              */
static int *Frow;
static int *Rrow;
static char *ops;       /* M S D I in alignment order */
static int nops;

static void emit_p_only(void) { ops[nops++] = swapped ? 'I' : 'D'; }
static void emit_q_only(void) { ops[nops++] = swapped ? 'D' : 'I'; }
static void emit_pair(int ip, int iq) { ops[nops++] = (P[ip] == Q[iq]) ? 'M' : 'S'; }

static int min3(int a, int b, int c)
{
    if (b < a) a = b;
    if (c < a) a = c;
    return a;
}

/* F[j] = ED( P[a0..mid) , Q[b0..b0+j) ), j = 0..nb */
static void forward(int a0, int mid, int b0, int nb)
{
    int i, j, diag, up;
    for (j = 0; j <= nb; j++) Frow[j] = j;
    for (i = a0; i < mid; i++) {
        diag = Frow[0];
        Frow[0] = i - a0 + 1;
        for (j = 1; j <= nb; j++) {
            up = Frow[j];
            Frow[j] = min3(diag + (P[i] == Q[b0 + j - 1] ? 0 : 1), up + 1, Frow[j - 1] + 1);
            diag = up;
        }
    }
}

/* R[j] = ED( P[mid..a1) , Q[b0+j..b0+nb) ), j = 0..nb */
static void backward(int mid, int a1, int b0, int nb)
{
    int i, j, diag, up;
    for (j = 0; j <= nb; j++) Rrow[j] = nb - j;
    for (i = a1 - 1; i >= mid; i--) {
        diag = Rrow[nb];
        Rrow[nb] = a1 - i;
        for (j = nb - 1; j >= 0; j--) {
            up = Rrow[j];
            Rrow[j] = min3(diag + (P[i] == Q[b0 + j] ? 0 : 1), up + 1, Rrow[j + 1] + 1);
            diag = up;
        }
    }
}

/* appends an optimal alignment of P[a0..a1) and Q[b0..b1) to ops */
static void hirschberg(int a0, int a1, int b0, int b1)
{
    int j, k, mid, nb, best, bestk, pos;

    nb = b1 - b0;
    if (a1 == a0) { for (j = b0; j < b1; j++) emit_q_only(); return; }
    if (nb == 0)  { for (j = a0; j < a1; j++) emit_p_only(); return; }
    if (a1 - a0 == 1) {                     /* one character of P against Q[b0..b1) */
        pos = -1;
        for (j = b0; j < b1; j++) if (Q[j] == P[a0]) { pos = j; break; }
        if (pos < 0) pos = b0;              /* no equal char: substitute the first one */
        for (j = b0; j < pos; j++) emit_q_only();
        emit_pair(a0, pos);
        for (j = pos + 1; j < b1; j++) emit_q_only();
        return;
    }
    mid = a0 + (a1 - a0) / 2;
    forward(a0, mid, b0, nb);
    backward(mid, a1, b0, nb);
    best = Frow[0] + Rrow[0]; bestk = 0;
    for (k = 1; k <= nb; k++)
        if (Frow[k] + Rrow[k] < best) { best = Frow[k] + Rrow[k]; bestk = k; }
    hirschberg(a0, mid, b0, b0 + bestk);
    hirschberg(mid, a1, b0 + bestk, b1);
}

/* reads one line (any length) without the trailing newline */
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
    char *A = NULL, *B = NULL, *ra = NULL, *rb = NULL, *rm = NULL;
    int m = 0, n = 0, lp, lq, i, j, k, s, e, dist, step, status = 0;

    if (PROMPTS) { printf("Enter string A (source, one line, may be empty): "); fflush(stdout); }
    A = read_line(&m);
    if (PROMPTS) { printf("Enter string B (target, one line, may be empty): "); fflush(stdout); }
    B = read_line(&n);
    if (!A || !B) { printf("Out of memory\n"); status = 1; goto cleanup; }

    if (m >= n) { P = A; lp = m; Q = B; lq = n; swapped = 0; }
    else        { P = B; lp = n; Q = A; lq = m; swapped = 1; }

    Frow = (int *)malloc((size_t)(lq + 2) * sizeof(int));
    Rrow = (int *)malloc((size_t)(lq + 2) * sizeof(int));
    ops  = (char *)malloc((size_t)(m + n + 2));
    if (!Frow || !Rrow || !ops) { printf("Out of memory\n"); status = 1; goto cleanup; }
    nops = 0;

    hirschberg(0, lp, 0, lq);
    ops[nops] = '\0';

    dist = 0;
    for (k = 0; k < nops; k++) if (ops[k] != 'M') dist++;

    if (!PROMPTS) {
        printf("%d\n%s\n", dist, ops);
    } else {
        printf("\nMinimum number of operations = %d\n", dist);
        printf("Operation string (M=match S=substitute D=delete I=insert):\n%s\n",
               nops ? ops : "(none - both strings are empty)");
        printf("\nTraceback (edit steps only, positions are 1-based):\n");
        i = 0; j = 0; step = 0;
        for (k = 0; k < nops; k++) {
            if (ops[k] == 'M') { i++; j++; }
            else if (ops[k] == 'S') {
                printf("  %d. Substitute A[%d]='%c' with B[%d]='%c'\n", ++step, i + 1, A[i], j + 1, B[j]);
                i++; j++;
            } else if (ops[k] == 'D') {
                printf("  %d. Delete A[%d]='%c'\n", ++step, i + 1, A[i]);
                i++;
            } else {
                printf("  %d. Insert B[%d]='%c'\n", ++step, j + 1, B[j]);
                j++;
            }
        }
        if (step == 0) printf("  (no edits needed - the strings are identical)\n");

        ra = (char *)malloc((size_t)nops + 1);
        rb = (char *)malloc((size_t)nops + 1);
        rm = (char *)malloc((size_t)nops + 1);
        if (ra && rb && rm) {
            i = 0; j = 0;
            for (k = 0; k < nops; k++) {
                ra[k] = (ops[k] == 'I') ? '-' : A[i];
                rb[k] = (ops[k] == 'D') ? '-' : B[j];
                rm[k] = (ops[k] == 'M') ? '|' : ' ';
                if (ops[k] != 'I') i++;
                if (ops[k] != 'D') j++;
            }
            ra[nops] = rb[nops] = rm[nops] = '\0';
            printf("\nAlignment:\n");
            if (nops == 0) printf("  (empty)\n");
            for (s = 0; s < nops; s += WRAP) {
                e = (s + WRAP < nops) ? s + WRAP : nops;
                printf("  A: %.*s\n     %.*s\n  B: %.*s\n\n",
                       e - s, ra + s, e - s, rm + s, e - s, rb + s);
            }
        } else {
            printf("\n(Not enough memory to draw the alignment view)\n");
        }
    }

cleanup:
    free(ra); free(rb); free(rm);
    free(A); free(B); free(Frow); free(Rrow); free(ops);
    return status;
}