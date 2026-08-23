/*
 * ============================================================================
 *  Application of Sorting - III  (VERIFIED / FIXED VERSION)
 * ----------------------------------------------------------------------------
 *  Given a set S of n integers and an integer T, determine whether some k
 *  of the integers in S add up to T.  Required time: O(n^(k-1) * log n).
 *
 *  ALGORITHM (fix k-1 elements via recursive combinations, binary-search
 *  for the k-th):
 *    1. Sort S                                          -> O(n log n)
 *    2. Recursively generate every combination of (k-1) strictly
 *       increasing indices                              -> O(n^(k-1))
 *    3. For each combination, binary-search for the value that completes
 *       the sum to T, restricted to indices after the largest chosen
 *       index (this guarantees distinctness for free -- no separate
 *       collision check needed)                          -> O(log n) each
 *    Total: O(n^(k-1) log n).
 *
 *  COMPLEXITY:
 *    Time  : O(n^(k-1) log n)  (k treated as a fixed constant)
 *    Space : O(k) extra (recursion stack / chosen-index array) -- O(1) wrt n.
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

static int cmp_int(const void *a, const void *b) {
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    return (ia > ib) - (ia < ib);
}

/* Binary search restricted to arr[lo..hi]. -> O(log n) */
static int binary_search_ranged(const int *arr, int lo, int hi, int target) {
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (arr[mid] == target) return mid;
        if (arr[mid] < target)  lo = mid + 1;
        else                    hi = mid - 1;
    }
    return -1;
}

typedef struct {
    const int *arr;
    int        n;
    int        k;
    long long  target;
    int       *chosen;
} KSumCtx;

/*
 * combo_search:
 *   Recursively picks (k-1) strictly increasing indices, then binary
 *   searches for the value completing the sum to T.
 */
static int combo_search(KSumCtx *ctx, int depth, int start, long long partial_sum) {
    int k = ctx->k;
    int n = ctx->n;

    if (depth == k - 1) {
        long long needed = ctx->target - partial_sum;
        /* FIX: this range check was already present here, but was
         * missing from the k==1 fast path in k_sum_exists -- see fix
         * there. Keeping it here too, since it's still needed for the
         * general k>=2 case. */
        if (needed < (long long)INT_MIN || needed > (long long)INT_MAX) return 0;

        int search_start = (depth > 0) ? ctx->chosen[depth - 1] + 1 : 0;
        int idx = binary_search_ranged(ctx->arr, search_start, n - 1, (int)needed);
        if (idx == -1) return 0;

        ctx->chosen[depth] = idx;
        return 1;
    }

    int remaining = (k - 1) - depth;
    for (int i = start; i <= n - remaining; i++) {
        ctx->chosen[depth] = i;
        if (combo_search(ctx, depth + 1, i + 1, partial_sum + ctx->arr[i])) {
            return 1;
        }
    }
    return 0;
}

/*
 * k_sum_exists:
 *   Determines whether some k elements of `s` (size n) sum to `target`.
 *   `s` is sorted in place (O(1) extra space).
 *   On success, writes the k values into out_values[0..k-1] and returns 1.
 */
int k_sum_exists(int *s, int n, int k, long long target, int *out_values) {
    if (k <= 0 || k > n) return 0;

    qsort(s, (size_t)n, sizeof(int), cmp_int);   /* O(n log n) */

    if (k == 1) {
        /* FIX: target must be range-checked BEFORE casting to int --
         * a plain (int)target on an out-of-range long long is
         * implementation-defined truncation and can produce false
         * positives (e.g. target = 3 + 2^32 truncates to 3 on a
         * typical 32-bit int, wrongly matching an actual 3 in S). */
        if (target < (long long)INT_MIN || target > (long long)INT_MAX) return 0;

        int idx = binary_search_ranged(s, 0, n - 1, (int)target);
        if (idx == -1) return 0;
        out_values[0] = s[idx];
        return 1;
    }

    int *chosen = (int *)malloc((size_t)k * sizeof(int));
    if (!chosen) { fprintf(stderr, "Memory allocation failed\n"); exit(1); }

    KSumCtx ctx = { s, n, k, target, chosen };
    int found = combo_search(&ctx, 0, 0, 0);

    if (found) {
        for (int i = 0; i < k; i++) out_values[i] = s[chosen[i]];
    }
    free(chosen);
    return found;
}

static int *read_array(int n) {
    int *arr = (int *)malloc((size_t)n * sizeof(int));
    if (!arr) { fprintf(stderr, "Memory allocation failed\n"); exit(1); }
    printf("Enter %d distinct integers of S: ", n);
    for (int i = 0; i < n; i++) {
        if (scanf("%d", &arr[i]) != 1) {
            fprintf(stderr, "Invalid input while reading S\n");
            free(arr);
            exit(1);
        }
    }
    return arr;
}

static void print_array(const int *arr, int n, const char *label) {
    printf("%s = { ", label);
    for (int i = 0; i < n; i++) printf("%d%s", arr[i], (i == n - 1) ? "" : ", ");
    printf(" }\n");
}

int main(void) {
    int n;
    printf("Enter n (size of S): ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        fprintf(stderr, "Invalid n\n");
        return 1;
    }

    int *s = read_array(n);

    int k;
    long long T;
    printf("Enter k (how many elements should sum to T): ");
    if (scanf("%d", &k) != 1 || k <= 0 || k > n) {
        fprintf(stderr, "Invalid k (must satisfy 1 <= k <= n)\n");
        free(s);
        return 1;
    }
    printf("Enter T (target sum): ");
    if (scanf("%lld", &T) != 1) {
        fprintf(stderr, "Invalid T\n");
        free(s);
        return 1;
    }

    printf("\n");
    print_array(s, n, "S (original order)");
    printf("k = %d, T = %lld\n\n", k, T);

    int *result = (int *)malloc((size_t)k * sizeof(int));
    if (!result) { fprintf(stderr, "Memory allocation failed\n"); free(s); return 1; }

    int found = k_sum_exists(s, n, k, T, result);

    if (found) {
        printf("RESULT: YES -- found %d elements that sum to %lld: {", k, T);
        long long sum_check = 0;
        for (int i = 0; i < k; i++) {
            printf(" %d%s", result[i], (i == k - 1) ? " " : ",");
            sum_check += result[i];
        }
        printf("} (verified sum = %lld)\n", sum_check);
    } else {
        printf("RESULT: NO -- no %d elements of S sum to %lld\n", k, T);
    }

    free(s);
    free(result);
    return 0;
}