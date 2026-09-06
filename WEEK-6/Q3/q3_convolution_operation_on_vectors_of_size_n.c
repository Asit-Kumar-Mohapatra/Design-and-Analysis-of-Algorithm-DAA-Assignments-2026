/* =====================================================================================
 *  CONVOLUTION OF TWO VECTORS  --  O(n log n) DIVIDE & CONQUER (FFT) ALGORITHM
 * =====================================================================================
 *
 *  PROBLEM
 *  -------
 *  Given A (length m) and B (length n), n >= m, 0-indexed, compute
 *
 *              C[k] = sum_{j=0}^{m-1} A[j] * B[k-j]      (B[t] = 0 for t<0 or t>=n)
 *
 *  which is exactly the coefficient vector of the product of the two polynomials
 *
 *              PA(x) = A[0] + A[1]x + ... + A[m-1] x^(m-1)
 *              PB(x) = B[0] + B[1]x + ... + B[n-1] x^(n-1)
 *
 *  i.e.  C = A * B  (polynomial multiplication).  The result C has length m+n-1.
 *
 *  WHY THE NAIVE ALGORITHM IS NOT GOOD ENOUGH
 *  --------------------------------------------
 *  The direct definition above needs O(m*n) multiplications (a double loop) -
 *  O(n^2) when m = Theta(n).  We are asked for O(n log n).
 *
 *  THE DIVIDE & CONQUER IDEA (Cooley-Tukey FFT)
 *  ---------------------------------------------
 *  The key fact from algebra:
 *
 *      A polynomial of degree < N is UNIQUELY determined by its values at
 *      any N distinct points (point-value representation), and
 *
 *      (PA * PB)(x_i) = PA(x_i) * PB(x_i)      -- pointwise product!
 *
 *  So instead of multiplying coefficient-by-coefficient (O(N^2)), we:
 *
 *      1) EVALUATE   PA and PB at N (a power of two, N >= m+n-1) points  -> O(N log N)
 *      2) MULTIPLY   the N pairs of values pointwise                     -> O(N)
 *      3) INTERPOLATE back to coefficient form                           -> O(N log N)
 *
 *  Steps 1 and 3 are cheap ONLY if we choose the evaluation points cleverly:
 *  the N-th complex roots of unity  w_N^k = e^{-2*pi*i*k/N},  k = 0..N-1.
 *  Evaluating (and interpolating) a degree-(N-1) polynomial at ALL N roots of
 *  unity simultaneously is exactly the (inverse) Discrete Fourier Transform,
 *  and it has a divide-and-conquer O(N log N) algorithm: the FFT.
 *
 *  THE DIVIDE & CONQUER RECURRENCE FOR THE FFT ITSELF
 *  ----------------------------------------------------
 *  Split a polynomial's N coefficients into its even-indexed and odd-indexed
 *  halves:
 *          P(x)  =  P_even(x^2)  +  x * P_odd(x^2)
 *
 *  P_even and P_odd each have N/2 coefficients.  Because w_N^2 = w_{N/2},
 *  evaluating P_even and P_odd at the N/2 roots of unity of order N/2 gives
 *  US EVERYTHING needed to reconstruct P at all N roots of unity of order N,
 *  using the "butterfly" combine step:
 *
 *          P(w_N^k)         = P_even(w_{N/2}^k) + w_N^k * P_odd(w_{N/2}^k)
 *          P(w_N^{k+N/2})   = P_even(w_{N/2}^k) - w_N^k * P_odd(w_{N/2}^k)
 *
 *  Recurrence:   T(N) = 2*T(N/2) + O(N)   =>   T(N) = O(N log N)     (Master theorem)
 *
 *  This recursive halving (even part / odd part, solved independently and
 *  combined in linear time) is precisely the "divide and conquer" algorithm
 *  requested by the problem.  The implementation below realizes it with an
 *  *iterative* bottom-up version of the same recursion (bit-reversal
 *  permutation + log N butterfly passes). This is mathematically identical
 *  to the recursive version but avoids recursion/call overhead, giving the
 *  best constant factor while keeping the exact same O(N log N) divide and
 *  conquer structure (each pass merges solutions of half the size, exactly
 *  as recursion would, just processed level-by-level instead of via the
 *  call stack). A pure top-down recursive version is also included
 *  (fft_recursive) so the divide-and-conquer structure is directly visible;
 *  the convolution itself uses the fast iterative engine.
 *
 *  ALGORITHM FOR THE CONVOLUTION (what convolve() below does)
 *  -------------------------------------------------------------
 *      1. N = smallest power of two >= m + n - 1   (result length is m+n-1)
 *      2. Zero-pad A and B to length N, copy into complex arrays.
 *      3. FFT(A), FFT(B)                         -- O(N log N) each
 *      4. Pointwise multiply: A[k] *= B[k]  for k = 0..N-1     -- O(N)
 *      5. Inverse FFT(A)                         -- O(N log N)
 *      6. Round the real parts of the first (m+n-1) entries to get integers.
 *
 *  Total time: O(N log N) = O((m+n) log(m+n)) = O(n log n)  since n >= m
 *  (so m+n-1 <= 2n-1 = Theta(n), and N is at most the next power of two of
 *  that, i.e. Theta(n)).
 *
 *  NUMERIC NOTE
 *  ------------
 *  FFT convolution works over complex (floating point) numbers, so results
 *  are rounded to the nearest integer at the end. This is safe/exact for
 *  integer inputs as long as intermediate magnitudes stay well inside double
 *  precision range (true for all reasonable test sizes/values used here).
 *
 *  Compile:      gcc -O2 -Wall -std=c11 convolution.c -lm -o convolution
 *  Run (input):  ./convolution            (prompts you for A and B, runs
 *                                           the FFT convolution on THEM, and
 *                                           prints a worked explanation of
 *                                           every step)
 *  Run demo:     ./convolution demo       (fixed built-in example, no typing)
 *  Run tests:    ./convolution test [N]   (N randomized tests, default 30000)
 * ===================================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <time.h>
#include <ctype.h>

typedef double complex cplx;
#define PI 3.14159265358979323846

/* -------------------------------------------------------------------------------------
 * next_pow2 : smallest power of two >= x   (x >= 1)
 * ------------------------------------------------------------------------------------- */
static size_t next_pow2(size_t x) {
    size_t p = 1;
    while (p < x) p <<= 1;
    return p;
}

/* -------------------------------------------------------------------------------------
 * fft : in-place iterative Cooley-Tukey FFT / inverse-FFT, length N (power of two).
 *
 * invert == 0 -> forward DFT   (evaluate polynomial at roots of unity)
 * invert == 1 -> inverse DFT   (interpolate back to coefficients, divides by N)
 *
 * This is the bottom-up realization of the divide-and-conquer recursion
 * described above:
 *   - "bit-reversal permutation" places every coefficient at the position its
 *     recursive even/odd splitting would eventually place it at the base case
 *     (arrays of size 1), which is exactly what unrolling the recursion down
 *     to the leaves does.
 *   - the two nested loops that follow perform, level by level (len = 2, 4,
 *     8, ..., N), precisely the "combine" (butterfly) step of the recursion:
 *     merge two transforms of size len/2 into one transform of size len,
 *     in O(len) work per merge -> O(N) work per level -> O(N log N) total.
 * ------------------------------------------------------------------------------------- */
static void fft(cplx *a, size_t N, int invert) {
    /* ---- bit-reversal permutation (iterative equivalent of recursive split) ---- */
    for (size_t i = 1, j = 0; i < N; i++) {
        size_t bit = N >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j) {
            cplx tmp = a[i];
            a[i] = a[j];
            a[j] = tmp;
        }
    }

    /* ---- butterfly passes: merge size-(len/2) transforms into size-len ---- */
    for (size_t len = 2; len <= N; len <<= 1) {
        double ang = 2.0 * PI / (double)len * (invert ? 1.0 : -1.0);
        cplx wlen = cos(ang) + I * sin(ang);           /* principal len-th root of unity */
        for (size_t i = 0; i < N; i += len) {
            cplx w = 1.0;
            for (size_t k = 0; k < len / 2; k++) {
                cplx u = a[i + k];
                cplx v = a[i + k + len / 2] * w;
                a[i + k]           = u + v;             /* butterfly: even combine */
                a[i + k + len / 2] = u - v;              /* butterfly: odd combine  */
                w *= wlen;
            }
        }
    }

    if (invert) {
        for (size_t i = 0; i < N; i++)
            a[i] /= (double)N;
    }
}

/* -------------------------------------------------------------------------------------
 * fft_recursive : textbook top-down divide-and-conquer FFT, shown purely to make the
 * recursive structure explicit for understanding (NOT used by convolve(), which uses
 * the faster iterative fft() above for best performance).
 *
 *      T(N) = 2 T(N/2) + O(N)   =>   O(N log N)   by the Master Theorem.
 * ------------------------------------------------------------------------------------- */
static void fft_recursive(cplx *a, size_t N, int invert) {
    if (N == 1) return;                          /* base case of the recursion */

    cplx *even = malloc((N / 2) * sizeof(cplx));
    cplx *odd  = malloc((N / 2) * sizeof(cplx));
    for (size_t i = 0; i < N / 2; i++) {
        even[i] = a[2 * i];
        odd[i]  = a[2 * i + 1];
    }

    fft_recursive(even, N / 2, invert);           /* divide: solve left half  */
    fft_recursive(odd,  N / 2, invert);            /* divide: solve right half */

    double ang = 2.0 * PI / (double)N * (invert ? 1.0 : -1.0);
    cplx w = 1.0, wn = cos(ang) + I * sin(ang);
    for (size_t k = 0; k < N / 2; k++) {           /* conquer: O(N) combine    */
        cplx t = w * odd[k];
        a[k]         = even[k] + t;
        a[k + N / 2] = even[k] - t;
        if (invert) { a[k] /= 2.0; a[k + N / 2] /= 2.0; }
        w *= wn;
    }

    free(even);
    free(odd);
}

/* -------------------------------------------------------------------------------------
 * convolve : the O(n log n) divide-and-conquer convolution requested by the problem.
 *
 *   A       : input vector of length m
 *   B       : input vector of length n   (n >= m, as specified in the problem)
 *   *outLen : receives m + n - 1, the length of the convolution result.
 *
 * Returns a freshly malloc'd array of long long (caller must free()).
 * ------------------------------------------------------------------------------------- */
long long *convolve(const long long *A, size_t m, const long long *B, size_t n, size_t *outLen) {
    size_t resLen = m + n - 1;
    size_t N = next_pow2(resLen);           /* pad to a power of two >= result length */

    cplx *fa = calloc(N, sizeof(cplx));
    cplx *fb = calloc(N, sizeof(cplx));
    if (!fa || !fb) { fprintf(stderr, "allocation failure\n"); exit(EXIT_FAILURE); }

    for (size_t i = 0; i < m; i++) fa[i] = (double)A[i];
    for (size_t i = 0; i < n; i++) fb[i] = (double)B[i];

    fft(fa, N, 0);                          /* step 1: evaluate PA at N roots of unity */
    fft(fb, N, 0);                          /* step 1: evaluate PB at N roots of unity */

    for (size_t i = 0; i < N; i++)
        fa[i] *= fb[i];                     /* step 2: pointwise multiply -> O(N)      */

    fft(fa, N, 1);                          /* step 3: inverse FFT -> interpolate      */

    long long *C = malloc(resLen * sizeof(long long));
    if (!C) { fprintf(stderr, "allocation failure\n"); exit(EXIT_FAILURE); }
    for (size_t i = 0; i < resLen; i++)
        C[i] = (long long)llround(creal(fa[i]));   /* round FP noise to nearest integer */

    free(fa);
    free(fb);
    *outLen = resLen;
    return C;
}

/* -------------------------------------------------------------------------------------
 * naive_convolve : the textbook O(m*n) definition, C[k] = sum_j A[j]*B[k-j].
 * Kept as a correctness oracle for testing/verifying convolve() - not the requested
 * O(n log n) algorithm itself, but used to double-check every answer we print.
 * ------------------------------------------------------------------------------------- */
static long long *naive_convolve(const long long *A, size_t m, const long long *B, size_t n, size_t *outLen) {
    size_t resLen = m + n - 1;
    long long *C = calloc(resLen, sizeof(long long));
    for (size_t j = 0; j < m; j++)
        for (size_t k = 0; k < n; k++)
            C[j + k] += A[j] * B[k];
    *outLen = resLen;
    return C;
}

/* =====================================================================================
 *  EXPLANATION HELPERS
 * ===================================================================================== */

static void print_vector(const char *name, const long long *v, size_t len) {
    printf("%s (length %zu) = [ ", name, len);
    for (size_t i = 0; i < len; i++) printf("%lld ", v[i]);
    printf("]\n");
}

/* print_complexity_summary: a standalone function (as requested) that lays out the
 * complexity analysis of naive vs. FFT convolution, independent of any specific run. */
void print_complexity_summary(void) {
    printf(
        "+---------------------------+------------------+------------------+\n"
        "| Approach                  | Time (worst)     | Extra Space      |\n"
        "+---------------------------+------------------+------------------+\n"
        "| Naive double loop         | O(m*n) = O(n^2)  | O(m+n) (output)  |\n"
        "| FFT divide & conquer      | O(n log n)       | O(n) complex     |\n"
        "+---------------------------+------------------+------------------+\n"
        "  m, n = lengths of A, B (n >= m); N = next power of two >= m+n-1,\n"
        "  and N = Theta(n) since n >= m. FFT cost breakdown for one run:\n"
        "    FFT(A)            O(N log N)\n"
        "    FFT(B)            O(N log N)\n"
        "    pointwise multiply         O(N)\n"
        "    inverse FFT       O(N log N)\n"
        "    ---------------------------------\n"
        "    total             O(N log N) = O(n log n)\n"
        "  Recurrence for the FFT itself: T(N) = 2*T(N/2) + O(N) -> O(N log N)\n"
        "  by the Master Theorem (case 2): a=2, b=2, f(N)=O(N) = Theta(N^log_b(a)).\n"
    );
}

/* explain_and_run: the "worked example" function requested - given the user's
 * actual A and B, walks through every step of the algorithm, printing what is
 * happening and why, then prints (and, for small enough sizes, cross-checks)
 * the result. This is what interactive mode and demo mode both call. */
void explain_and_run(const long long *A, size_t m, const long long *B, size_t n) {
    printf("\n================ YOUR INPUT ================\n");
    print_vector("A", A, m);
    print_vector("B", B, n);

    size_t resLen = m + n - 1;
    size_t N = next_pow2(resLen);

    printf("\n--- Step 0: Problem restated ---\n");
    printf("We need C[k] = sum_{j=0}^{m-1} A[j]*B[k-j] for k = 0..%zu,\n", resLen - 1);
    printf("i.e. the coefficients of the product polynomial PA(x)*PB(x).\n");
    printf("Naive double loop would cost O(m*n) = O(%zu * %zu) = O(%zu) multiplications.\n",
           m, n, m * n);

    printf("\n--- Step 1: Choose N (evaluation point count) ---\n");
    printf("Result length is m+n-1 = %zu, so we need N = next power of two >= %zu.\n",
           resLen, resLen);
    printf("N = %zu.  We will evaluate both polynomials at all N complex N-th\n", N);
    printf("roots of unity, using the FFT, in O(N log N) time each.\n");

    printf("\n--- Step 2 & 3: FFT both polynomials, multiply pointwise, inverse FFT ---\n");
    printf("FFT(A) turns A's coefficients into N point-values PA(w_N^0..w_N^{N-1}).\n");
    printf("FFT(B) does the same for B. Then (PA*PB)(w_N^k) = PA(w_N^k)*PB(w_N^k)\n");
    printf("is just N pointwise complex multiplications - O(N) total, not O(N^2).\n");
    printf("An inverse FFT then interpolates those N products back to the m+n-1\n");
    printf("coefficients of C, again in O(N log N).\n");

    clock_t t0 = clock();
    size_t outLen;
    long long *C = convolve(A, m, B, n, &outLen);
    clock_t t1 = clock();
    double ms = (double)(t1 - t0) * 1000.0 / CLOCKS_PER_SEC;

    printf("\n--- Result ---\n");
    print_vector("C = A * B (via FFT)", C, outLen);
    printf("(FFT convolution computed in %.4f ms)\n", ms);

    /* Cross-check against the O(m*n) naive definition whenever it's cheap
     * enough to be a near-instant sanity check (guards against huge m*n
     * making the "explanation" run slow). */
    if (m * n <= 4000000ULL) {
        size_t naiveLen;
        long long *Cref = naive_convolve(A, m, B, n, &naiveLen);
        int match = (naiveLen == outLen);
        if (match) {
            for (size_t i = 0; i < outLen; i++)
                if (Cref[i] != C[i]) { match = 0; break; }
        }
        printf("\nCross-check vs. naive O(m*n) definition: %s\n",
               match ? "MATCH (exact integer agreement)" : "MISMATCH (unexpected!)");
        free(Cref);
    } else {
        printf("\n(Skipping the O(m*n) naive cross-check here since m*n = %zu is\n", m * n);
        printf(" large enough that the naive check itself would be slow; the FFT\n");
        printf(" path is verified against the naive oracle on ~30000 smaller\n");
        printf(" randomized cases by `./convolution test`.)\n");
    }

    printf("\n================ COMPLEXITY SUMMARY ================\n");
    print_complexity_summary();

    free(C);
}

/* =====================================================================================
 *  INPUT HANDLING
 * ===================================================================================== */
static size_t read_size_prompt(const char *prompt, size_t lo) {
    char line[256];
    long long value;
    while (1) {
        printf("%s", prompt);
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) return lo;   /* EOF -> sane fallback */
        if (sscanf(line, "%lld", &value) == 1 && value >= (long long)lo)
            return (size_t)value;
        printf("  Please enter an integer >= %zu.\n", lo);
    }
}

/* Reads `count` long long values (whitespace/newline separated, any layout)
 * from stdin into arr. On premature EOF, remaining entries are zero-filled. */
static void read_ll_vector(const char *label, long long *arr, size_t count) {
    printf("Enter the %zu elements of %s (space or newline separated):\n", count, label);
    size_t filled = 0;
    char line[4096];
    while (filled < count) {
        if (!fgets(line, sizeof(line), stdin)) {
            for (; filled < count; filled++) arr[filled] = 0;
            break;
        }
        char *p = line;
        while (*p) {
            char *end;
            long long val = strtoll(p, &end, 10);
            if (end == p) {
                if (isspace((unsigned char)*p)) { p++; continue; }
                break;
            }
            if (filled < count) arr[filled++] = val;
            p = end;
        }
    }
}

/* =====================================================================================
 *  INTERACTIVE MODE
 * ===================================================================================== */
static void run_interactive(void) {
    printf("=== Convolution of Two Vectors - Interactive Mode ===\n");
    printf("This computes C = A * B (convolution) via the O(n log n) FFT\n");
    printf("divide-and-conquer algorithm, explaining each step as it runs.\n");
    printf("Per the problem statement, B must be at least as long as A (n >= m).\n\n");

    size_t m = read_size_prompt("Enter length of vector A (m >= 1): ", 1);
    long long *A = malloc(m * sizeof(long long));
    read_ll_vector("A", A, m);

    size_t n;
    while (1) {
        n = read_size_prompt("Enter length of vector B (n >= m): ", m);
        if (n >= m) break;
        printf("  n must be >= m (%zu); please re-enter.\n", m);
    }
    long long *B = malloc(n * sizeof(long long));
    read_ll_vector("B", B, n);

    explain_and_run(A, m, B, n);

    free(A);
    free(B);
}

/* =====================================================================================
 *  FIXED-DATA DEMO (kept for quick sanity checks / grading without typing input)
 * =====================================================================================
 * A = [1, 2, 3]              (m = 3)
 * B = [4, 5, 6, 7]            (n = 4, n >= m)
 * Equivalent to multiplying (1 + 2x + 3x^2) * (4 + 5x + 6x^2 + 7x^3).
 * ===================================================================================== */
static void run_demo(void) {
    long long A[] = {1, 2, 3};
    long long B[] = {4, 5, 6, 7};
    explain_and_run(A, 3, B, 4);
}

/* =====================================================================================
 *  RANDOMIZED STRESS TEST HARNESS
 *  Compares convolve() (FFT, O(n log n)) against naive_convolve() (O(m n) oracle)
 *  over a large number of randomly generated test cases of varying sizes/values.
 * ===================================================================================== */
static long long rand_val(int range) {
    return (long long)(rand() % (2 * range + 1)) - range;
}

static int run_tests(int NUM_TESTS) {
    srand(12345);
    int failures = 0;
    long long max_abs_err = 0;

    clock_t start = clock();

    for (int t = 0; t < NUM_TESTS; t++) {
        /* vary sizes: mostly small (typical case), occasionally larger (stress) */
        size_t m, n;
        int sizeClass = t % 100;
        if (sizeClass < 80) {            /* 80%: small, m in [1,20], n in [m,25] */
            m = 1 + rand() % 20;
            n = m + rand() % (26 - (int)m);
            if (n < m) n = m;
        } else if (sizeClass < 97) {     /* 17%: medium, m in [1,200], n in [m,300] */
            m = 1 + rand() % 200;
            int span = 301 - (int)m;
            n = m + rand() % (span > 0 ? span : 1);
        } else {                         /* 3%: larger stress sizes */
            m = 1 + rand() % 800;
            n = m + rand() % 500;
        }
        if (n < m) n = m;

        int range = (rand() % 3 == 0) ? 1000 : 20;  /* mix small & larger coefficient magnitudes */

        long long *A = malloc(m * sizeof(long long));
        long long *B = malloc(n * sizeof(long long));
        for (size_t i = 0; i < m; i++) A[i] = rand_val(range);
        for (size_t i = 0; i < n; i++) B[i] = rand_val(range);

        size_t lenFFT, lenNaive;
        long long *cFFT   = convolve(A, m, B, n, &lenFFT);
        long long *cNaive = naive_convolve(A, m, B, n, &lenNaive);

        int ok = (lenFFT == lenNaive);
        if (ok) {
            for (size_t i = 0; i < lenFFT; i++) {
                long long diff = llabs(cFFT[i] - cNaive[i]);
                if (diff > max_abs_err) max_abs_err = diff;
                if (diff != 0) { ok = 0; break; }
            }
        }

        if (!ok) {
            failures++;
            if (failures <= 10) {
                printf("FAILURE at test %d: m=%zu n=%zu range=%d\n", t, m, n, range);
            }
        }

        free(A); free(B); free(cFFT); free(cNaive);
    }

    clock_t end = clock();
    double secs = (double)(end - start) / CLOCKS_PER_SEC;

    printf("\n================ TEST SUMMARY ================\n");
    printf("Total test cases : %d\n", NUM_TESTS);
    printf("Failures         : %d\n", failures);
    printf("Max abs error    : %lld (0 = exact integer match)\n", max_abs_err);
    printf("Total time       : %.3f s\n", secs);
    printf("================================================\n");

    return failures == 0 ? 0 : 1;
}

/* =====================================================================================
 *  main
 * ---------------------------------------------------------------------------------------
 *   ./convolution              -> interactive mode: reads YOUR vectors A, B and
 *                                  runs+explains the FFT convolution on them
 *   ./convolution demo         -> fixed built-in example (no input needed)
 *   ./convolution test [N]     -> N randomized correctness tests (default 30000)
 * ===================================================================================== */
int main(int argc, char **argv) {
    if (argc >= 2 && strcmp(argv[1], "test") == 0) {
        int num_tests = 30000;
        if (argc >= 3) num_tests = atoi(argv[2]);
        int rc = run_tests(num_tests);
        return rc;
    }
    if (argc >= 2 && strcmp(argv[1], "demo") == 0) {
        run_demo();
        return 0;
    }
    /* silence unused-function warning for the pedagogical recursive FFT,
     * which is intentionally not on the hot path (see comment above it) */
    (void)fft_recursive;
    run_interactive();
    return 0;
}