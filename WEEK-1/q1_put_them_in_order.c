/* ============================================================================
 *  DAA Assignment Q1 : place the given functions in increasing order of growth
 *  - Prints the asymptotic order on a single line with strict inequalities (<).
 *  - Displays the actual computed values for a large n.
 *  - Includes ASCII terminal graph visualization.
 * ==========================================================================*/
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>

#define FUNC_COUNT       12
#define CHART_COLS       52
#define LARGE_N_VAL      1e8    /* Large n to demonstrate computed values */
#define MAX_N_VAL        1e100  /* A massively large n for the ASCII graph */
#define INF_LOG_THRESH   305.0  /* Threshold to prevent pow() overflow crashes */

#define GRP_INVERSE      0
#define GRP_LOG          1
#define GRP_POLY         2
#define GRP_NLOGN        3
#define GRP_SUPERPOLY    4
#define GRP_EXPONENT     5

typedef struct
{
    char   eqName[32];
    int    eqGroup;
    double eqPower;
    double eqCoeff;            /* Added to tie-break identical Big-O classes */
    double (*fnEval)(double);  /* Computes actual value */
    double (*fnMag)(double);   /* Computes log10 of value for graph scaling */
} EqnInfo;

typedef int (*SortCmpFunc)(const EqnInfo *, const EqnInfo *);

/* ===== SECTION 1 : Value and Magnitude Functions ========================= */
static double calc_safe_pow10(double x)  { return (x > INF_LOG_THRESH) ? HUGE_VAL : pow(10.0, x); }
static double calc_log10_floor(double v) { return (v > 0.0) ? log10(v) : -DBL_MAX / 4.0; }

/* Actual Evaluators */
static double calc_f_inv(double n)       { return 1.0 / n; }
static double calc_f_log2(double n)      { return log2(n); }
static double calc_f_12sqrt(double n)    { return 12.0 * sqrt(n); }
static double calc_f_50sqrt(double n)    { return 50.0 * pow(n, 0.5); }
static double calc_f_pow051(double n)    { return pow(n, 0.51); }
static double calc_f_lin2p32(double n)   { return pow(2.0, 32.0) * n; }
static double calc_f_nlogn(double n)     { return n * log2(n); }
static double calc_f_quad100(double n)   { return 100.0 * n * n + 6.0 * n; }
static double calc_f_quadm324(double n)  { return n * n - 324.0; }
static double calc_f_cubic2(double n)    { return 2.0 * n * n * n; }
static double calc_f_npowlogn(double n)  { return calc_safe_pow10(log2(n) * log10(n)); }
static double calc_f_exp3(double n)      { return calc_safe_pow10(n * log10(3.0)); }

/* Magnitude (log10) Evaluators */
static double calc_m_inv(double n)       { return -log10(n); }
static double calc_m_log2(double n)      { return calc_log10_floor(log2(n)); }
static double calc_m_12sqrt(double n)    { return log10(12.0) + 0.5 * log10(n); }
static double calc_m_50sqrt(double n)    { return log10(50.0) + 0.5 * log10(n); }
static double calc_m_pow051(double n)    { return 0.51 * log10(n); }
static double calc_m_lin2p32(double n)   { return 32.0 * log10(2.0) + log10(n); }
static double calc_m_nlogn(double n)     { return log10(n) + calc_log10_floor(log2(n)); }
static double calc_m_quad100(double n)   { return log10(n) + log10(100.0 * n + 6.0); }
static double calc_m_quadm324(double n)  { return calc_log10_floor(n * n - 324.0); }
static double calc_m_cubic2(double n)    { return log10(2.0) + 3.0 * log10(n); }
static double calc_m_npowlogn(double n)  { return log2(n) * log10(n); }
static double calc_m_exp3(double n)      { return n * log10(3.0); }

/* ===== SECTION 2 : Function Table Initialization ========================= */
static EqnInfo equations[FUNC_COUNT];

static void initMathFunc(int idx, const char *title, int grp, double pwr, double coeff,
                         double (*evalFn)(double), double (*magFn)(double))
{
    EqnInfo *eq = &equations[idx];
    snprintf(eq->eqName, sizeof eq->eqName, "%s", title);
    eq->eqGroup = grp;  
    eq->eqPower = pwr;
    eq->eqCoeff = coeff; /* Store coefficient for strict tie-breaking */
    eq->fnEval = evalFn;
    eq->fnMag = magFn;
}

static void populateFuncArray(void)
{
    initMathFunc(0,  "1/n",         GRP_INVERSE,   -1.0, 1.0,          calc_f_inv,      calc_m_inv);
    initMathFunc(1,  "log2(n)",     GRP_LOG,        0.0, 1.0,          calc_f_log2,     calc_m_log2);
    initMathFunc(2,  "12*sqrt(n)",  GRP_POLY,       0.5, 12.0,         calc_f_12sqrt,   calc_m_12sqrt);
    initMathFunc(3,  "50*n^0.5",    GRP_POLY,       0.5, 50.0,         calc_f_50sqrt,   calc_m_50sqrt);
    initMathFunc(4,  "n^0.51",      GRP_POLY,      0.51, 1.0,          calc_f_pow051,   calc_m_pow051);
    initMathFunc(5,  "(2^32)*n",    GRP_POLY,       1.0, 4294967296.0, calc_f_lin2p32,  calc_m_lin2p32);
    initMathFunc(6,  "n*log2(n)",   GRP_NLOGN,      1.0, 1.0,          calc_f_nlogn,    calc_m_nlogn);
    initMathFunc(7,  "100*n^2+6*n", GRP_POLY,       2.0, 100.0,        calc_f_quad100,  calc_m_quad100);
    initMathFunc(8,  "n^2-324",     GRP_POLY,       2.0, 1.0,          calc_f_quadm324, calc_m_quadm324);
    initMathFunc(9,  "2*n^3",       GRP_POLY,       3.0, 2.0,          calc_f_cubic2,   calc_m_cubic2);
    initMathFunc(10, "n^(log2 n)",  GRP_SUPERPOLY,  0.0, 1.0,          calc_f_npowlogn, calc_m_npowlogn);
    initMathFunc(11, "3^n",         GRP_EXPONENT,   3.0, 1.0,          calc_f_exp3,     calc_m_exp3);
}

/* ===== SECTION 3 : Theoretical Sorting (Merge Sort) ====================== */
static int getGroupLevel(int grp)
{
    switch (grp) {
        case GRP_INVERSE:    return 0;  
        case GRP_LOG:        return 1;  
        case GRP_POLY:                  
        case GRP_NLOGN:      return 2;  
        case GRP_SUPERPOLY:  return 3;  
        default:             return 4;  
    }
}

static int cmpGrowthClass(const EqnInfo *eq1, const EqnInfo *eq2)
{
    int lvl1 = getGroupLevel(eq1->eqGroup), lvl2 = getGroupLevel(eq2->eqGroup);
    if (lvl1 != lvl2) return (lvl1 < lvl2) ? -1 : 1;  
    
    if (lvl1 == 2) { 
        if (eq1->eqPower < eq2->eqPower) return -1;
        if (eq1->eqPower > eq2->eqPower) return  1;
        if (eq1->eqGroup != eq2->eqGroup)
            return (eq1->eqGroup == GRP_NLOGN) ? 1 : -1;
    }
    
    /* Strict Tie-Breaker: Resolves identical Big-O classes using their coefficients */
    if (eq1->eqCoeff < eq2->eqCoeff) return -1;
    if (eq1->eqCoeff > eq2->eqCoeff) return  1;
    
    return 0; 
}

static EqnInfo tempSortBuf[FUNC_COUNT];
static void combineArrays(EqnInfo arr[], int left, int mid, int right, SortCmpFunc cmpRoutine)
{
    int iter1 = left, iter2 = mid + 1, k = left;
    while (iter1 <= mid && iter2 <= right) {
        if (cmpRoutine(&arr[iter1], &arr[iter2]) <= 0) tempSortBuf[k++] = arr[iter1++];
        else                                           tempSortBuf[k++] = arr[iter2++];
    }
    while (iter1 <= mid) tempSortBuf[k++] = arr[iter1++];
    while (iter2 <= right) tempSortBuf[k++] = arr[iter2++];
    for (k = left; k <= right; k++) arr[k] = tempSortBuf[k];
}

static void sortArrayRange(EqnInfo arr[], int left, int right, SortCmpFunc cmpRoutine)
{
    if (left >= right) return;              
    int midPt = left + (right - left) / 2;      
    sortArrayRange(arr, left, midPt, cmpRoutine);
    sortArrayRange(arr, midPt + 1, right, cmpRoutine);
    combineArrays(arr, left, midPt, right, cmpRoutine);
}

/* ===== SECTION 4 : Output Display ======================================== */
static void displayAsymptoticOrder(void)
{
    printf("\n========================================================================\n"
           " 1. ASYMPTOTIC ORDER OF GROWTH (Single Line Strict Inequality)\n"
           "========================================================================\n ");
           
    for (int idx = 0; idx < FUNC_COUNT; idx++) {
        printf("%s", equations[idx].eqName);
        if (idx + 1 < FUNC_COUNT) {
            printf("%s", cmpGrowthClass(&equations[idx], &equations[idx + 1]) == 0 
                   ? " = " : " < ");
        }
    }
    printf("\n");
}

static void displayValuesAtLargeN(double test_n)
{
    printf("\n=======================================================\n"
           " 2. COMPUTED VALUES AT LARGE n (n = %.0e)\n"
           "=======================================================\n", test_n);
           
    for (int idx = 0; idx < FUNC_COUNT; idx++) {
        double val = equations[idx].fnEval(test_n);
        if (isinf(val)) {
            printf(" %-14s : INF (Overflows standard double limits)\n", equations[idx].eqName);
        } else {
            printf(" %-14s : %.4e\n", equations[idx].eqName, val);
        }
    }
}

static void drawAsciiBars(void)
{
    double scores[FUNC_COUNT], minScore = DBL_MAX, maxScore = -DBL_MAX;
    
    printf("\n=======================================================\n"
           " 3. ASCII GROWTH VISUALIZATION (at n = 1e100)\n"
           "=======================================================\n"
           "Log-of-log scale: Bar size ~ log10(decades of f(n)).\n\n");
    
    for (int fIdx = 0; fIdx < FUNC_COUNT; fIdx++) {
        scores[fIdx] = log10(equations[fIdx].fnMag(MAX_N_VAL) + 10.0);
        if (scores[fIdx] < minScore) minScore = scores[fIdx];
        if (scores[fIdx] > maxScore) maxScore = scores[fIdx];
    }
    
    for (int i = 0; i < FUNC_COUNT; i++) {
        int barLen = 1 + (int)((scores[i] - minScore) / (maxScore - minScore) * (CHART_COLS - 1));
        printf("%-14s ", equations[i].eqName);
        while (barLen-- > 0) printf("\u2588");
        putchar('\n');
    }
    printf("\n");
}

/* ===== MAIN ============================================================== */
int main(void)
{
    populateFuncArray();             
    
    /* Sort purely based on theoretical bounds, including tie-breakers */
    sortArrayRange(equations, 0, FUNC_COUNT - 1, cmpGrowthClass); 
    
    displayAsymptoticOrder();
    displayValuesAtLargeN(LARGE_N_VAL);
    drawAsciiBars();
    
    return 0;
}