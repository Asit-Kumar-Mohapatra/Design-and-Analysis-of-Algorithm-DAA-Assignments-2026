/*
 * Collatz Conjecture Analyser  (modular, single-file)
 * ---------------------------------------------------
 * T(n) = n/2 (n even), 3n+1 (n odd).  Iterate until n == 1.
 *
 * Modes
 *   1. Single value n >= 1 : full trajectory, steps, peak, #even/#odd steps
 *   2. Interval [a,b]      : longest chain, highest peak, average steps, ...
 *
 * Usage
 *   ./collatz              interactive menu
 *   ./collatz n            analyse one value
 *   ./collatz a b          analyse the interval [a,b]
 *
 * Design notes
 *   - uint64_t everywhere; 3n+1 is overflow-checked BEFORE it is computed
 *     (n > (UINT64_MAX-1)/3), so no undefined/wrapped behaviour ever occurs.
 *   - Interval mode: memoisation table (steps + peak) for values <= CACHE_MAX
 *     and <= b. Every uncached prefix of a walk is back-filled, so each
 *     number in the table is computed once -> ~O(1) amortised per n.
 *     Consecutive even steps are collapsed with count-trailing-zeros.
 *   - Space: O(min(b, CACHE_MAX)) for the table + O(longest walk) for buffers.
 *   - Dynamic arrays (malloc/realloc, geometric growth) are used for the
 *     trajectory and the back-fill stack; all memory is freed.
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Configuration                                                       */
/* ------------------------------------------------------------------ */
#ifndef CACHE_MAX
#define CACHE_MAX ((uint64_t)1 << 24)          /* 16M entries * 12 B = 192 MB max */
#endif
#define OVERFLOW_LIMIT ((UINT64_MAX - 1) / 3)  /* n > this  =>  3n+1 overflows   */

/* ------------------------------------------------------------------ */
/* Small helpers                                                       */
/* ------------------------------------------------------------------ */
static inline int ctz64(uint64_t x) {          /* x != 0 */
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_ctzll(x);
#else
    int c = 0;
    while (!(x & 1)) { x >>= 1; c++; }
    return c;
#endif
}

static inline uint64_t max_u64(uint64_t a, uint64_t b) { return a > b ? a : b; }

/* Strict parse of an unsigned 64-bit decimal. Rejects '-', junk, overflow. */
static bool parse_u64(const char *s, uint64_t *out) {
    while (*s == ' ' || *s == '\t') s++;
    if (*s == '\0' || *s == '\n' || *s == '\r' || *s == '-' || *s == '+') return false;
    errno = 0;
    char *end;
    unsigned long long v = strtoull(s, &end, 10);
    if (errno == ERANGE || end == s) return false;
    while (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r') end++;
    if (*end != '\0') return false;
    *out = (uint64_t)v;
    return true;
}

/* Reads one line. An over-long line is discarded completely (so leftovers
 * never leak into the next prompt) and reported as invalid. */
static bool read_u64(const char *prompt, uint64_t *out) {
    char buf[128];
    printf("%s", prompt);
    fflush(stdout);
    if (!fgets(buf, sizeof buf, stdin)) return false;
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] != '\n' && !feof(stdin)) {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) { /* discard */ }
        return false;
    }
    return parse_u64(buf, out);
}

/* ------------------------------------------------------------------ */
/* Dynamic array of uint64_t  (trajectory storage)                     */
/* ------------------------------------------------------------------ */
typedef struct { uint64_t *data; size_t len, cap; } U64Vec;

static bool vec_push(U64Vec *v, uint64_t x) {
    if (v->len == v->cap) {
        size_t nc = v->cap ? v->cap * 2 : 64;
        uint64_t *p = realloc(v->data, nc * sizeof *p);
        if (!p) return false;
        v->data = p; v->cap = nc;
    }
    v->data[v->len++] = x;
    return true;
}
static void vec_free(U64Vec *v) { free(v->data); v->data = NULL; v->len = v->cap = 0; }

/* ------------------------------------------------------------------ */
/* Single-value analysis                                               */
/* ------------------------------------------------------------------ */
typedef struct {
    uint64_t start;
    uint64_t steps;        /* total applications of T until 1            */
    uint64_t even_steps;   /* n/2 steps                                   */
    uint64_t odd_steps;    /* 3n+1 steps                                  */
    uint64_t peak;         /* max value on the trajectory                 */
    uint64_t peak_index;   /* step index at which the peak first occurs   */
    bool     overflow;     /* true if 3n+1 would exceed 64 bits           */
    uint64_t overflow_at;  /* value n whose 3n+1 overflowed               */
} Trajectory;

/* Builds the trajectory into 'path'. Returns false only on out-of-memory. */
static bool collatz_trajectory(uint64_t n, U64Vec *path, Trajectory *r) {
    memset(r, 0, sizeof *r);
    r->start = n; r->peak = n;
    if (!vec_push(path, n)) return false;
    while (n != 1) {
        if (n & 1) {
            if (n > OVERFLOW_LIMIT) { r->overflow = true; r->overflow_at = n; break; }
            n = 3 * n + 1;
            r->odd_steps++;
        } else {
            n >>= 1;
            r->even_steps++;
        }
        r->steps++;
        if (n > r->peak) { r->peak = n; r->peak_index = r->steps; }
        if (!vec_push(path, n)) return false;
    }
    return true;
}

static void print_trajectory_report(const Trajectory *r, const U64Vec *path) {
    printf("\n=== Trajectory of %llu ===\n", (unsigned long long)r->start);
    for (size_t i = 0; i < path->len; i++) {
        printf("%llu%s", (unsigned long long)path->data[i],
               i + 1 < path->len ? " -> " : "\n");
        if ((i + 1) % 8 == 0 && i + 1 < path->len) printf("\n");
    }
    if (r->overflow)
        printf("\n[!] 64-bit overflow: 3*%llu+1 does not fit in uint64_t. "
               "Trajectory aborted (NOT a counter-example).\n",
               (unsigned long long)r->overflow_at);
    printf("\nSteps (total)     : %llu\n", (unsigned long long)r->steps);
    printf("  even steps (n/2): %llu\n", (unsigned long long)r->even_steps);
    printf("  odd steps (3n+1): %llu\n", (unsigned long long)r->odd_steps);
    printf("Peak value        : %llu (first reached at step %llu)\n",
           (unsigned long long)r->peak, (unsigned long long)r->peak_index);
    printf("Length (terms)    : %zu\n", path->len);
    printf("Reached 1         : %s\n", r->overflow ? "unknown (overflow)" : "yes");
}

static void analyse_single(uint64_t n) {
    U64Vec path = {0};
    Trajectory r;
    if (!collatz_trajectory(n, &path, &r)) {
        fprintf(stderr, "Error: out of memory.\n");
        vec_free(&path);
        return;
    }
    print_trajectory_report(&r, &path);
    vec_free(&path);
}

/* ------------------------------------------------------------------ */
/* Memoised engine for intervals                                       */
/* ------------------------------------------------------------------ */
typedef struct { uint64_t value; uint64_t t; uint64_t segmax; } Frame;

typedef struct {
    uint64_t limit;     /* table covers values 1..limit                  */
    uint32_t *steps;    /* steps+1 (0 == unknown)                        */
    uint64_t *peak;
    Frame    *stack;    /* reusable back-fill stack                      */
    size_t    scap;
} Memo;

static bool memo_init(Memo *m, uint64_t b) {
    memset(m, 0, sizeof *m);
    uint64_t lim = b < CACHE_MAX ? b : CACHE_MAX;
    while (lim >= 1) {                       /* degrade gracefully if RAM is short */
        m->steps = calloc((size_t)lim + 1, sizeof *m->steps);
        m->peak  = calloc((size_t)lim + 1, sizeof *m->peak);
        if (m->steps && m->peak) break;
        free(m->steps); free(m->peak); m->steps = NULL; m->peak = NULL;
        lim >>= 1;
    }
    if (lim < 1) return false;
    m->limit = lim;
    m->steps[1] = 1;                          /* steps(1) = 0  (stored +1) */
    m->peak[1]  = 1;
    m->scap = 256;
    m->stack = malloc(m->scap * sizeof *m->stack);
    return m->stack != NULL;
}

static void memo_free(Memo *m) { free(m->steps); free(m->peak); free(m->stack); }

typedef struct { uint64_t steps, peak; bool overflow; bool oom; } WalkResult;

/* Number of steps and peak for n, filling the table along the way. */
static WalkResult memo_walk(Memo *m, uint64_t n) {
    WalkResult res = {0, n, false, false};
    size_t sp = 0;
    uint64_t cur = n, t = 0, segmax = n, overall = n;

    for (;;) {
        if (cur <= m->limit) {
            if (m->steps[cur]) break;                        /* joined the table */
            if (sp == m->scap) {
                size_t nc = m->scap * 2;
                Frame *p = realloc(m->stack, nc * sizeof *p);
                if (!p) { res.oom = true; return res; }
                m->stack = p; m->scap = nc;
            }
            if (sp) m->stack[sp - 1].segmax = segmax;         /* close previous segment */
            m->stack[sp++] = (Frame){cur, t, cur};
            segmax = cur;
        }
        if (cur & 1) {
            if (cur > OVERFLOW_LIMIT) { res.overflow = true; res.steps = t; res.peak = overall; return res; }
            cur = 3 * cur + 1; t++;                           /* result is even */
            if (cur > overall) overall = cur;
            if (cur > segmax)  segmax  = cur;
        } else {
            int k = ctz64(cur);                               /* collapse k halvings */
            cur >>= k; t += (uint64_t)k;
        }
    }
    if (sp) m->stack[sp - 1].segmax = segmax;

    uint64_t total = t + (m->steps[cur] - 1);
    uint64_t pk = m->peak[cur];
    for (size_t i = sp; i-- > 0;) {                           /* back-fill */
        pk = max_u64(pk, m->stack[i].segmax);
        m->steps[m->stack[i].value] = (uint32_t)(total - m->stack[i].t) + 1;
        m->peak [m->stack[i].value] = pk;
    }
    res.steps = total;
    res.peak  = max_u64(overall, m->peak[cur]);
    return res;
}

/* ------------------------------------------------------------------ */
/* Interval analysis                                                   */
/* ------------------------------------------------------------------ */
typedef struct {
    uint64_t longest_start, longest_steps;
    uint64_t peak_start, peak_value;
    uint64_t overflow_count, first_overflow;
    uint64_t sum_steps;      /* <= 2^64 numbers * ~1e3 steps: cannot wrap in any feasible run */
    uint64_t analysed;
} IntervalStats;

static void analyse_interval(uint64_t a, uint64_t b) {
    Memo m;
    if (!memo_init(&m, b)) { fprintf(stderr, "Error: out of memory.\n"); memo_free(&m); return; }

    IntervalStats s;
    memset(&s, 0, sizeof s);
    s.longest_start = a;
    s.peak_start = a;

    for (uint64_t n = a;; n++) {
        WalkResult w = memo_walk(&m, n);
        if (w.oom) { fprintf(stderr, "Error: out of memory.\n"); break; }
        if (w.overflow) {
            if (!s.overflow_count) s.first_overflow = n;
            s.overflow_count++;
        } else {
            if (s.analysed == s.overflow_count || w.steps > s.longest_steps) {
                s.longest_steps = w.steps; s.longest_start = n;   /* first valid or strictly longer */
            }
            s.sum_steps += w.steps;
        }
        if (w.peak > s.peak_value) { s.peak_value = w.peak; s.peak_start = n; }
        s.analysed++;
        if (n == b) break;               /* safe even when b == UINT64_MAX */
    }

    uint64_t ok = s.analysed - s.overflow_count;
    printf("\n=== Collatz analysis on [%llu, %llu] ===\n",
           (unsigned long long)a, (unsigned long long)b);
    printf("Numbers analysed     : %llu\n", (unsigned long long)s.analysed);
    if (ok) {
        printf("Longest chain        : start %llu, %llu steps\n",
               (unsigned long long)s.longest_start, (unsigned long long)s.longest_steps);
        printf("Average steps        : %.4f\n", (double)s.sum_steps / (double)ok);
    }
    printf("Highest peak         : %llu (reached from start %llu)\n",
           (unsigned long long)s.peak_value, (unsigned long long)s.peak_start);
    printf("All reached 1        : %s\n",
           s.overflow_count ? "no - some trajectories overflowed uint64_t (inconclusive)" : "yes");
    if (s.overflow_count) {
        printf("Overflowed starts    : %llu (first at %llu)\n",
               (unsigned long long)s.overflow_count, (unsigned long long)s.first_overflow);
        printf("Note                 : longest chain / average exclude overflowed starts; "
               "peak is a lower bound for them.\n");
    }
    printf("Memo table size      : %llu entries\n", (unsigned long long)m.limit);
    memo_free(&m);
}

/* ------------------------------------------------------------------ */
/* Front-end                                                           */
/* ------------------------------------------------------------------ */
static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s            (interactive)\n"
                    "       %s n          (n >= 1)\n"
                    "       %s a b        (1 <= a <= b)\n", prog, prog, prog);
}

static int run_single(const char *s) {
    uint64_t n;
    if (!parse_u64(s, &n) || n < 1) {
        fprintf(stderr, "Invalid input: n must be an integer in [1, %llu].\n",
                (unsigned long long)UINT64_MAX);
        return 1;
    }
    analyse_single(n);
    return 0;
}

static int run_interval(const char *sa, const char *sb) {
    uint64_t a, b;
    if (!parse_u64(sa, &a) || !parse_u64(sb, &b) || a < 1) {
        fprintf(stderr, "Invalid input: a and b must be integers >= 1.\n");
        return 1;
    }
    if (a > b) { fprintf(stderr, "Invalid interval: a must be <= b.\n"); return 1; }
    analyse_interval(a, b);
    return 0;
}

static void interactive(void) {
    for (;;) {
        printf("\n--- Collatz Analyser ---\n"
               "1) Analyse a single starting value\n"
               "2) Analyse an interval [a, b]\n"
               "3) Exit\n");
        uint64_t c;
        if (!read_u64("Choice: ", &c)) {
            if (feof(stdin)) return;
            printf("Invalid choice.\n"); continue;
        }
        if (c == 3) return;
        if (c == 1) {
            uint64_t n;
            if (!read_u64("Enter n (>= 1): ", &n) || n < 1) { printf("Invalid n.\n"); continue; }
            analyse_single(n);
        } else if (c == 2) {
            uint64_t a, b;
            if (!read_u64("Enter a (>= 1): ", &a) || a < 1) { printf("Invalid a.\n"); continue; }
            if (!read_u64("Enter b (>= a): ", &b) || b < a) { printf("Invalid b (need b >= a).\n"); continue; }
            analyse_interval(a, b);
        } else {
            printf("Invalid choice.\n");
        }
    }
}

int main(int argc, char **argv) {
    if (argc == 1) { interactive(); return 0; }
    if (argc == 2) return run_single(argv[1]);
    if (argc == 3) return run_interval(argv[1], argv[2]);
    usage(argv[0]);
    return 1;
}
