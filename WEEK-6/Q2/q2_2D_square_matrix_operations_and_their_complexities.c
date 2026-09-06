
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

/* ---------------------------------------------------------------------
 * Macro for flattened row-major indexing.
 * --------------------------------------------------------------------- */
#define IDX(i, j, n) ((i) * (n) + (j))
#define EPS 1e-9

/* =====================================================================
 * (i) MATRIX ADDITION            ->  Theta(n^2)
 * ---------------------------------------------------------------------
 * Every one of the n^2 output cells needs exactly one addition, and
 * every input cell must be touched at least once (n^2 numbers can't be
 * produced from a strict subset of the n^2+n^2 inputs), so Omega(n^2)
 * is a matching lower bound.  Hence Theta(n^2), no algorithm can beat
 * this order.  Space: O(n^2) for the output (O(1) extra if done
 * in place, A += B).
 * ===================================================================== */
void mat_add(const double *A, const double *B, double *C, int n) {
    int total = n * n;                 /* single linear pass: best for cache */
    for (int k = 0; k < total; k++)
        C[k] = A[k] + B[k];
}

void mat_sub(const double *A, const double *B, double *C, int n) {
    int total = n * n;
    for (int k = 0; k < total; k++)
        C[k] = A[k] - B[k];
}

/* =====================================================================
 * (ii) MATRIX MULTIPLICATION     ->  O(n^3) with the classic algorithm
 * ---------------------------------------------------------------------
 * Textbook triple loop performs n^3 scalar multiplications and
 * n^2 (n-1) additions => Theta(n^3).
 * The loop order below is i-k-j ("ikj"), NOT the textbook i-j-k. This
 * keeps the inner-most loop scanning both B and C row-wise (stride-1),
 * which is dramatically more cache-friendly than i-j-k (which strides
 * through B by n on every step) - same asymptotic complexity, much
 * better constant factor in practice.
 *
 * NOTE ON THE TRUE ASYMPTOTIC LOWER BOUND:
 * Strassen's algorithm achieves O(n^2.807); the current best known
 * bound (Coppersmith-Winograd family / recent improvements) is close
 * to O(n^2.371). These are NOT used here because (a) they carry large
 * hidden constants and are only worthwhile for very large n
 * (n in the thousands+), and (b) they are numerically less stable.
 * For a general-purpose routine, O(n^3) is the practically optimal
 * choice, so that is what we implement and what we report as the
 * answer to this question.
 * ===================================================================== */
void mat_mult(const double *A, const double *B, double *C, int n) {
    memset(C, 0, sizeof(double) * n * n);
    for (int i = 0; i < n; i++) {
        for (int k = 0; k < n; k++) {
            double a_ik = A[IDX(i, k, n)];
            if (a_ik == 0.0) continue;         /* cheap sparsity skip */
            const double *brow = &B[IDX(k, 0, n)];
            double *crow = &C[IDX(i, 0, n)];
            for (int j = 0; j < n; j++)
                crow[j] += a_ik * brow[j];
        }
    }
}

/* Independent reference implementation (standard i-j-k, explicit dot
 * product) used ONLY by the test harness to cross-validate mat_mult
 * with a differently-ordered, independently written computation. */
static void mat_mult_reference(const double *A, const double *B, double *C, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++)
                sum += A[IDX(i, k, n)] * B[IDX(k, j, n)];
            C[IDX(i, j, n)] = sum;
        }
    }
}

/* =====================================================================
 * (iii) IS ZERO MATRIX           ->  O(n^2) worst case, Omega(1) best
 * ---------------------------------------------------------------------
 * Must examine an entry before knowing it is 0, so in the worst case
 * (matrix genuinely all-zero, or the single non-zero entry is last)
 * all n^2 entries are visited: O(n^2). Early-exit on the first
 * non-zero entry found gives a best case of Omega(1). We report the
 * worst-case bound O(n^2) as requested.
 * ===================================================================== */
int is_zero_matrix(const double *A, int n, double eps) {
    int total = n * n;
    for (int k = 0; k < total; k++)
        if (fabs(A[k]) > eps)
            return 0;                  /* early exit: best case O(1) */
    return 1;
}

/* =====================================================================
 * (iv) IS SYMMETRIC MATRIX       ->  O(n^2) worst case
 * ---------------------------------------------------------------------
 * A is symmetric iff A[i][j] == A[j][i] for all i<j. Only the
 * n(n-1)/2 entries strictly above the diagonal need to be compared
 * against their mirror below the diagonal, which is still Theta(n^2)
 * asymptotically (constant factor 1/2 only). Early exit on first
 * mismatch gives best case O(1).
 * ===================================================================== */
int is_symmetric(const double *A, int n, double eps) {
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (fabs(A[IDX(i, j, n)] - A[IDX(j, i, n)]) > eps)
                return 0;
    return 1;
}

/* =====================================================================
 * (v) DETERMINANT                ->  O(n^3) via LU (Gaussian elimination
 *                                     with partial pivoting)
 * ---------------------------------------------------------------------
 * Naive cofactor / Laplace expansion is O(n!) - catastrophically slow.
 * Gaussian elimination reduces A to an upper-triangular matrix U in
 * O(n^3) time; det(A) = (-1)^(#row swaps) * product(diagonal of U).
 * Partial pivoting (choosing the largest-magnitude pivot in each
 * column) is used for numerical stability, not for asymptotic
 * complexity - it does not change the O(n^3) bound.
 * O(n^3) is optimal for general dense determinant computation using
 * elementary row operations, and is the standard practical answer.
 * (A sub-cubic determinant algorithm exists in theory, matching the
 * best matrix-multiplication exponent, but is never used in practice
 * for the reasons given in part (ii).)
 * ===================================================================== */
double determinant_lu(const double *A_in, int n) {
    double *A = (double *)malloc(sizeof(double) * n * n);
    memcpy(A, A_in, sizeof(double) * n * n);

    double det = 1.0;
    for (int col = 0; col < n; col++) {
        /* partial pivot: find largest entry in this column at/below row=col */
        int pivot = col;
        double best = fabs(A[IDX(col, col, n)]);
        for (int r = col + 1; r < n; r++) {
            double v = fabs(A[IDX(r, col, n)]);
            if (v > best) { best = v; pivot = r; }
        }
        if (best < EPS) { free(A); return 0.0; }   /* singular matrix */

        if (pivot != col) {
            for (int j = 0; j < n; j++) {
                double tmp = A[IDX(col, j, n)];
                A[IDX(col, j, n)] = A[IDX(pivot, j, n)];
                A[IDX(pivot, j, n)] = tmp;
            }
            det = -det;                              /* row swap flips sign */
        }

        det *= A[IDX(col, col, n)];
        double inv_pivot = 1.0 / A[IDX(col, col, n)];
        for (int r = col + 1; r < n; r++) {
            double factor = A[IDX(r, col, n)] * inv_pivot;
            if (factor == 0.0) continue;
            for (int j = col; j < n; j++)
                A[IDX(r, j, n)] -= factor * A[IDX(col, j, n)];
        }
    }
    free(A);
    return det;
}

/* Independent O(n!) cofactor-expansion reference, ONLY used by the
 * test harness for small n (<=6) to cross-validate determinant_lu. */
static double determinant_cofactor(const double *A, int n) {
    if (n == 1) return A[0];
    if (n == 2) return A[0] * A[3] - A[1] * A[2];

    double *sub = (double *)malloc(sizeof(double) * (n - 1) * (n - 1));
    double det = 0.0;
    double sign = 1.0;
    for (int col = 0; col < n; col++) {
        int sr = 0;
        for (int r = 1; r < n; r++) {
            int sc = 0;
            for (int c = 0; c < n; c++) {
                if (c == col) continue;
                sub[IDX(sr, sc, n - 1)] = A[IDX(r, c, n)];
                sc++;
            }
            sr++;
        }
        det += sign * A[IDX(0, col, n)] * determinant_cofactor(sub, n - 1);
        sign = -sign;
    }
    free(sub);
    return det;
}

/* =====================================================================
 * (vi) TRANSPOSE IN PLACE (IN SITU)  ->  Theta(n^2) time, O(1) extra space
 * ---------------------------------------------------------------------
 * Since the matrix is square, transpose can be done in place by
 * swapping A[i][j] <-> A[j][i] for all i<j (diagonal untouched).
 * Exactly n(n-1)/2 swaps => Theta(n^2) time, and only a single scalar
 * temporary is needed => O(1) auxiliary space. This is optimal: every
 * off-diagonal element must move, so Omega(n^2) is also a lower bound.
 * (Note: an in-place transpose of a *non-square* matrix is a much
 * harder O(n^2) time / O(1) space cycle-following problem - not
 * needed here since the input is guaranteed square.)
 * ===================================================================== */
void transpose_inplace(double *A, int n) {
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++) {
            double tmp = A[IDX(i, j, n)];
            A[IDX(i, j, n)] = A[IDX(j, i, n)];
            A[IDX(j, i, n)] = tmp;
        }
}

/* =====================================================================
 * (vii) EIGENVALUES & EIGENVECTORS
 * ---------------------------------------------------------------------
 * There is NO closed-form (finite-arithmetic) formula for n >= 5,
 * because the characteristic polynomial has degree n and, by the
 * Abel-Ruffini theorem, general polynomials of degree >= 5 have no
 * solution by radicals. So eigenvalues MUST be found iteratively /
 * numerically for general n; there is no "worst case complexity" in
 * the strict algebraic sense - only "iterations to reach precision
 * eps", so the honest complexity is O(k * n^3), k = #iterations
 * (data- and precision-dependent, not a function of n alone).
 *
 * Two standard iterative approaches, both implemented below:
 *
 *  (a) JACOBI EIGENVALUE ALGORITHM (for SYMMETRIC matrices)
 *      Each Jacobi rotation zeroes one off-diagonal pair and costs
 *      O(n) to apply (updates one row+column); one full sweep zeroes
 *      all n(n-1)/2 pairs => O(n^3) per sweep. Converges quadratically,
 *      so ~O(log log(1/eps)) sweeps suffice in practice.
 *      Total practical cost: O(n^3 * sweeps) ~ O(n^3) for fixed eps.
 *      Guaranteed to converge (unlike QR on general matrices) and
 *      produces both eigenvalues AND orthonormal eigenvectors - the
 *      standard "optimal" method taught for the symmetric case.
 *
 *  (b) POWER ITERATION (for GENERAL matrices, dominant eigenpair only)
 *      Each iteration is one matrix-vector product: O(n^2).
 *      Converges linearly at rate |lambda2/lambda1| per iteration, so
 *      total cost O(n^2 * k). Only finds the single largest-magnitude
 *      eigenvalue/eigenvector (extending to all n eigenpairs for a
 *      general non-symmetric matrix requires the full O(n^3)-per-
 *      iteration QR algorithm, which is what LAPACK's dgeev uses).
 * ===================================================================== */

/* (a) Jacobi eigenvalue algorithm — A must be symmetric.
 * Destroys the working copy internally; writes eigenvalues to `eigval`
 * (length n) and eigenvectors as COLUMNS of `eigvec` (n x n, flattened).
 * Returns number of sweeps used. */
int jacobi_eigen(const double *A_in, int n, double *eigval, double *eigvec,
                  double eps, int max_sweeps) {
    double *A = (double *)malloc(sizeof(double) * n * n);
    memcpy(A, A_in, sizeof(double) * n * n);

    /* eigvec starts as the identity matrix */
    memset(eigvec, 0, sizeof(double) * n * n);
    for (int i = 0; i < n; i++) eigvec[IDX(i, i, n)] = 1.0;

    int sweep;
    for (sweep = 0; sweep < max_sweeps; sweep++) {
        double off = 0.0;
        for (int i = 0; i < n; i++)
            for (int j = i + 1; j < n; j++)
                off += A[IDX(i, j, n)] * A[IDX(i, j, n)];
        if (off < eps) break;                       /* converged */

        for (int p = 0; p < n; p++) {
            for (int q = p + 1; q < n; q++) {
                double apq = A[IDX(p, q, n)];
                if (fabs(apq) < 1e-15) continue;

                double app = A[IDX(p, p, n)];
                double aqq = A[IDX(q, q, n)];
                double phi = 0.5 * atan2(2.0 * apq, aqq - app);
                double c = cos(phi), s = sin(phi);

                for (int k = 0; k < n; k++) {
                    double akp = A[IDX(k, p, n)];
                    double akq = A[IDX(k, q, n)];
                    A[IDX(k, p, n)] = c * akp - s * akq;
                    A[IDX(k, q, n)] = s * akp + c * akq;
                }
                for (int k = 0; k < n; k++) {
                    double apk = A[IDX(p, k, n)];
                    double aqk = A[IDX(q, k, n)];
                    A[IDX(p, k, n)] = c * apk - s * aqk;
                    A[IDX(q, k, n)] = s * apk + c * aqk;
                }
                for (int k = 0; k < n; k++) {
                    double vkp = eigvec[IDX(k, p, n)];
                    double vkq = eigvec[IDX(k, q, n)];
                    eigvec[IDX(k, p, n)] = c * vkp - s * vkq;
                    eigvec[IDX(k, q, n)] = s * vkp + c * vkq;
                }
            }
        }
    }
    for (int i = 0; i < n; i++) eigval[i] = A[IDX(i, i, n)];
    free(A);
    return sweep;
}

/* (b) Power iteration — general (non-symmetric OK) matrix, dominant
 * eigenpair. Returns 1 on convergence within max_iter, else 0. */
int power_iteration(const double *A, int n, double *lambda, double *v,
                     double eps, int max_iter) {
    double *tmp = (double *)malloc(sizeof(double) * n);
    for (int i = 0; i < n; i++) v[i] = 1.0 / sqrt((double)n);

    double lambda_old = 0.0;
    for (int it = 0; it < max_iter; it++) {
        for (int i = 0; i < n; i++) {
            double sum = 0.0;
            for (int j = 0; j < n; j++)
                sum += A[IDX(i, j, n)] * v[j];
            tmp[i] = sum;
        }
        double norm = 0.0;
        for (int i = 0; i < n; i++) norm += tmp[i] * tmp[i];
        norm = sqrt(norm);
        if (norm < 1e-14) { free(tmp); *lambda = 0.0; return 0; }
        for (int i = 0; i < n; i++) v[i] = tmp[i] / norm;

        /* Rayleigh quotient v^T A v gives current eigenvalue estimate */
        double num = 0.0;
        for (int i = 0; i < n; i++) {
            double row = 0.0;
            for (int j = 0; j < n; j++) row += A[IDX(i, j, n)] * v[j];
            num += v[i] * row;
        }
        *lambda = num;
        if (fabs(*lambda - lambda_old) < eps) { free(tmp); return 1; }
        lambda_old = *lambda;
    }
    free(tmp);
    return 0;
}

/* =====================================================================
 * PRINT COMPLEXITY SUMMARY / EXPLANATION
 * ===================================================================== */
void print_complexity_table(void) {
    printf(
        "+-----+------------------------------------+----------------+---------------+\n"
        "| No. | Operation                          | Time (worst)   | Extra Space   |\n"
        "+-----+------------------------------------+----------------+---------------+\n"
        "| i   | Matrix Addition                    | Theta(n^2)     | O(n^2)/O(1)*  |\n"
        "| ii  | Matrix Multiplication              | O(n^3)         | O(n^2)        |\n"
        "| iii | Is Zero Matrix                     | O(n^2)         | O(1)          |\n"
        "| iv  | Is Symmetric Matrix                | O(n^2)         | O(1)          |\n"
        "| v   | Determinant (LU/Gauss elim.)       | O(n^3)         | O(n^2)        |\n"
        "| vi  | Transpose In-Place                 | Theta(n^2)     | O(1)          |\n"
        "| vii | Eigenvalues/vectors (Jacobi/Power) | O(k*n^3)/O(k*n^2)** | O(n^2)   |\n"
        "+-----+------------------------------------+----------------+---------------+\n"
        "  *   O(1) if done as A += B in place, O(n^2) if a new matrix C is produced.\n"
        "  **  k = number of iterations to reach the requested precision; no\n"
        "      closed-form / finite exact algorithm exists in general (n>=5) due\n"
        "      to the Abel-Ruffini theorem, so eigen-decomposition is inherently\n"
        "      iterative. Jacobi (symmetric) -> O(n^3) per sweep; Power iteration\n"
        "      (general, dominant pair only) -> O(n^2) per iteration.\n"
    );
}

/* =====================================================================
 * SMALL PRINT HELPER (shared by demo and interactive modes)
 * ===================================================================== */
static void print_matrix(const char *name, const double *A, int n) {
    printf("%s =\n", name);
    for (int i = 0; i < n; i++) {
        printf("  [ ");
        for (int j = 0; j < n; j++) printf("%8.3f ", A[IDX(i, j, n)]);
        printf("]\n");
    }
}

/* =====================================================================
 * INPUT HANDLING
 * ---------------------------------------------------------------------
 * read_int_prompt / read_matrix_from_user: interactively read n and
 * the n*n entries of a square matrix from stdin, with basic input
 * validation (re-prompts on non-numeric or out-of-range input).
 *
 * A second matrix B (same size) is also read, since two of the
 * operations (addition/subtraction) are binary. If the user has no
 * second matrix in mind they can just enter the identity or zeros;
 * this is explained on screen.
 * ===================================================================== */
static int read_int_prompt(const char *prompt, int lo, int hi) {
    char line[256];
    int value;
    while (1) {
        printf("%s", prompt);
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) {
            /* No more input (e.g. piped/EOF) -> fall back to a sane default */
            return lo;
        }
        if (sscanf(line, "%d", &value) == 1 && value >= lo && value <= hi)
            return value;
        printf("  Please enter an integer between %d and %d.\n", lo, hi);
    }
}

static void read_matrix_from_user(const char *label, double *A, int n) {
    char line[256];
    printf("Enter the %d entries of matrix %s, row by row.\n", n * n, label);
    printf("(You may type a whole row on one line, or one number at a time.)\n");
    int filled = 0;
    while (filled < n * n) {
        if (!fgets(line, sizeof(line), stdin)) {
            /* EOF reached early: pad remaining entries with 0 */
            for (; filled < n * n; filled++) A[filled] = 0.0;
            break;
        }
        char *p = line;
        while (*p) {
            char *end;
            double val = strtod(p, &end);
            if (end == p) {           /* no number found on the remainder of this line */
                if (isspace((unsigned char)*p)) { p++; continue; }
                break;
            }
            if (filled < n * n) A[filled++] = val;
            p = end;
        }
    }
}

/* =====================================================================
 * EXPLANATION-DRIVEN OPERATION RUNNER
 * ---------------------------------------------------------------------
 * For each of the 7 required operations, run_and_explain() prints:
 *   1. A short plain-English description of WHAT the algorithm does,
 *   2. WHY that is the worst-case complexity (tight lower bound
 *      argument, in one or two lines),
 *   3. the actual numeric result computed on the user-supplied matrix.
 *
 * This turns the source file itself into a self-contained worked
 * example / answer key for the assignment, driven by whatever matrix
 * the user typed in, instead of only a hard-coded demo matrix.
 * ===================================================================== */
void explain_and_run(const double *A, const double *B, int n) {
    double *C   = malloc(sizeof(double) * n * n);
    double *D   = malloc(sizeof(double) * n * n);
    double *T   = malloc(sizeof(double) * n * n);
    double *Sym = malloc(sizeof(double) * n * n);
    double *eigval = malloc(sizeof(double) * n);
    double *eigvec = malloc(sizeof(double) * n * n);
    double *v      = malloc(sizeof(double) * n);

    printf("\n================ YOUR MATRICES ================\n");
    print_matrix("A", A, n);
    print_matrix("B", B, n);

    /* ---------------- (i) Addition ---------------- */
    printf("\n--- (i) Matrix Addition ---\n");
    printf("What:  C[i][j] = A[i][j] + B[i][j] for every cell.\n");
    printf("Why O(n^2): there are n^2 output cells and each needs exactly\n");
    printf("           one addition; you cannot produce n^2 numbers by\n");
    printf("           touching fewer than n^2 inputs, so this is tight\n");
    printf("           (Theta(n^2), not just an upper bound).\n");
    mat_add(A, B, C, n);
    print_matrix("A + B", C, n);

    /* ---------------- (ii) Multiplication ---------------- */
    printf("\n--- (ii) Matrix Multiplication ---\n");
    printf("What:  C[i][j] = sum_k A[i][k]*B[k][j] (n multiply-adds per cell).\n");
    printf("Why O(n^3): n^2 output cells, each an inner product of length n,\n");
    printf("           so n^2 * n = n^3 scalar multiplications overall.\n");
    printf("           (Sub-cubic algorithms like Strassen exist but are not\n");
    printf("           used here - large constants / worse numerical\n");
    printf("           stability, not worthwhile at typical problem sizes.)\n");
    mat_mult(A, B, C, n);
    print_matrix("A * B", C, n);

    /* ---------------- (iii) Zero check ---------------- */
    printf("\n--- (iii) Is Zero Matrix? ---\n");
    printf("What:  scan every entry; if any |entry| > eps, it's not zero.\n");
    printf("Why O(n^2) worst case: a single non-zero value could be the\n");
    printf("           very last cell examined, so all n^2 cells may need\n");
    printf("           checking. (Best case is O(1) via early exit.)\n");
    int zA = is_zero_matrix(A, n, EPS);
    printf("is_zero_matrix(A) = %d (%s)\n", zA, zA ? "yes, A is the zero matrix" : "no");

    /* ---------------- (iv) Symmetry check ---------------- */
    printf("\n--- (iv) Is Symmetric Matrix? ---\n");
    printf("What:  compare A[i][j] with A[j][i] for every i<j.\n");
    printf("Why O(n^2) worst case: there are n(n-1)/2 such pairs to check\n");
    printf("           (still Theta(n^2) up to a constant factor of 1/2);\n");
    printf("           a mismatch might only show up in the very last pair.\n");
    int symA = is_symmetric(A, n, EPS);
    printf("is_symmetric(A) = %d (%s)\n", symA, symA ? "yes" : "no");

    /* ---------------- (v) Determinant ---------------- */
    printf("\n--- (v) Determinant (Gaussian elimination with pivoting) ---\n");
    printf("What:  reduce A to upper-triangular form U via row operations,\n");
    printf("       then det(A) = (-1)^(#row swaps) * product of U's diagonal.\n");
    printf("Why O(n^3): eliminating below the pivot in column c costs\n");
    printf("           O((n-c)^2); summing c=0..n-1 gives O(n^3) total.\n");
    printf("           (Naive cofactor expansion is O(n!) - used only in the\n");
    printf("           test harness, for cross-checking on tiny matrices.)\n");
    double det = determinant_lu(A, n);
    printf("det(A) = %.6f\n", det);

    /* ---------------- (vi) Transpose in place ---------------- */
    printf("\n--- (vi) Transpose In Place ---\n");
    printf("What:  swap A[i][j] <-> A[j][i] for every i<j; diagonal untouched.\n");
    printf("Why Theta(n^2): exactly n(n-1)/2 swaps are needed and every\n");
    printf("           off-diagonal entry must move, so this is a matching\n");
    printf("           lower bound too (not just an upper bound). Only one\n");
    printf("           scalar temporary is used, so O(1) extra space.\n");
    memcpy(T, A, sizeof(double) * n * n);
    transpose_inplace(T, n);
    print_matrix("transpose(A)", T, n);

    /* ---------------- (vii) Eigenvalues/eigenvectors ---------------- */
    printf("\n--- (vii) Eigenvalues & Eigenvectors ---\n");
    printf("What:  no closed-form solution exists for n>=5 (Abel-Ruffini),\n");
    printf("       so we use iterative numerical methods instead:\n");
    printf("         (a) Jacobi rotations (needs a SYMMETRIC matrix) ->\n");
    printf("             full eigenvalues + orthonormal eigenvectors.\n");
    printf("         (b) Power iteration (works on ANY matrix) -> only the\n");
    printf("             single dominant eigenpair.\n");
    printf("Why O(k*n^3) / O(k*n^2): Jacobi does O(n^3) work per sweep and\n");
    printf("           needs k sweeps to converge; power iteration does\n");
    printf("           O(n^2) work (one matrix-vector product) per\n");
    printf("           iteration and needs k iterations to converge; k\n");
    printf("           depends on the data and requested precision, not on\n");
    printf("           n alone, which is why there is no fixed worst-case\n");
    printf("           bound purely in terms of n.\n");

    memcpy(Sym, A, sizeof(double) * n * n);
    /* force symmetry for the Jacobi run, since it requires it */
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            Sym[IDX(j, i, n)] = Sym[IDX(i, j, n)];
    int sweeps = jacobi_eigen(Sym, n, eigval, eigvec, 1e-12, 200);
    printf("(Using a symmetrized copy of A for Jacobi, since Jacobi requires\n");
    printf(" a symmetric input; A was made symmetric by mirroring the upper\n");
    printf(" triangle onto the lower triangle for this step only.)\n");
    printf("Jacobi eigenvalues (after %d sweeps): ", sweeps);
    for (int i = 0; i < n; i++) printf("%.6f  ", eigval[i]);
    printf("\n");

    double lambda;
    int converged = power_iteration(A, n, &lambda, v, 1e-10, 2000);
    printf("Power-iteration dominant eigenvalue of A: %.6f (%s), eigenvector: [ ",
           lambda, converged ? "converged" : "did not fully converge");
    for (int i = 0; i < n; i++) printf("%.4f ", v[i]);
    printf("]\n");

    printf("\n================ COMPLEXITY SUMMARY ================\n");
    print_complexity_table();

    free(C); free(D); free(T); free(Sym); free(eigval); free(eigvec); free(v);
}

/* =====================================================================
 * INTERACTIVE MODE: ask the user for n and two matrices, then run
 * every operation with an explanation via explain_and_run().
 * ===================================================================== */
static void run_interactive(void) {
    printf("=== Square Matrix Operations - Interactive Mode ===\n");
    printf("This will compute all 7 operations on YOUR matrix and explain\n");
    printf("each one's worst-case time complexity as it goes.\n\n");
    printf("Two matrices are needed because addition/subtraction are binary\n");
    printf("operations; if you only care about A, just enter zeros for B.\n\n");

    int n = read_int_prompt("Enter matrix size n (1-20): ", 1, 20);

    double *A = malloc(sizeof(double) * n * n);
    double *B = malloc(sizeof(double) * n * n);

    read_matrix_from_user("A", A, n);
    read_matrix_from_user("B", B, n);

    explain_and_run(A, B, n);

    free(A);
    free(B);
}

/* =====================================================================
 * FIXED-DATA DEMO (kept for quick sanity checks / grading without
 * needing to type input by hand)
 * ===================================================================== */
static void run_demo(void) {
    int n = 3;
    double A[9] = { 4, 1, 1,
                     1, 4, 1,
                     1, 1, 4 };   /* symmetric, positive-definite */
    double B[9] = { 1, 0, 0,
                     0, 1, 0,
                     0, 0, 1 };
    explain_and_run(A, B, n);
}

/* =====================================================================
 * STRESS-TEST HARNESS: 30000 randomized test cases (default)
 * covering every operation above, each checked against an
 * independent invariant / independent implementation.
 * ===================================================================== */
static double frand(double lo, double hi) {
    return lo + (hi - lo) * ((double)rand() / (double)RAND_MAX);
}

static void fill_random(double *A, int n) {
    for (int k = 0; k < n * n; k++) A[k] = frand(-10.0, 10.0);
}

static void make_symmetric(double *A, int n) {
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            A[IDX(j, i, n)] = A[IDX(i, j, n)];
}

static void make_zero(double *A, int n) {
    memset(A, 0, sizeof(double) * n * n);
}

static int run_tests(int num_tests) {
    srand(42);
    long fails = 0;

    for (int t = 0; t < num_tests; t++) {
        int n = 1 + rand() % 6;              /* n in [1,6], keeps O(n!) ref. feasible */
        int sz = n * n;
        double *A = malloc(sizeof(double) * sz);
        double *B = malloc(sizeof(double) * sz);
        double *C = malloc(sizeof(double) * sz);
        double *Cref = malloc(sizeof(double) * sz);
        double *I = malloc(sizeof(double) * sz);
        double *back = malloc(sizeof(double) * sz);
        double *At = malloc(sizeof(double) * sz);
        double *T = malloc(sizeof(double) * sz);
        double *Asym = malloc(sizeof(double) * sz);
        double *eigval = malloc(sizeof(double) * n);
        double *eigvec = malloc(sizeof(double) * sz);

        fill_random(A, n);
        fill_random(B, n);

        /* occasionally force special structure to exercise edge cases */
        int roll = rand() % 10;
        if (roll == 0) make_zero(A, n);
        else if (roll == 1) make_symmetric(A, n);

        /* ---- (i) addition: (A+B)-B==A and (A+B)-A==B ---- */
        mat_add(A, B, C, n);
        mat_sub(C, B, back, n);
        for (int k = 0; k < sz; k++)
            if (fabs(back[k] - A[k]) > 1e-6) { fails++; goto next; }
        mat_sub(C, A, back, n);
        for (int k = 0; k < sz; k++)
            if (fabs(back[k] - B[k]) > 1e-6) { fails++; goto next; }

        /* ---- (ii) multiplication: cross-check vs independent ref,
                 and identity property A*I == A ---- */
        mat_mult(A, B, C, n);
        mat_mult_reference(A, B, Cref, n);
        for (int k = 0; k < sz; k++)
            if (fabs(C[k] - Cref[k]) > 1e-6) { fails++; goto next; }

        memset(I, 0, sizeof(double) * sz);
        for (int i = 0; i < n; i++) I[IDX(i, i, n)] = 1.0;
        mat_mult(A, I, C, n);
        for (int k = 0; k < sz; k++)
            if (fabs(C[k] - A[k]) > 1e-6) { fails++; goto next; }

        /* ---- (iii) zero-matrix check ---- */
        {
            int expect_zero = 1;
            for (int k = 0; k < sz; k++) if (fabs(A[k]) > EPS) { expect_zero = 0; break; }
            if (is_zero_matrix(A, n, EPS) != expect_zero) { fails++; goto next; }
        }

        /* ---- (iv) symmetric check ---- */
        {
            int expect_sym = 1;
            for (int i = 0; i < n && expect_sym; i++)
                for (int j = i + 1; j < n; j++)
                    if (fabs(A[IDX(i, j, n)] - A[IDX(j, i, n)]) > EPS) { expect_sym = 0; break; }
            if (is_symmetric(A, n, EPS) != expect_sym) { fails++; goto next; }
        }

        /* ---- (v) determinant: LU result vs O(n!) cofactor reference,
                 and det(A) == det(transpose(A)) ---- */
        {
            double d_lu = determinant_lu(A, n);
            double d_ref = determinant_cofactor(A, n);
            double tol = 1e-4 * (1.0 + fabs(d_ref));
            if (fabs(d_lu - d_ref) > tol) { fails++; goto next; }

            memcpy(At, A, sizeof(double) * sz);
            transpose_inplace(At, n);
            double d_lu_t = determinant_lu(At, n);
            if (fabs(d_lu_t - d_lu) > tol) { fails++; goto next; }
        }

        /* ---- (vi) transpose in place: transpose(transpose(A))==A ---- */
        {
            memcpy(T, A, sizeof(double) * sz);
            transpose_inplace(T, n);
            transpose_inplace(T, n);
            for (int k = 0; k < sz; k++)
                if (fabs(T[k] - A[k]) > 1e-9) { fails++; goto next; }
        }

        /* ---- (vii) eigen: for a *symmetric* version of A, check
                 A_sym * v_i ~= lambda_i * v_i (residual test) ---- */
        {
            memcpy(Asym, A, sizeof(double) * sz);
            make_symmetric(Asym, n);

            jacobi_eigen(Asym, n, eigval, eigvec, 1e-10, 200);

            double maxres = 0.0;
            for (int col = 0; col < n; col++) {
                for (int i = 0; i < n; i++) {
                    double sum = 0.0;
                    for (int j = 0; j < n; j++)
                        sum += Asym[IDX(i, j, n)] * eigvec[IDX(j, col, n)];
                    double resid = fabs(sum - eigval[col] * eigvec[IDX(i, col, n)]);
                    if (resid > maxres) maxres = resid;
                }
            }
            double scale = 1.0;
            for (int k = 0; k < sz; k++) if (fabs(Asym[k]) > scale) scale = fabs(Asym[k]);
            if (maxres > 1e-5 * scale + 1e-6) { fails++; goto next; }
        }

        next:
        free(A); free(B); free(C); free(Cref); free(I); free(back);
        free(At); free(T); free(Asym); free(eigval); free(eigvec);
    }

    printf("Ran %d randomized test cases (n in [1,6]) covering (i)-(vii).\n", num_tests);
    printf("Failures: %ld\n", fails);
    return fails == 0 ? 0 : 1;
}

/* =====================================================================
 * main
 * ---------------------------------------------------------------------
 *   ./matrix_ops              -> interactive mode: reads YOUR matrix
 *                                 and runs+explains every operation
 *   ./matrix_ops demo         -> fixed built-in example (no input needed)
 *   ./matrix_ops test [N]     -> N randomized correctness tests
 * ===================================================================== */
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
    run_interactive();
    return 0;
}
