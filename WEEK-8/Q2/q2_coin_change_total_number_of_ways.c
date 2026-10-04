/*
 * Coin Change - total number of combinations (order irrelevant, unlimited coins)
 *
 * PORTABLE ANSI C (C89): no type aliases, no struct, no long long, no __int128,
 * no compiler builtins, no math.h.  Works on old and new compilers.
 *
 * Algorithm : 1-D DP, coins in the OUTER loop:  dp[v] += dp[v - c]
 *             (outer loop over coins => 1+2 and 2+1 are counted once)
 * Big number: each dp[v] is an exact big integer stored as L limbs in base 10^9
 *             (little-endian) inside one flat array -> NO overflow, ever.
 *             L is computed from the proven bound  ways <= product(V/c_i + 1).
 * Time      : O(n * V * L)   (L = 1 whenever the answer fits in ~9 digits,
 *                             so it is the textbook O(n * V) in that case)
 * Space     : O(V * L)       (O(V) when the answer is small)
 *
 * Input  : 1) n  = how many coin denominations
 *          2) the n distinct positive coin values (separated by spaces)
 *          3) V  = target amount
 * Output : total number of distinct combinations of coins that sum to V.
 * Handles: V = 0 -> 1, V < 0 -> 0, n = 0, coins <= 0 or > V ignored,
 *          duplicates ignored, all memory freed on every exit path.
 */
#include <stdio.h>
#include <stdlib.h>

#define BASE 1000000000UL

/* 1 = show friendly prompts and a labelled answer (for typing by hand)
   0 = read silently and print only the number (for online judges / file input) */
#ifndef PROMPTS
#define PROMPTS 1
#endif

static int cmp_int(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;
    if (x < y) return -1;
    if (x > y) return 1;
    return 0;
}

/* printed only immediately before a real answer */
static void label(void)
{
    if (PROMPTS) printf("Total number of distinct combinations = ");
}

int main(void)
{
    int n, i, k, m, L, bits, t;
    long V, v, x;
    int *c = NULL, *d = NULL;
    unsigned long *dp = NULL, carry, s;
    unsigned long *dst, *src;
    size_t total;
    int j, top;
    int status = 0;

    if (PROMPTS) printf("Enter number of coin denominations (n): ");
    if (scanf("%d", &n) != 1 || n < 0) {
        printf("Invalid input for n\n");
        return 1;
    }

    c = (int *)malloc((size_t)(n > 0 ? n : 1) * sizeof(int));
    d = (int *)malloc((size_t)(n > 0 ? n : 1) * sizeof(int));
    if (!c || !d) {
        printf("Out of memory\n");
        status = 1;
        goto done;
    }

    if (PROMPTS) printf("Enter the %d coin values (space separated): ", n);
    for (i = 0; i < n; i++) {
        if (scanf("%d", &c[i]) != 1) {
            printf("Invalid coin value\n");
            status = 1;
            goto done;
        }
    }

    if (PROMPTS) printf("Enter target amount (V): ");
    if (scanf("%ld", &V) != 1) {
        printf("Invalid input for V\n");
        status = 1;
        goto done;
    }

    if (V < 0) { label(); printf("0\n"); goto done; }
    if (V == 0) { label(); printf("1\n"); goto done; }

    /* keep only usable coins: 1 <= c <= V, sorted, duplicates removed */
    m = 0;
    for (i = 0; i < n; i++)
        if (c[i] > 0 && (long)c[i] <= V) d[m++] = c[i];
    if (m > 0) qsort(d, (size_t)m, sizeof(int), cmp_int);
    k = 0;
    for (i = 0; i < m; i++)
        if (k == 0 || d[i] != d[k - 1]) d[k++] = d[i];

    if (k == 0) { label(); printf("0\n"); goto done; }

    /* bound on number of bits of the answer: ways <= prod (V/c + 1) < 2^bits */
    bits = 0;
    for (i = 0; i < k; i++) {
        x = V / d[i] + 1;
        while (x > 0) { bits++; x >>= 1; }
    }
    L = bits / 29 + 2;             /* one base-10^9 limb holds more than 29 bits */

    /* guard: (V+1) * L * sizeof(unsigned long) must not overflow size_t */
    total = (size_t)V + 1;
    if (total < (size_t)V ||
        total > ((size_t)-1) / sizeof(unsigned long) / (size_t)L) {
        printf("V is too large for available memory\n");
        status = 1;
        goto done;
    }

    dp = (unsigned long *)calloc(total * (size_t)L, sizeof(unsigned long));
    if (!dp) {
        printf("Out of memory (V too large)\n");
        status = 1;
        goto done;
    }
    dp[0] = 1UL;                   /* dp[0] = 1 (limb 0 of number 0) */

    for (i = 0; i < k; i++) {
        t = d[i];
        for (v = t; v <= V; v++) {
            dst = dp + (size_t)v * (size_t)L;
            src = dp + (size_t)(v - t) * (size_t)L;
            carry = 0UL;
            for (j = 0; j < L; j++) {
                s = dst[j] + src[j] + carry;
                if (s >= BASE) { s -= BASE; carry = 1UL; }
                else carry = 0UL;
                dst[j] = s;
            }
        }
    }

    /* print dp[V] */
    dst = dp + (size_t)V * (size_t)L;
    top = L - 1;
    while (top > 0 && dst[top] == 0UL) top--;
    label();
    printf("%lu", dst[top]);
    for (j = top - 1; j >= 0; j--) printf("%09lu", dst[j]);
    printf("\n");

done:
    free(dp);
    free(c);
    free(d);
    return status;
}