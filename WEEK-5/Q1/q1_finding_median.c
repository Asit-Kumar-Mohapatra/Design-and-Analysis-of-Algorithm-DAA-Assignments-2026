#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void swap(int *a, int *b) {
    int t = *a; *a = *b; *b = t;
}

void insertionSortSmall(int arr[], int l, int r) {
    for (int i = l + 1; i <= r; i++) {
        int key = arr[i], j = i - 1;
        while (j >= l && arr[j] > key) { arr[j + 1] = arr[j]; j--; }
        arr[j + 1] = key;
    }
}

int medianOfMedians(int arr[], int l, int r) {
    int n = r - l + 1;
    if (n <= 5) {
        insertionSortSmall(arr, l, r);
        return arr[l + n / 2];
    }

    int numGroups = (n + 4) / 5;
    for (int i = 0; i < numGroups; i++) {
        int groupL = l + i * 5;
        int groupR = groupL + 4;
        if (groupR > r) groupR = r;
        insertionSortSmall(arr, groupL, groupR);
        int medianIdx = groupL + (groupR - groupL) / 2;
        swap(&arr[l + i], &arr[medianIdx]);
    }
    return medianOfMedians(arr, l, l + numGroups - 1);
}

void partition3way(int arr[], int l, int r, int pivotVal, int *lt, int *gt) {
    int i = l;
    *lt = l;
    *gt = r;
    while (i <= *gt) {
        if (arr[i] < pivotVal) {
            swap(&arr[*lt], &arr[i]);
            (*lt)++; i++;
        } else if (arr[i] > pivotVal) {
            swap(&arr[i], &arr[*gt]);
            (*gt)--;
        } else {
            i++;
        }
    }
}

int introSelect(int arr[], int l, int r, int k, int depthBudget) {
    while (l < r) {
        int pivotVal;

        if (depthBudget <= 0) {
            pivotVal = medianOfMedians(arr, l, r);
        } else {
            int randomIndex = l + rand() % (r - l + 1);
            pivotVal = arr[randomIndex];
        }
        depthBudget--;

        int lt, gt;
        partition3way(arr, l, r, pivotVal, &lt, &gt);

        if (k < lt) {
            r = lt - 1;
        } else if (k > gt) {
            l = gt + 1;
        } else {
            return arr[k];
        }
    }
    return arr[l];
}

double findMedianOptimized(int arr[], int n) {
    int budget = 0;
    for (int t = n; t > 1; t >>= 1) budget++;
    budget *= 2;

    if (n % 2 == 1) {
        return (double) introSelect(arr, 0, n - 1, n / 2, budget);
    } else {
        int lowRank = n / 2 - 1;
        int lowVal = introSelect(arr, 0, n - 1, lowRank, budget);

        int highVal = arr[lowRank + 1];
        for (int i = lowRank + 2; i < n; i++) {
            if (arr[i] < highVal) highVal = arr[i];
        }
        /* Cast BEFORE adding -- lowVal + highVal as plain int arithmetic
           can overflow (e.g. two values near INT_MAX or INT_MIN), silently
           wrapping to a wrong result. Promoting to double first avoids that. */
        return (double) lowVal / 2.0 + (double) highVal / 2.0;
    }
}

/* ===========================================================================
   printComplexityAnalysis
   Explains the time/space complexity of each function in the algorithm.
   Called from main() after computing the result, so the analysis is part
   of the program's own output/documentation.
   =========================================================================== */
void printComplexityAnalysis(int n) {
    printf("\n===================== COMPLEXITY ANALYSIS =====================\n");

    printf("\n1. insertionSortSmall(l, r)\n");
    printf("   Sorts a fixed-size group of at most 5 elements.\n");
    printf("   Time  : O(1)      -- group size is a constant, independent of N\n");
    printf("   Space : O(1)\n");

    printf("\n2. medianOfMedians(l, r)\n");
    printf("   Splits the range into groups of 5, takes each group's median\n");
    printf("   in O(1), then recurses on the N/5 medians.\n");
    printf("   Recurrence : T(N) = T(N/5) + O(N)  =>  T(N) = O(N)\n");
    printf("   Time  : O(N)\n");
    printf("   Space : O(log N)  -- recursion depth is log base 5 of N\n");
    printf("   Purpose: returns a pivot guaranteed to discard >= ~30%% of\n");
    printf("            the array every time it is used, which is what\n");
    printf("            bounds the worst case of the selection algorithm.\n");

    printf("\n3. partition3way(l, r, pivotVal)\n");
    printf("   Single left-to-right pass grouping elements into\n");
    printf("   [< pivot | == pivot | > pivot].\n");
    printf("   Time  : O(r - l)  -- proportional to current sub-array size\n");
    printf("   Space : O(1)\n");

    printf("\n4. introSelect(l, r, k, depthBudget)\n");
    printf("   Average case:\n");
    printf("     Random pivot each step, each partition roughly halves the\n");
    printf("     range: N + N/2 + N/4 + ... converges to O(N)\n");
    printf("   Worst case:\n");
    printf("     If depthBudget runs out (partitions were repeatedly\n");
    printf("     unbalanced), falls back to medianOfMedians for that step.\n");
    printf("     Recurrence becomes T(N) = T(0.7N) + O(N)  =>  O(N)\n");
    printf("   Time  : O(N) average AND O(N) worst case\n");
    printf("           (plain randomized quickselect alone is O(N^2) worst\n");
    printf("           case -- the median-of-medians fallback removes that risk)\n");
    printf("   Space : O(1) extra for the loop itself\n");

    printf("\n5. findMedianOptimized(n)\n");
    printf("   Calls introSelect once           : O(N)\n");
    printf("   Even N -> one extra linear scan   : O(N)\n");
    printf("   (avoids a second full O(N) selection call for the 2nd rank)\n");
    printf("   Time  : O(N)\n");
    printf("   Space : O(log N)  -- dominated by medianOfMedians recursion\n");

    printf("\n----------------------------------------------------------------\n");
    printf("OVERALL for N = %d elements:\n", n);
    printf("   Time  (average)     : O(N)\n");
    printf("   Time  (worst case)  : O(N)   [guaranteed, unlike plain quickselect]\n");
    printf("   Space (extra)       : O(log N)\n");
    printf("   Lower bound         : Omega(N) -- any correct algorithm must\n");
    printf("                         inspect every element at least once,\n");
    printf("                         so this algorithm is asymptotically optimal.\n");
    printf("=================================================================\n");
}

int main() {
    srand((unsigned int) time(NULL));

    int n;
    printf("Enter the number of elements (N): ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Invalid input. N must be a positive integer.\n");
        return 1;
    }

    int *arr = (int *) malloc(n * sizeof(int));
    if (arr == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    printf("Enter %d integers separated by spaces:\n", n);
    for (int i = 0; i < n; i++) {
        if (scanf("%d", &arr[i]) != 1) {
            printf("Invalid input at element %d.\n", i + 1);
            free(arr);
            return 1;
        }
    }

    double median = findMedianOptimized(arr, n);

    printf("\nN = %d\n", n);
    printf("Median = %.2f\n", median);

    printComplexityAnalysis(n);

    free(arr);
    return 0;
}