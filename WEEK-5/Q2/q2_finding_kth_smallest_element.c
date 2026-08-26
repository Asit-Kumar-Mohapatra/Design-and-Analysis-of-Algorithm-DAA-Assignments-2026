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

/* introSelect(arr, l, r, k, depthBudget)
   Returns the element that would sit at 0-indexed position k if the
   array were fully sorted -- i.e. the (k+1)'th smallest element. */
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

/* findKthSmallest: user gives K as 1-indexed (K=1 means smallest element).
   Internally converted to 0-indexed rank (K-1) for introSelect. */
int findKthSmallest(int arr[], int n, int k) {
    int budget = 0;
    for (int t = n; t > 1; t >>= 1) budget++;
    budget *= 2;

    return introSelect(arr, 0, n - 1, k - 1, budget);
}

void printComplexityAnalysis(int n, int k) {
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
    printf("            the array every time it is used, bounding the\n");
    printf("            worst case of selection regardless of which K is asked.\n");

    printf("\n3. partition3way(l, r, pivotVal)\n");
    printf("   Single left-to-right pass grouping elements into\n");
    printf("   [< pivot | == pivot | > pivot].\n");
    printf("   Time  : O(r - l)  -- proportional to current sub-array size\n");
    printf("   Space : O(1)\n");

    printf("\n4. introSelect(l, r, k, depthBudget)\n");
    printf("   Average case:\n");
    printf("     Random pivot each step, each partition roughly halves the\n");
    printf("     range: N + N/2 + N/4 + ... converges to O(N)\n");
    printf("     -- this holds for ANY target rank K, not just the median.\n");
    printf("   Worst case:\n");
    printf("     If depthBudget runs out, falls back to medianOfMedians.\n");
    printf("     Recurrence becomes T(N) = T(0.7N) + O(N)  =>  O(N)\n");
    printf("   Time  : O(N) average AND O(N) worst case\n");
    printf("           (plain randomized quickselect alone is O(N^2) worst\n");
    printf("           case -- the fallback removes that risk for any K)\n");
    printf("   Space : O(1) extra for the loop itself\n");

    printf("\n5. findKthSmallest(n, k)\n");
    printf("   Single call to introSelect            : O(N)\n");
    printf("   NOTE: unlike median-finding (which needs 1-2 ranks), a\n");
    printf("   general K'th-smallest query only ever needs ONE selection\n");
    printf("   call regardless of where K falls, so there is no special\n");
    printf("   case to optimize away here.\n");
    printf("   Time  : O(N)\n");
    printf("   Space : O(log N)  -- dominated by medianOfMedians recursion\n");

    printf("\n----------------------------------------------------------------\n");
    printf("OVERALL for N = %d elements, K = %d:\n", n, k);
    printf("   Time  (average)     : O(N)\n");
    printf("   Time  (worst case)  : O(N)   [guaranteed, unlike plain quickselect]\n");
    printf("   Space (extra)       : O(log N)\n");
    printf("   Lower bound         : Omega(N) -- any correct algorithm must\n");
    printf("                         inspect every element at least once,\n");
    printf("                         so this algorithm is asymptotically optimal\n");
    printf("                         for EVERY value of K, not just the median.\n");
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

    int k;
    printf("Enter K (1 <= K <= %d): ", n);
    if (scanf("%d", &k) != 1 || k < 1 || k > n) {
        printf("Invalid K. Must be between 1 and %d.\n", n);
        free(arr);
        return 1;
    }

    int result = findKthSmallest(arr, n, k);

    printf("\nN = %d, K = %d\n", n, k);
    printf("%d'th smallest element = %d\n", k, result);

    printComplexityAnalysis(n, k);

    free(arr);
    return 0;
}