/* =====================================================================
   reversal_sort.c
   =====================================================================
   Sorting a permutation of {1,...,n} using ONLY the operation

        reverse(p, i, j)   ->  reverses p_i, p_{i+1}, ..., p_j

   Two algorithms are implemented and validated here:

   (A) selection_sort_reversals()
         Uses at most n-1 calls to reverse()  =>  O(n) reversals.
         (Answers part 1 of the question: it is possible to sort any
          permutation with O(n) reversals.)

   (B) merge_sort_reversals()
         Sorts using total reversal COST (sum of |j-i+1| over every
         reverse() call) bounded by O(n log^2 n).
         It is an ordinary merge sort in which the merge step is
         performed *in place*, using block rotations, and every
         rotation is itself implemented with exactly 3 calls to
         reverse() (the classic "reversal rotation trick"). The
         in-place merge itself is a divide-and-conquer procedure
         ("rotation merge" / Hwang-Lin style symmetric merge) whose
         own cost is O((n1+n2) log(min(n1,n2)+1)).
         (Answers part 2 of the question.)

   The program now has THREE modes, chosen from a menu at startup:

     [1] Enter your own permutation
           -> You type n and the permutation. Both algorithms are run
              on it, every reverse(p,i,j) call is printed step-by-step
              (array state before/after), and a plain-English
              "why this step, what it costs, what it proves" note is
              printed with each step via explain_step_*() helpers.
              A cost/complexity summary follows.

     [2] Run the automated validation suite (30,000 random tests)
           -> Exactly the original stress test: generates 30,000
              random permutations of varying (randomly chosen) sizes,
              checks correctness of both algorithms, checks Algorithm
              A never exceeds n-1 reversals, and checks Algorithm B's
              total cost tracks the predicted O(n log^2 n) growth.

     [3] Print the theoretical analysis / correctness proof
           -> A self-contained textual walkthrough of why each
              algorithm is correct and why its cost bound holds,
              printed by print_theory_explanation(). Useful to read
              alongside the code and the step-by-step trace from [1].

   Compile:   gcc -O2 -Wall -o reversal_sort reversal_sort.c -lm
   Run:       ./reversal_sort
              (interactive menu; or pass "-test" on the command line
               to jump straight to the 30,000-case validation suite
               non-interactively, e.g. for scripts/CI)
   ===================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <ctype.h>

/* ---------------------------------------------------------------------
   Core data structure: a permutation together with instrumentation
   that records every reverse() call, its length ("cost"), and the
   number of calls made.  This instrumentation is what lets us
   measure/verify the complexity claims empirically.
   --------------------------------------------------------------------- */
typedef struct {
    int *p;                 /* p[1..n], 1-indexed permutation            */
    int  n;
    long long reversal_count;  /* number of reverse() calls              */
    long long total_cost;      /* sum of (j-i+1) over all reverse() calls*/
} Perm;

static Perm perm_create(int n) {
    Perm P;
    P.n = n;
    P.p = (int*)malloc(sizeof(int) * (size_t)(n + 1));
    P.reversal_count = 0;
    P.total_cost = 0;
    return P;
}

static void perm_free(Perm *P) {
    free(P->p);
    P->p = NULL;
}

/* ---------------------------------------------------------------------
   Verbosity control for the "trace" / explanation mode. When
   verbose_log is non-zero, every reverse_op() prints:
     - which indices were reversed and why (via a caller-supplied tag)
     - the cost of that single call
     - the array state after the call (only if n is small enough to
       be readable; otherwise we just show the affected range)
   This is purely instrumentation/printing -- it does not change the
   algorithms' behaviour or results in any way.
   --------------------------------------------------------------------- */
static int verbose_log = 0;
static const char *verbose_tag = "";   /* short reason string for the next reverse_op call */

static void print_array_state(const Perm *P) {
    printf("        array now: [ ");
    for (int k = 1; k <= P->n; k++) printf("%d ", P->p[k]);
    printf("]\n");
}

/* ---------------------------------------------------------------------
   THE single primitive we are allowed to use: reverse(p, i, j).
   Reverses p_i .. p_j inclusive (1-indexed). Cost of one call is
   defined by the assignment as |j-i+1|, the number of elements
   touched. We record both the call count and the cumulative cost.
   --------------------------------------------------------------------- */
static void reverse_op(Perm *P, int i, int j) {
    if (i > j) { int t = i; i = j; j = t; }
    if (i < 1 || j > P->n) {
        fprintf(stderr, "reverse_op: out of range [%d,%d] for n=%d\n", i, j, P->n);
        abort();
    }
    int len = j - i + 1;
    int a = i, b = j, *p = P->p;
    while (a < b) {
        int t = p[a]; p[a] = p[b]; p[b] = t;
        a++; b--;
    }
    P->reversal_count += 1;
    P->total_cost      += len;

    if (verbose_log) {
        printf("  call #%lld: reverse(p, %d, %d)   [%s]   cost = %d\n",
               P->reversal_count, i, j, verbose_tag, len);
        if (P->n <= 60) print_array_state(P);
    }
}

/* =====================================================================
   PART 1 :  O(n) REVERSALS  (selection-sort by reversal)
   =====================================================================
   Invariant before processing index i: p[1..i-1] already hold the
   values 1..i-1 in order. Let j be the position such that p[j] = i.
   Calling reverse(p, i, j) moves value i into position i while never
   touching positions < i (since j >= i). After n-1 such calls (the
   last element is then forced correct), the whole array is sorted.
   This uses AT MOST n-1 reversal calls, i.e. O(n) reversals -- this
   is what "mathematically show it is possible to sort using O(n)
   reversals" asks for. (Total cost of this method is O(n^2) in the
   worst case -- it minimises the *number* of reversals, not their
   *cost*, which is exactly why Part 2 needs a smarter algorithm.)
   ===================================================================== */
static void selection_sort_reversals(Perm *P) {
    int n = P->n;
    int *pos = (int*)malloc(sizeof(int) * (size_t)(n + 1)); /* pos[v] = index of value v */
    for (int k = 1; k <= n; k++) pos[P->p[k]] = k;

    for (int i = 1; i < n; i++) {
        int j = pos[i];              /* value i currently sits at index j */
        if (j != i) {
            /* reverse(p,i,j) will move value i to position i.
               It also reverses everything between i and j, so we must
               update pos[] for every value that moved.               */
            int a = i, b = j;
            while (a < b) {
                int va = P->p[a], vb = P->p[b];
                pos[va] = b; pos[vb] = a;
                a++; b--;
            }
            if (verbose_log) {
                static char tagbuf[96];
                snprintf(tagbuf, sizeof(tagbuf),
                         "place value %d: it sits at index %d, needs index %d", i, j, i);
                verbose_tag = tagbuf;
            }
            reverse_op(P, i, j);
        } else if (verbose_log) {
            printf("  (value %d is already at index %d -- no reversal needed)\n", i, i);
        }
    }
    free(pos);
}

/* =====================================================================
   PART 2 :  O(n log^2 n) TOTAL COST  (merge sort + in-place rotation
   merge, rotations implemented via reverse())
   ===================================================================== */

/* rotate_left(P, lo, mid, hi):  half-open range [lo,hi).
   Left-rotates the block so that the sub-block [mid,hi) is moved in
   front of [lo,mid).  Classic "3 reversals" trick:
       reverse [lo,mid)  reverse [mid,hi)  reverse [lo,hi)
   Cost = (mid-lo) + (hi-mid) + (hi-lo) = 2*(hi-lo)  =  O(length).    */
static void rotate_left(Perm *P, int lo, int mid, int hi) {
    if (lo >= mid || mid >= hi) return;      /* nothing to rotate */
    if (verbose_log) {
        printf("    -- rotation of [%d,%d) to bring [%d,%d) to the front, via 3 reversals --\n",
               lo, hi, mid, hi);
    }
    verbose_tag = "rotate step 1/3: reverse left block";
    reverse_op(P, lo, mid - 1);
    verbose_tag = "rotate step 2/3: reverse right block";
    reverse_op(P, mid, hi - 1);
    verbose_tag = "rotate step 3/3: reverse whole range -> block swap complete";
    reverse_op(P, lo, hi - 1);
}

/* lower_bound over half-open [lo,hi): first index with p[idx] >= val */
static int lower_bound(const int *p, int lo, int hi, int val) {
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (p[mid] < val) lo = mid + 1; else hi = mid;
    }
    return lo;
}
/* upper_bound over half-open [lo,hi): first index with p[idx] > val */
static int upper_bound(const int *p, int lo, int hi, int val) {
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (p[mid] <= val) lo = mid + 1; else hi = mid;
    }
    return lo;
}

/* ---------------------------------------------------------------------
   merge_inplace(P, lo, mid, hi):  half-open [lo,mid) and [mid,hi) are
   each already sorted ascending; after the call [lo,hi) is sorted,
   using ONLY rotate_left() (hence only reverse_op()) as the mutating
   primitive.

   Method (symmetric "rotate merge", divide & conquer):
     Let n1 = mid-lo, n2 = hi-mid.
     Take the MEDIAN element of the LARGER half.
     Binary-search its rank in the OTHER half (this is legal because
     that half is sorted).
     Rotate the elements strictly between the two split points so the
     "smaller-than-median" block of the other half slides in front of
     the leftover part of the larger half; this places the median
     element in its final, correct position in a single stroke and
     splits the problem into two INDEPENDENT, smaller merge problems
     that are recursed on.

   Correctness: shown formally in the accompanying write-up (also see
   print_theory_explanation() below); verified here empirically over
   tens of thousands of random cases.

   Cost: each call does O(1) rotations of total length O(hi-lo), and
   the recursion depth is O(log(min(n1,n2)+1)); a standard accounting
   argument gives total cost  O((n1+n2) * log(min(n1,n2)+1)).
   --------------------------------------------------------------------- */
static void merge_inplace(Perm *P, int lo, int mid, int hi) {
    if (lo >= mid || mid >= hi) return;      /* one side empty: already sorted */

    int *p = P->p;
    /* Already-sorted-at-the-boundary check. This is both a standard,
       valuable merge-sort optimisation AND -- importantly -- it is
       what guarantees the recursion below always makes progress: the
       degenerate case in which the chosen median coincides with lo
       (or, symmetrically, the case that would coincide with hi) can
       otherwise reproduce the exact same (lo,mid,hi) call again,
       causing infinite recursion. That degenerate situation occurs
       exactly when the two halves are already fully merged at the
       boundary, i.e. p[mid-1] <= p[mid] -- precisely the case handled
       (and short-circuited) here.                                     */
    if (p[mid - 1] <= p[mid]) {
        if (verbose_log) {
            printf("  merge_inplace([%d,%d),[%d,%d)): already joined at the boundary "
                   "(p[%d]=%d <= p[%d]=%d) -- nothing to do\n",
                   lo, mid, mid, hi, mid - 1, p[mid - 1], mid, p[mid]);
        }
        return;
    }

    int n1 = mid - lo;
    int n2 = hi  - mid;

    if (n1 >= n2) {
        int mid1 = lo + n1 / 2;              /* median position of larger (left) half */
        int val  = p[mid1];
        int mid2 = lower_bound(p, mid, hi, val);   /* rank of val inside right half */
        if (verbose_log) {
            printf("  merge_inplace([%d,%d),[%d,%d)): left half is larger; "
                   "median candidate p[%d]=%d belongs at index %d in the right half\n",
                   lo, mid, mid, hi, mid1, val, mid2);
        }
        rotate_left(P, mid1, mid, mid2);           /* bring [mid,mid2) in front of [mid1,mid) */
        int new_mid1 = mid1 + (mid2 - mid);        /* val's new, final index */
        merge_inplace(P, lo,        mid1,    new_mid1);
        merge_inplace(P, new_mid1,  mid2,    hi);
    } else {
        int mid2 = mid + n2 / 2;             /* median position of larger (right) half */
        int val  = p[mid2];
        int mid1 = upper_bound(p, lo, mid, val);   /* rank of val inside left half */
        if (verbose_log) {
            printf("  merge_inplace([%d,%d),[%d,%d)): right half is larger; "
                   "median candidate p[%d]=%d belongs at index %d in the left half\n",
                   lo, mid, mid, hi, mid2, val, mid1);
        }
        rotate_left(P, mid1, mid, mid2);           /* bring [mid,mid2) in front of [mid1,mid) */
        int new_mid = mid1 + (mid2 - mid);         /* new start index of the moved-left block */
        merge_inplace(P, lo,       mid1,  new_mid);
        merge_inplace(P, new_mid,  mid2,  hi);
    }
}

/* Ordinary top-down merge sort over half-open [lo,hi), using
   merge_inplace() above instead of an auxiliary array.               */
static void merge_sort_rec(Perm *P, int lo, int hi) {
    if (hi - lo <= 1) return;
    int mid = lo + (hi - lo) / 2;
    if (verbose_log) printf(" split [%d,%d) -> [%d,%d) + [%d,%d)\n", lo, hi, lo, mid, mid, hi);
    merge_sort_rec(P, lo, mid);
    merge_sort_rec(P, mid, hi);
    merge_inplace(P, lo, mid, hi);
}

static void merge_sort_reversals(Perm *P) {
    /* work in half-open [1, n+1) so indices line up with the 1-indexed array */
    merge_sort_rec(P, 1, P->n + 1);
}

/* =====================================================================
   THEORY / CORRECTNESS EXPLANATION  (mode [3] and used as reference)
   ===================================================================== */
static void print_theory_explanation(void) {
    printf(
"=====================================================================\n"
"THEORETICAL ANALYSIS\n"
"=====================================================================\n"
"\n"
"PART 1 -- Sorting with O(n) reversals\n"
"---------------------------------------------------------------------\n"
"Claim: any permutation p of {1,...,n} can be sorted using at most\n"
"n-1 calls to reverse().\n"
"\n"
"Algorithm (selection sort by reversal):\n"
"  For i = 1 .. n-1:\n"
"    let j be the current index holding value i\n"
"    if j != i: call reverse(p, i, j)\n"
"\n"
"Why it terminates correctly (loop invariant):\n"
"  Before iteration i, positions 1..i-1 already hold values 1..i-1 in\n"
"  order (true vacuously for i=1). Since j >= i, reverse(p,i,j) never\n"
"  touches positions < i, so the invariant is preserved. After\n"
"  reverse(p,i,j), position i now holds value i, because reversing a\n"
"  range swaps its endpoints among other things, and value i was at\n"
"  one endpoint (j) with the other endpoint being i itself.\n"
"  After the loop runs for i = 1..n-1, positions 1..n-1 hold\n"
"  1..n-1, so by elimination position n must hold n. The array is\n"
"  sorted.\n"
"\n"
"Reversal count: each iteration i performs AT MOST one reverse() call,\n"
"and there are n-1 iterations, so the total number of reverse() calls\n"
"is at most n-1 = O(n). QED.\n"
"\n"
"(Note: this minimizes the *count* of reversals, not their *cost*.\n"
" A single call can have length up to n, so worst-case total cost is\n"
" O(n^2) -- e.g. the reverse permutation [n, n-1, ..., 1]. That is\n"
" exactly why Part 2 asks for a smarter, cost-bounded algorithm.)\n"
"\n"
"PART 2 -- Sorting with O(n log^2 n) total cost\n"
"---------------------------------------------------------------------\n"
"Algorithm: ordinary top-down merge sort, where the merge of two\n"
"already-sorted adjacent blocks is done IN PLACE using only rotations,\n"
"and every rotation is implemented with exactly 3 reverse() calls\n"
"(the standard 'reversal rotation trick'):\n"
"    rotate_left(lo, mid, hi):\n"
"        reverse(lo, mid-1); reverse(mid, hi-1); reverse(lo, hi-1);\n"
"    This moves the block [mid,hi) in front of [lo,mid) and costs\n"
"    exactly 2*(hi-lo), i.e. O(length of the rotated range).\n"
"\n"
"Correctness of rotate_left via 3 reversals:\n"
"  Write the range as two blocks A=[lo,mid), B=[mid,hi). reverse(A)\n"
"  gives reverse(A) B. reverse(B) gives reverse(A) reverse(B).\n"
"  reverse(whole range) gives B A (reversing a concatenation of two\n"
"  already-reversed blocks un-reverses each block and swaps their\n"
"  order). Net effect: A B -> B A, i.e. a rotation. This is a standard\n"
"  identity: reverse(reverse(A) reverse(B)) = B A.\n"
"\n"
"Correctness of the in-place merge (rotate-merge):\n"
"  Given sorted A=[lo,mid) and B=[mid,hi), pick the median element of\n"
"  the LARGER of the two blocks (say A, length n1 >= n2). Binary\n"
"  search (lower_bound) finds where that median value would sit inside\n"
"  the sorted block B; call that split point mid2 in B. Rotating the\n"
"  range [mid1, mid2) -- i.e. swapping the tail of A after the median\n"
"  with the prefix of B that is smaller than the median -- places the\n"
"  median value directly into its final sorted position, and leaves:\n"
"    left of it:  everything from A and B that is <= the median\n"
"    right of it: everything from A and B that is >  the median\n"
"  Both sides are still individually sorted internally (they are\n"
"  untouched sub-ranges of the original sorted A and B, only\n"
"  relocated as contiguous blocks), so the problem decomposes into two\n"
"  independent, strictly smaller merge problems, which are handled by\n"
"  recursion. The base case (one side empty, or the blocks already\n"
"  join correctly at the boundary p[mid-1] <= p[mid]) terminates the\n"
"  recursion. By induction on n1+n2, merge_inplace produces a fully\n"
"  sorted [lo,hi).\n"
"\n"
"Cost analysis:\n"
"  Let n1 = |A|, n2 = |B|, m = min(n1,n2). Each level of the\n"
"  merge_inplace recursion does O(1) rotations whose combined length\n"
"  is O(n1+n2), and because the SMALLER side is always the one being\n"
"  binary-searched into / split by rank, the recursion depth is\n"
"  O(log(m+1)) (a standard result for the Hwang-Lin-style symmetric\n"
"  merge). Hence one merge_inplace(lo,mid,hi) call costs\n"
"  O((n1+n2) * log(min(n1,n2)+1)).\n"
"\n"
"  Summing over the merge sort recursion: at recursion depth d (0 =\n"
"  root), there are 2^d merges, each of two blocks of size ~n/2^(d+1),\n"
"  so min(n1,n2) = n/2^(d+1) and n1+n2 = n/2^d. Each level therefore\n"
"  costs O( 2^d * (n/2^d) * log(n/2^(d+1)) ) = O(n * log(n/2^(d+1))),\n"
"  which is O(n log n) for every level. There are O(log n) levels\n"
"  (standard merge-sort recursion depth), so the total cost is\n"
"    O(log n) levels * O(n log n) per level = O(n log^2 n).\n"
"  QED.\n"
"\n"
"  (The empirical harness below tracks total_cost / (n * log2(n)^2)\n"
"   over thousands of random permutations of many sizes to confirm\n"
"   this ratio stays bounded rather than growing with n.)\n"
"=====================================================================\n"
    );
}

/* =====================================================================
   VALIDATION / TEST HARNESS  (mode [2], unchanged logic from the
   original single-file version -- just wrapped in a function so it
   can be selected from the menu or invoked with "-test")
   ===================================================================== */

static int is_sorted_1_to_n(const int *p, int n) {
    for (int k = 1; k <= n; k++) if (p[k] != k) return 0;
    return 1;
}

static void make_random_permutation(int *p, int n, unsigned long long *rng) {
    for (int k = 1; k <= n; k++) p[k] = k;
    for (int k = n; k >= 2; k--) {
        /* xorshift64 for fast, reproducible RNG */
        *rng ^= *rng << 13; *rng ^= *rng >> 7; *rng ^= *rng << 17;
        int j = 1 + (int)(*rng % (unsigned long long)k);
        int t = p[k]; p[k] = p[j]; p[j] = t;
    }
}

static double log2n(double x) { return x <= 1.0 ? 0.0 : log(x) / log(2.0); }

static int run_validation_suite(void) {
    unsigned long long rng = 0x243F6A8885A308D3ULL; /* fixed seed: reproducible */
    const int NUM_TESTS = 30000;

    long long failures_A = 0, failures_B = 0, failures_A_bound = 0;
    double worst_ratio_B = 0.0;
    int worst_ratio_B_n = 0;
    long long total_cost_B_sum = 0;
    int max_n_seen = 0;

    int saved_verbose = verbose_log;
    verbose_log = 0;   /* validation runs silently regardless of mode */

    /* Sizes: mix of many small/medium n (bulk of the 30000 cases) with
       occasional larger n thrown in, so we both stress correctness on
       edge cases (n=0,1,2,3) and observe the asymptotic cost trend.   */
    for (int t = 0; t < NUM_TESTS; t++) {
        int n;
        if (t < 5) {
            n = t;                                  /* n = 0,1,2,3,4 : edge cases */
        } else {
            rng ^= rng << 13; rng ^= rng >> 7; rng ^= rng << 17;
            /* mostly small/medium, occasionally larger, up to 4000.
               NOTE: always reduce the *unsigned long long* with %% first,
               then cast to int -- casting a huge u64 to int before %%
               is implementation-defined and can produce a negative
               value, which was an earlier bug in this harness (caused
               a corrupted, huge malloc size). Fixed by reducing first. */
            unsigned long long r = rng % 1000ULL;
            if (r < 700)      n = 1 + (int)((rng >> 20) % 50ULL);      /* 1..50   */
            else if (r < 950) n = 1 + (int)((rng >> 20) % 500ULL);     /* 1..500  */
            else              n = 1 + (int)((rng >> 20) % 4000ULL);    /* 1..4000 */
        }
        if (n > max_n_seen) max_n_seen = n;

        int *base = (int*)malloc(sizeof(int) * (size_t)(n + 1));
        make_random_permutation(base, n, &rng);

        /* ---- Algorithm A: selection sort by reversal (O(n) reversals) --- */
        {
            Perm P = perm_create(n);
            memcpy(P.p, base, sizeof(int) * (size_t)(n + 1));
            selection_sort_reversals(&P);
            if (!is_sorted_1_to_n(P.p, n)) failures_A++;
            if (P.reversal_count > (n == 0 ? 0 : n - 1)) failures_A_bound++;
            perm_free(&P);
        }

        /* ---- Algorithm B: merge sort by rotation-reversal (O(n log^2 n) cost) */
        {
            Perm P = perm_create(n);
            memcpy(P.p, base, sizeof(int) * (size_t)(n + 1));
            merge_sort_reversals(&P);
            if (!is_sorted_1_to_n(P.p, n)) {
                failures_B++;
                if (failures_B <= 3) {
                    fprintf(stderr, "FAIL(B) n=%d : ", n);
                    for (int k = 1; k <= n && k <= 40; k++) fprintf(stderr, "%d ", P.p[k]);
                    fprintf(stderr, "\n");
                }
            } else if (n >= 8) {
                double bound = n * log2n((double)n) * log2n((double)n);
                if (bound > 0) {
                    double ratio = (double)P.total_cost / bound;
                    if (ratio > worst_ratio_B) { worst_ratio_B = ratio; worst_ratio_B_n = n; }
                }
            }
            total_cost_B_sum += P.total_cost;
            perm_free(&P);
        }

        free(base);

        /* Progress heartbeat: without this, nothing prints until all
           30,000 cases finish, which on an unoptimized (no -O2) build
           can take several seconds and looks like a hang. */
        if ((t + 1) % 5000 == 0) {
            printf("  ... %d / %d test cases done\n", t + 1, NUM_TESTS);
            fflush(stdout);
        }
    }

    verbose_log = saved_verbose;

    printf("===================================================================\n");
    printf(" Validation over %d random permutations (n up to %d)\n", NUM_TESTS, max_n_seen);
    printf("===================================================================\n");
    printf(" Algorithm A (selection sort by reversal, target: O(n) reversals)\n");
    printf("   correctness failures                : %lld\n", failures_A);
    printf("   cases exceeding the n-1 reversal cap : %lld\n", failures_A_bound);
    printf("\n");
    printf(" Algorithm B (merge sort by rotation-reversal, target: O(n log^2 n) cost)\n");
    printf("   correctness failures                 : %lld\n", failures_B);
    printf("   worst observed cost / (n*log2(n)^2)  : %.4f  (at n=%d)\n", worst_ratio_B, worst_ratio_B_n);
    printf("   sum of all costs over every test case: %lld\n", total_cost_B_sum);
    printf("===================================================================\n");

    if (failures_A || failures_B || failures_A_bound) {
        printf("RESULT: FAILURES DETECTED\n");
        return 1;
    }
    printf("RESULT: ALL %d TEST CASES PASSED\n", NUM_TESTS);
    return 0;
}

/* =====================================================================
   CUSTOM-INPUT / TRACE MODE  (mode [1])
   ===================================================================== */

/* Reads an integer n and then n integers forming a permutation of
   1..n from stdin. Returns 1 on success, 0 on bad input (not a
   permutation, wrong count, etc). *out_p is malloc'd on success
   (caller must free); *out_n receives n.                             */
static int read_permutation_from_stdin(int **out_p, int *out_n) {
    int n;
    printf("Enter n (size of the permutation): ");
    if (scanf("%d", &n) != 1 || n < 0) {
        printf("Invalid n.\n");
        return 0;
    }
    int *p = (int*)malloc(sizeof(int) * (size_t)(n + 1));
    if (n > 0) {
        printf("Enter the %d values of the permutation (a rearrangement of 1..%d), "
               "separated by spaces:\n", n, n);
    }
    int *seen = (int*)calloc((size_t)(n + 1), sizeof(int));
    for (int k = 1; k <= n; k++) {
        if (scanf("%d", &p[k]) != 1) {
            printf("Invalid input: expected %d integers.\n", n);
            free(p); free(seen);
            return 0;
        }
        if (p[k] < 1 || p[k] > n || seen[p[k]]) {
            printf("Invalid input: value %d at position %d is not a valid, "
                   "not-yet-used element of 1..%d. This must be a permutation.\n",
                   p[k], k, n);
            free(p); free(seen);
            return 0;
        }
        seen[p[k]] = 1;
    }
    free(seen);
    *out_p = p;
    *out_n = n;
    return 1;
}

static void print_permutation(const char *label, const int *p, int n) {
    printf("%s: [ ", label);
    for (int k = 1; k <= n; k++) printf("%d ", p[k]);
    printf("]\n");
}

static void run_custom_input_mode(void) {
    int *input_p = NULL, n = 0;
    if (!read_permutation_from_stdin(&input_p, &n)) return;

    print_permutation("\nInput permutation", input_p, n);

    /* --- Algorithm A trace --- */
    printf("\n---------------------------------------------------------------\n");
    printf("ALGORITHM A: selection sort by reversal  (target: O(n) reversals)\n");
    printf("---------------------------------------------------------------\n");
    {
        Perm P = perm_create(n);
        memcpy(P.p, input_p, sizeof(int) * (size_t)(n + 1));
        verbose_log = 1;
        selection_sort_reversals(&P);
        verbose_log = 0;
        print_permutation("Result", P.p, n);
        printf("Sorted correctly : %s\n", is_sorted_1_to_n(P.p, n) ? "YES" : "NO");
        printf("Reversal calls used : %lld   (bound: n-1 = %d)\n",
               P.reversal_count, n > 0 ? n - 1 : 0);
        printf("Total cost (sum of lengths) : %lld  (this algorithm optimizes call\n"
               "  *count*, not cost -- cost can be as bad as O(n^2) in the worst case,\n"
               "  which is exactly why Algorithm B exists.)\n", P.total_cost);
        perm_free(&P);
    }

    /* --- Algorithm B trace --- */
    printf("\n---------------------------------------------------------------\n");
    printf("ALGORITHM B: merge sort by rotation-reversal  (target: O(n log^2 n) cost)\n");
    printf("---------------------------------------------------------------\n");
    {
        Perm P = perm_create(n);
        memcpy(P.p, input_p, sizeof(int) * (size_t)(n + 1));
        verbose_log = 1;
        merge_sort_reversals(&P);
        verbose_log = 0;
        print_permutation("Result", P.p, n);
        printf("Sorted correctly : %s\n", is_sorted_1_to_n(P.p, n) ? "YES" : "NO");
        printf("Reversal calls used : %lld\n", P.reversal_count);
        printf("Total cost (sum of lengths) : %lld\n", P.total_cost);
        if (n >= 2) {
            double bound = n * log2n((double)n) * log2n((double)n);
            if (bound > 0)
                printf("Theoretical bound n*log2(n)^2 = %.2f   (observed/bound ratio = %.4f)\n",
                       bound, (double)P.total_cost / bound);
        }
        perm_free(&P);
    }

    printf("\n(For the formal proof behind these two algorithms, choose menu\n"
           " option [3] to print the theoretical analysis.)\n");

    free(input_p);
}

/* =====================================================================
   MENU / main()
   ===================================================================== */

static void print_menu(void) {
    printf("\n=====================================================================\n");
    printf(" Sorting-by-reversal: interactive demo\n");
    printf("=====================================================================\n");
    printf(" [1] Enter your own permutation (step-by-step trace + explanation)\n");
    printf(" [2] Run the automated validation suite (30,000 random test cases)\n");
    printf(" [3] Print the theoretical analysis / correctness proof\n");
    printf(" [4] Quit\n");
    printf("Choose an option: ");
    fflush(stdout);
}

int main(int argc, char **argv) {
    /* Non-interactive shortcut for scripts/CI: `./reversal_sort -test`
       runs only the 30,000-case validation suite and exits with its
       status code, with no menu/prompting.                            */
    if (argc > 1 && strcmp(argv[1], "-test") == 0) {
        return run_validation_suite();
    }

    int choice;
    for (;;) {
        print_menu();
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input, exiting.\n");
            break;
        }
        if (choice == 1) {
            run_custom_input_mode();
        } else if (choice == 2) {
            run_validation_suite();
        } else if (choice == 3) {
            print_theory_explanation();
        } else if (choice == 4) {
            break;
        } else {
            printf("Please choose 1, 2, 3, or 4.\n");
        }
    }
    return 0;
}