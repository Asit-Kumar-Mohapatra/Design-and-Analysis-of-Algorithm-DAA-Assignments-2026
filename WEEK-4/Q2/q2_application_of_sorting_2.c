#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

static int cmp_int(const void *a, const void *b) {
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    return (ia > ib) - (ia < ib);
}

/* Widened to long long to avoid overflow when x - s2[j] doesn't fit in int. */
static int binary_search(const int *arr, int n, long long target) {
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if ((long long)arr[mid] == target) return 1;
        if ((long long)arr[mid] < target) lo = mid + 1;
        else                              hi = mid - 1;
    }
    return 0;
}

int find_pair_with_sum_inplace(int *s1, int n1, const int *s2, int n2,
                                int x, int *out_a, int *out_b) {
    qsort(s1, (size_t)n1, sizeof(int), cmp_int);

    for (int j = 0; j < n2; j++) {
        /* Compute in long long: x - s2[j] can overflow int (e.g. x = INT_MAX,
         * s2[j] = INT_MIN), which is undefined behavior for plain int. */
        long long needed = (long long)x - (long long)s2[j];
        if (needed >= INT_MIN && needed <= INT_MAX && binary_search(s1, n1, needed)) {
            *out_a = (int)needed;
            *out_b = s2[j];
            return 1;
        }
    }
    return 0;
}

static int *read_array(int n, const char *label) {
    int *arr = (int *)malloc((size_t)n * sizeof(int));
    if (!arr) { fprintf(stderr, "Memory allocation failed\n"); exit(1); }
    printf("Enter %d elements of %s: ", n, label);
    for (int i = 0; i < n; i++) {
        if (scanf("%d", &arr[i]) != 1) {
            fprintf(stderr, "Invalid input while reading %s\n", label);
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
    printf("Enter n (size of each set): ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        fprintf(stderr, "Invalid n\n");
        return 1;
    }

    int *s1 = read_array(n, "S1");
    int *s2 = read_array(n, "S2");

    int x;
    printf("Enter x (target sum): ");
    if (scanf("%d", &x) != 1) {
        fprintf(stderr, "Invalid x\n");
        free(s1); free(s2);
        return 1;
    }

    printf("\n");
    print_array(s1, n, "S1 (original order)");
    print_array(s2, n, "S2");
    printf("Target x = %d\n\n", x);

    int a, b;
    int found = find_pair_with_sum_inplace(s1, n, s2, n, x, &a, &b);

    print_array(s1, n, "S1 (after in-place sort, as a side effect)");
    printf("\n");

    if (found) {
        printf("RESULT: YES -- found pair (%d from S1, %d from S2) with %d + %d = %d\n",
               a, b, a, b, x);
    } else {
        printf("RESULT: NO -- no pair (a in S1, b in S2) sums to %d\n", x);
    }

    free(s1);
    free(s2);
    return 0;
}