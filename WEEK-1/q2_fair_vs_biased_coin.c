/* ============================================================================
 *  Fair vs Biased Coin Simulation
 *  - Demonstrates Law of Large Numbers (convergence to 0.5 for fair coin).
 *  - Compares varying biases and exports data to 'fair_vs_biased_coin.csv'.
 *  - Visualizes observed probabilities with a standard ASCII terminal graph.
 * ==========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define CONVERGENCE_STEPS  6
#define NUM_COINS          5
#define HISTOGRAM_WIDTH    50

/* Struct to keep data encapsulated and avoid messy global arrays */
typedef struct {
    int expected_bias_pct;
    long long heads_count;
    long long tails_count;
    double observed_prob;
} CoinExperiment;

/* ===== Helper: Simulate a single coin toss ===== */
static int flip_coin(int bias_pct) 
{
    return (rand() % 100) < bias_pct;
}

/* ===== Phase 1: Show convergence of a fair coin (50%) ===== */
static void display_fair_convergence(void) 
{
    printf("\n--- 1. FAIR COIN CONVERGENCE (50%% Bias) ---\n");
    printf("%-12s %-10s %-10s %-15s %-15s\n", "Flips", "Heads", "Tails", "Observed Prob", "Absolute Error");
    printf("----------------------------------------------------------------------\n");

    long long flips = 10;
    for (int step = 0; step < CONVERGENCE_STEPS; step++) {
        long long heads = 0, tails = 0;
        
        for (long long i = 0; i < flips; i++) {
            if (flip_coin(50)) heads++;
            else               tails++;
        }
        
        double observed = (double)heads / flips;
        printf("%-12lld %-10lld %-10lld %-15.6f %-15.6f\n",
               flips, heads, tails, observed, fabs(observed - 0.5));
               
        flips *= 10; /* Scale up by 10x each step to show convergence */
    }
}

/* ===== Phase 2: Compare fair vs biased coins & Export to CSV ===== */
static void compare_and_export(CoinExperiment coins[], int total_flips) 
{
    FILE *csv_file = fopen("fair_vs_biased_coin.csv", "w");
    if (!csv_file) {
        fprintf(stderr, "Error: Could not create 'fair_vs_biased_coin.csv'\n");
        return;
    }

    fprintf(csv_file, "TargetBiasPct,Heads,Tails,ObservedProb,ExpectedProb,Error\n");

    printf("\n--- 2. FAIR VS BIASED COMPARISON (%d flips each) ---\n", total_flips);
    printf("%-8s %-10s %-10s %-12s %-12s %-12s\n", "Bias", "Heads", "Tails", "Observed", "Expected", "Error");
    printf("----------------------------------------------------------------------\n");

    for (int i = 0; i < NUM_COINS; i++) {
        long long h = 0, t = 0;
        
        for (int f = 0; f < total_flips; f++) {
            if (flip_coin(coins[i].expected_bias_pct)) h++;
            else                                       t++;
        }

        coins[i].heads_count = h;
        coins[i].tails_count = t;
        coins[i].observed_prob = (double)h / total_flips;

        double expected = coins[i].expected_bias_pct / 100.0;
        double error = fabs(coins[i].observed_prob - expected);

        printf("%2d%%      %-10lld %-10lld %-12.5f %-12.5f %-12.5f\n",
               coins[i].expected_bias_pct, h, t, coins[i].observed_prob, expected, error);

        /* Write row to CSV */
        fprintf(csv_file, "%d,%lld,%lld,%.5f,%.5f,%.5f\n",
                coins[i].expected_bias_pct, h, t, coins[i].observed_prob, expected, error);
    }

    fclose(csv_file);
    printf("\n[*] Data successfully exported to 'fair_vs_biased_coin.csv'.\n");
}

/* ===== Phase 3: Display Standard ASCII Histogram ===== */
static void draw_histogram(const CoinExperiment coins[]) 
{
    printf("\n--- 3. ASCII VISUALIZATION (Observed Probabilities) ---\n");
    for (int i = 0; i < NUM_COINS; i++) {
        printf("Bias %2d%% (%.4f) | ", coins[i].expected_bias_pct, coins[i].observed_prob);
        
        int bar_length = (int)(coins[i].observed_prob * HISTOGRAM_WIDTH + 0.5);
        for (int j = 0; j < bar_length; j++) {
            putchar('#'); /* Using standard # to prevent cross-platform encoding bugs */
        }
        putchar('\n');
    }
    printf("\n");
}

/* ===== MAIN ============================================================== */
int main(void) 
{
    srand((unsigned int)time(NULL));

    /* Initialize array of coin structs with distinct biases */
    CoinExperiment my_coins[NUM_COINS] = {
        {50, 0, 0, 0.0},
        {60, 0, 0, 0.0},
        {70, 0, 0, 0.0},
        {90, 0, 0, 0.0},
        {30, 0, 0, 0.0}
    };

    int flip_count;
    printf("Enter number of flips for the comparison matrix: ");
    if (scanf("%d", &flip_count) != 1 || flip_count <= 0) {
        flip_count = 10000; /* Safe fallback */
        printf("Invalid input. Defaulting to %d flips.\n", flip_count);
    }

    display_fair_convergence();
    compare_and_export(my_coins, flip_count);
    draw_histogram(my_coins);

    return 0;
}