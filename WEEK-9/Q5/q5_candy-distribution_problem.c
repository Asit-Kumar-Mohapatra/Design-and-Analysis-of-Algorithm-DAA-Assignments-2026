/*
 * Candy Distribution Problem (Bi-directional Slope Greedy)
 * Time : O(n) single pass      Space : O(1) extra (ratings streamed)
 *
 * Input : n, then n integer ratings (whitespace separated)
 * Output: minimum total number of candies
 *
 * Ratings form rising slopes, falling slopes and plateaus.
 *  - rising  slope of length u : 1,2,...,u+1
 *  - falling slope of length d : d+1,...,2,1
 *  - shared peak needs max(u,d)+1 candies
 *  - equal neighbours impose no constraint -> reset
 */
#include <stdio.h>

static int read_int(long long *out)           /* fast reader; 0 on EOF */
{
    int c = getchar_unlocked(), neg = 0;
    while (c != EOF && c != '-' && (c < '0' || c > '9')) c = getchar_unlocked();
    if (c == EOF) return 0;
    if (c == '-') { neg = 1; c = getchar_unlocked(); }
    long long x = 0;
    while (c >= '0' && c <= '9') { x = x * 10 + (c - '0'); c = getchar_unlocked(); }
    *out = neg ? -x : x;
    return 1;
}

int main(void)
{
    long long n, cur, prev = 0, total = 0, up = 0, down = 0, peak = 0;
    if (!read_int(&n) || n <= 0) { printf("0\n"); return 0; }

    for (long long i = 0; i < n; i++) {
        if (!read_int(&cur)) break;
        if (i == 0)            total = 1;
        else if (cur > prev) { up++; peak = up; down = 0; total += 1 + up; }
        else if (cur == prev){ up = down = peak = 0; total += 1; }
        else                 { up = 0; down++; total += down;
                               if (down > peak) total++; }
        prev = cur;
    }
    printf("%lld\n", total);
    return 0;
}