/* ============================================================================
 *  Towers of Hanoi (ToH) Simulation
 *  - Simulates the step-by-step solution.
 *  - Plots total moves required for solving n-discs.
 *  - Exports the dataset to 'towers_of_hanoi.csv'.
 *  - Concludes the algorithm's performance based on the obtained plot.
 * ==========================================================================*/
#include <stdio.h>
#include <stdlib.h>

#define MAX_DISCS 25
#define DEMO_DISCS 3

/* Structure to hold simulation data cleanly */
typedef struct {
    int disc_count;
    unsigned long long simulated_moves;
    unsigned long long theoretical_moves;
} HanoiRecord;

/* ===== Core Recursive Algorithm ===== */
/* Note: Passing step_counter by pointer instead of using a global variable */
static void execute_transfer(int n, char src, char dest, char aux, unsigned long long *step_counter, int show_output) 
{
    if (n == 0) {
        return;
    }
    
    execute_transfer(n - 1, src, aux, dest, step_counter, show_output);
    
    (*step_counter)++;
    if (show_output) {
        printf("  Step %-4llu : Move disc %d from Peg %c -> Peg %c\n", *step_counter, n, src, dest);
    }
    
    execute_transfer(n - 1, aux, dest, src, step_counter, show_output);
}

/* ===== Simulation Runners ===== */
static void demonstrate_small_n(int n) 
{
    unsigned long long steps = 0;
    printf("\n--- VISUAL DEMONSTRATION (N = %d discs) ---\n", n);
    execute_transfer(n, 'A', 'C', 'B', &steps, 1);
    printf("  Total Moves: %llu\n", steps);
}

static void run_batch_simulation(HanoiRecord *dataset, int max_n) 
{
    for (int i = 1; i <= max_n; i++) {
        unsigned long long steps = 0;
        execute_transfer(i, 'A', 'C', 'B', &steps, 0); /* 0 = silent execution */
        
        dataset[i - 1].disc_count = i;
        dataset[i - 1].simulated_moves = steps;
        dataset[i - 1].theoretical_moves = (1ULL << i) - 1; /* Formula: 2^n - 1 */
    }
}

/* ===== I/O and Visualization ===== */
static void generate_csv_file(const HanoiRecord *dataset, int max_n) 
{
    const char *filename = "towers_of_hanoi.csv";
    FILE *fp = fopen(filename, "w");
    
    if (fp == NULL) {
        fprintf(stderr, "[!] Error: Could not create %s\n", filename);
        return;
    }

    fprintf(fp, "DiscCount,SimulatedMoves,FormulaMoves\n");
    for (int i = 0; i < max_n; i++) {
        fprintf(fp, "%d,%llu,%llu\n", 
                dataset[i].disc_count, 
                dataset[i].simulated_moves, 
                dataset[i].theoretical_moves);
    }

    fclose(fp);
    printf("\n[*] Dataset successfully exported to '%s'.\n", filename);
}

static void render_analysis_dashboard(const HanoiRecord *dataset, int max_n) 
{
    printf("\n--- PERFORMANCE ANALYSIS & LOG-SCALE GRAPH ---\n");
    printf("%-7s | %-15s | %-15s | %s\n", "Discs", "Simulated", "Theoretical", "Growth Plot (Log Scale)");
    printf("--------+-----------------+-----------------+--------------------------------\n");

    for (int i = 0; i < max_n; i++) {
        printf(" %-6d | %-15llu | %-15llu | ", 
               dataset[i].disc_count, 
               dataset[i].simulated_moves, 
               dataset[i].theoretical_moves);
               
        /* Graph plotting: Since moves double every step, making the bar length 
           equal to 'n' creates a perfect Logarithmic representation of the exponential curve */
        for (int j = 0; j < dataset[i].disc_count; j++) {
            putchar('#');
        }
        putchar('\n');
    }
}

static void print_analytical_conclusion(void) 
{
    printf("\n========================================================================\n");
    printf(" CONCLUSION: What can be concluded from the plot obtained?\n");
    printf("========================================================================\n");
    printf(" 1. Exponential Growth: The table and formula confirm the time\n");
    printf("    complexity is precisely O(2^n). For every single disc added, the\n");
    printf("    total number of required moves essentially doubles.\n");
    printf(" 2. Practical Limitation: The log-scale ASCII plot illustrates a\n");
    printf("    perfect linear trend, which implies the underlying data is growing\n");
    printf("    exponentially. The algorithm becomes computationally infeasible\n");
    printf("    for large values of n (e.g., n > 64).\n");
    printf(" 3. Space Efficiency: Despite exponential time, the recursion depth\n");
    printf("    only goes 'n' levels deep, resulting in a space complexity of O(n).\n");
    printf("========================================================================\n\n");
}

/* ===== MAIN ============================================================== */
int main(void) 
{
    HanoiRecord simulation_data[MAX_DISCS];

    printf("========================================================================\n");
    printf(" TOWERS OF HANOI (ToH) SIMULATOR\n");
    printf("========================================================================\n");

    /* 1. Show a small visual example */
    demonstrate_small_n(DEMO_DISCS);

    /* 2. Run the heavy simulation invisibly */
    run_batch_simulation(simulation_data, MAX_DISCS);

    /* 3. Output results and create file */
    render_analysis_dashboard(simulation_data, MAX_DISCS);
    generate_csv_file(simulation_data, MAX_DISCS);
    
    /* 4. Answer the specific question asked in the assignment prompt */
    print_analytical_conclusion();

    return 0;
}