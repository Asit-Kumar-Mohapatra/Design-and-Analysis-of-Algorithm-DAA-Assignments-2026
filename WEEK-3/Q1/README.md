# Q1 — Binary vs Ternary Search

## Explanation of Question
The core objective of this problem is to compare the performance of **Binary Search** and **Ternary Search** on a sorted array, specifically focusing on the number of key comparisons required in the worst-case scenario. 

When searching for an element in a sorted array:
- **Binary Search** computes a single midpoint, effectively splitting the search space into 2 halves. It checks if the target is the midpoint, and if not, determines whether to search the left half or the right half.
- **Ternary Search** computes two midpoints, splitting the search space into 3 equal thirds. It checks if the target matches either midpoint, and if not, determines which of the three zones (left, middle, or right) the target belongs to.

**Intuition:** At first glance, one might assume Ternary Search is faster because it reduces the search space more aggressively (dividing by 3 instead of 2), resulting in a shallower recursion tree. However, determining which third to enter requires more comparisons at every single level. The goal of this question is to analyze whether the shallower tree depth actually offsets the increased comparison cost per level.

## Algorithmic & Mathematical Analysis
### Binary Search Analysis
At each recursive step or loop iteration, Binary Search performs up to **2 comparisons**:
1. **Equality check:** `arr[mid] == target`
2. **Direction check:** `arr[mid] > target` (to decide left vs right)

- **Recurrence Relation:** `T(n) = T(n/2) + 2`
- **Tree Height:** `log₂(n)`
- **Worst-case Comparisons:** `C_binary(n) = 2 · log₂(n)`

### Ternary Search Analysis
At each step, Ternary Search performs up to **4 comparisons**:
1. **Equality check 1:** `arr[mid1] == target`
2. **Equality check 2:** `arr[mid2] == target`
3. **Direction check 1:** `target < arr[mid1]`
4. **Direction check 2:** `target > arr[mid2]` (to decide right vs middle)

- **Recurrence Relation:** `T(n) = T(n/3) + 4`
- **Tree Height:** `log₃(n) = log₂(n) / log₂(3) ≈ log₂(n) / 1.585`
- **Worst-case Comparisons:** `C_ternary(n) = 4 · log₃(n) ≈ 2.524 · log₂(n)`

### Conclusion
Comparing the two mathematically:
`C_ternary / C_binary = (4 / log₂3) / 2 = 2 / log₂3 ≈ 1.26`

This mathematical ratio proves that Ternary Search introduces a **~26% overhead** in comparisons compared to Binary Search. Despite the shallower tree, the cost of making 4 comparisons per level makes it strictly slower than Binary Search for sorted arrays.

## C Code Explanation
### `binarySearch()` Function
```c
int binarySearch(int *arr, int size, int target, int *comparisons) {
    int low = 1, high = size;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        (*comparisons)++;           // equality check
        if (arr[mid] == target) return mid;
        (*comparisons)++;           // direction check
        if (arr[mid] > target) high = mid - 1;
        else low = mid + 1;
    }
    return -1;
}
```
*Notice how `(*comparisons)` is tightly tracked per `if` block, ensuring we accurately simulate the worst-case key comparisons.*

### `ternarySearch()` Function
```c
int ternarySearch(int *arr, int size, int target, int *comparisons) {
    int low = 1, high = size;
    while (low <= high) {
        int mid1 = low + (high - low) / 3;
        int mid2 = high - (high - low) / 3;
        (*comparisons)++; if (arr[mid1] == target) return mid1;
        (*comparisons)++; if (arr[mid2] == target) return mid2;
        (*comparisons)++;
        if (target < arr[mid1]) high = mid1 - 1;
        else {
            (*comparisons)++;
            if (target > arr[mid2]) low = mid2 + 1;
            else { low = mid1 + 1; high = mid2 - 1; }
        }
    }
    return -1;
}
```

## Graph Analysis
The complexity graph plots the number of comparisons vs array size `N` for both algorithms.
- **Logarithmic Growth:** Both curves flatten out logarithmically, proving their `O(log N)` nature visually.
- **The Gap:** The Ternary Search curve consistently sits **above** the Binary Search curve. At every data point, the height of the Ternary curve is approximately 1.26 times the height of the Binary curve.
- **Divergence:** As `N` grows astronomically large, the absolute numerical gap between the two curves widens, further visualizing why Binary Search remains the superior choice in practical software development.
