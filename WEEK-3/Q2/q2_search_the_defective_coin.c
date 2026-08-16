#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Function Prototypes
int weighScale(const int* coins, int leftStart, int leftEnd, int rightStart, int rightEnd);
int findDefectiveCoin(const int* coins, int n);
void printExplanation(int n);

int weighScale(const int* coins, int leftStart, int leftEnd, int rightStart, int rightEnd) {
    long long weightLeft = 0;   /* long long sums prevent overflow for large n */
    long long weightRight = 0;

    for (int i = leftStart; i < leftEnd; i++) {
        weightLeft += coins[i];
    }
    for (int i = rightStart; i < rightEnd; i++) {
        weightRight += coins[i];
    }

    if (weightLeft < weightRight) return -1;
    if (weightRight < weightLeft) return 1;
    return 0;
}

int findDefectiveCoin(const int* coins, int n) {
    // NOTE: n < 2 is already validated and handled in main() before this call.
    // The guard below is retained only as a defensive safety net.
    if (n < 2) return -1;

    int start = 0;
    int end = n;
    bool defectGuaranteed = false;

    // Iterative Divide and Conquer Loop
    while ((end - start) > 1) {
        int count = end - start;
        int mid = count / 2;

        int leftStart = start;
        int leftEnd = start + mid;
        int rightStart = leftEnd;
        int rightEnd = rightStart + mid;

        // Perform Scale Weighing Operation
        int scaleResult = weighScale(coins, leftStart, leftEnd, rightStart, rightEnd);

        if (scaleResult == -1) {
            // Left pan is lighter -> defective coin is inside left half
            start = leftStart;
            end = leftEnd;
            defectGuaranteed = true;
        } 
        else if (scaleResult == 1) {
            // Right pan is lighter -> defective coin is inside right half
            start = rightStart;
            end = rightEnd;
            defectGuaranteed = true;
        } 
        else {
            // Pans balance perfectly
            if (count % 2 == 0) {
                // Even set balanced -> All coins in this sub-range are perfect
                return -1; 
            } else {
                // Odd set balanced -> The 1 set-aside coin is our suspect
                int extraIdx = end - 1;
                int knownGoodIdx = start; // Coins in balanced pans are proven perfect

                // Final verification against a known good coin
                if (weighScale(coins, extraIdx, extraIdx + 1, knownGoodIdx, knownGoodIdx + 1) == -1) {
                    return extraIdx;
                }
                return -1;
            }
        }
    }

    // Base case: narrowed down to 1 coin that was proven defective in a prior step
    if ((end - start) == 1 && defectGuaranteed) {
        return start;
    }

    return -1;
}

int main() {
    int n;

    printf("=====================================================\n");
    printf("   DEFECTIVE COIN SEARCH (DIVIDE & CONQUER O(log n)) \n");
    printf("=====================================================\n\n");

    // 1. Interactive User Input for Number of Coins
    printf("Enter total number of coins (n): ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        fprintf(stderr, "\n[Error]: Invalid input! Number of coins must be a positive integer.\n");
        return EXIT_FAILURE;
    }

    if (n < 2) {
        printf("\n[Notice]: At least 2 coins are required to perform a balance test.\n");
        printf("A single coin cannot be certified defective without a standard reference weight.\n");
        return EXIT_SUCCESS;
    }

    // 2. Dynamic Memory Allocation
    int* coins = (int*)malloc((size_t)n * sizeof(int));
    if (!coins) {
        fprintf(stderr, "\n[Fatal Error]: Memory allocation failed!\n");
        return EXIT_FAILURE;
    }

    // 3. Interactive Input for Coin Weights
    printf("\nEnter weights for %d coins:\n", n);
    for (int i = 0; i < n; i++) {
        printf(" Coin [%d] weight: ", i);
        if (scanf("%d", &coins[i]) != 1) {
            fprintf(stderr, "\n[Error]: Invalid weight entered!\n");
            free(coins);
            return EXIT_FAILURE;
        }
    }

    // 4. Run Divide & Conquer Search
    int resultIndex = findDefectiveCoin(coins, n);

    // 5. Output Results
    printf("\n-----------------------------------------------------\n");
    printf("SEARCH RESULT:\n");
    if (resultIndex != -1) {
        printf(" -> DEFECTIVE COIN FOUND at Index [%d] (Weight: %d)\n",
               resultIndex, coins[resultIndex]);
    } else {
        printf(" -> NO DEFECTIVE COIN FOUND! All coins have identical weight.\n");
    }
    printf("-----------------------------------------------------\n");

    // 6. Print Full Academic Explanation
    printExplanation(n);

    // 7. Cleanup
    free(coins);
    return EXIT_SUCCESS;
}

void printExplanation(int n) {
    printf("\n=====================================================\n");
    printf("     ALGORITHM CONCEPT & COMPLEXITY EXPLANATION      \n");
    printf("=====================================================\n");
    printf("1. ALGORITHMIC MECHANICS & DIVIDE-AND-CONQUER:\n");
    printf("   - Divide: The array of n coins is divided into two equal halves of size floor(n/2).\n");
    printf("   - Conquer: The two equal halves are placed on the balance weighing scale.\n");
    printf("   - Branch 1 (Scale Tips Left): Defective coin is in the left half.\n");
    printf("   - Branch 2 (Scale Tips Right): Defective coin is in the right half.\n");
    printf("   - Branch 3 (Scale Balances):\n");
    printf("     * If n is EVEN: All coins in the current sub-range are identical (no defect).\n");
    printf("     * If n is ODD : The single set-aside coin is tested against a known good coin.\n\n");

    printf("2. RECURRENCE RELATION:\n");
    printf("   - Base Case  (n < 2) : T(n) = O(1)\n");
    printf("   - Recurrence (n >= 2): T(n) = T(n/2) + 1\n");
    printf("     where '1' represents a single physical scale weighing operation.\n\n");

    printf("3. TIME COMPLEXITY DERIVATION (MASTER THEOREM):\n");
    printf("   [A] Physical Scale Weighings (as asked by the question):\n");
    printf("   - Recurrence Form: W(n) = W(n/2) + 1  [1 weighing per step]\n");
    printf("   - Parameters     : a = 1, b = 2, f(n) = O(1)\n");
    printf("   - Critical Value : n^(log_2(1)) = n^0 = 1 = Theta(f(n)) -> Case 2\n");
    printf("     W(n) = Theta(log n) => Maximum Weighings = ceil(log_2 n) + 1\n");
    printf("   - This satisfies the problem requirement: log_2(n) + c weighings.\n\n");
    printf("   [B] Total Computational Complexity (CPU operations):\n");
    printf("   - Each weighScale() call loops over n/2 elements -> O(n) per call\n");
    printf("   - There are O(log n) calls, so total CPU work = O(n) * O(log n)\n");
    printf("   - Total Computational Time Complexity = O(n log n)\n");
    printf("   - To achieve true O(log n) computation, coins would need\n");
    printf("     to be pre-tagged with group sums (prefix sums), allowing\n");
    printf("     O(1) per weighing instead of O(n).\n\n");

    printf("4. SPACE COMPLEXITY ANALYSIS:\n");
    printf("   - Call Stack Aux Space = O(1) (Iterative Binary Search eliminates recursion overhead)\n");
    printf("   - Heap Allocation Space= O(1) aux memory during execution.\n");
    printf("=====================================================\n");
}
