/*
 * ============================================================================
 *  Application of Sorting - I  (OPTIMAL: O(n) TIME, O(1) EXTRA SPACE)
 * ----------------------------------------------------------------------------
 *  n pairs (number, color), pre-sorted by number.
 *  Re-sort by color (Red, Blue, Yellow) such that numbers stay sorted
 *  within each color, in O(n) time, O(1) extra space, in place.
 *
 *  This complexity is optimal: you must examine every node at least once
 *  (Omega(n) lower bound), and a linked list lets you re-splice nodes with
 *  6 pointers total, independent of n -- no array/copy is ever needed.
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

typedef enum { RED = 0, BLUE = 1, YELLOW = 2, NUM_COLORS = 3 } Color;

typedef struct Node {
    int          number;
    Color        color;
    struct Node *next;
} Node;

/* Head+tail travel together now -- can't get out of sync like two
 * separate arrays could. */
typedef struct {
    Node *head;
    Node *tail;
} Bucket;

static const char *color_name(Color c) {
    switch (c) {
        case RED:    return "Red";
        case BLUE:   return "Blue";
        case YELLOW: return "Yellow";
        default:     return "Unknown";
    }
}

static bool parse_color(const char *token, Color *out) {
    if (!token) return false;
    if (token[0] == 'R' || token[0] == 'r') { *out = RED;    return true; }
    if (token[0] == 'B' || token[0] == 'b') { *out = BLUE;   return true; }
    if (token[0] == 'Y' || token[0] == 'y') { *out = YELLOW; return true; }
    return false;
}

/*
 * sort_by_color_inplace:
 *   Stably reorders the linked list `*head_ref` by color, IN PLACE.
 *   Time  : O(n)  -- single pass, O(1) work per node.
 *   Space : O(1)  -- one Bucket[3] (6 pointers total), regardless of n.
 */
void sort_by_color_inplace(Node **head_ref) {
    Bucket buckets[NUM_COLORS] = { {NULL, NULL}, {NULL, NULL}, {NULL, NULL} };

    Node *cur = *head_ref;
    while (cur != NULL) {
        Node *next_node = cur->next;   /* save before rewiring cur->next */
        Color c = cur->color;

        cur->next = NULL;              /* detach node */
        if (buckets[c].head == NULL) {
            buckets[c].head = cur;
            buckets[c].tail = cur;
        } else {
            buckets[c].tail->next = cur;  /* O(1) append via tail pointer */
            buckets[c].tail = cur;
        }
        cur = next_node;
    }

    /* Concatenate Red -> Blue -> Yellow (skipping any empty buckets) */
    Node *new_head = NULL, *new_tail = NULL;
    for (int c = 0; c < NUM_COLORS; c++) {
        if (buckets[c].head == NULL) continue;
        if (new_head == NULL) {
            new_head = buckets[c].head;
            new_tail = buckets[c].tail;
        } else {
            new_tail->next = buckets[c].head;
            new_tail = buckets[c].tail;
        }
    }

    *head_ref = new_head;
}

static void print_list(const Node *head) {
    bool first = true;
    for (const Node *n = head; n != NULL; n = n->next) {
        printf("%s(%d, %s)", first ? "" : "  ", n->number, color_name(n->color));
        first = false;
    }
    printf("\n");
}

static bool is_valid_result(const Node *head) {
    int last_color = -1;   /* below RED(0), so the first item always passes */
    int last_number_for_color[NUM_COLORS];
    for (int c = 0; c < NUM_COLORS; c++) last_number_for_color[c] = INT_MIN;

    for (const Node *n = head; n != NULL; n = n->next) {
        if ((int)n->color < last_color) return false;
        last_color = (int)n->color;
        if (n->number < last_number_for_color[n->color]) return false;
        last_number_for_color[n->color] = n->number;
    }
    return true;
}

static void free_list(Node *head) {
    while (head) {
        Node *next = head->next;
        free(head);
        head = next;
    }
}

int main(void) {
    long n;
    printf("Enter number of pairs (n): ");
    int rc = scanf("%ld", &n);
    if (rc != 1) {
        fprintf(stderr, feof(stdin) ? "Unexpected end of input\n" : "Invalid n (not a number)\n");
        return 1;
    }
    if (n <= 0 || n > 100000000L) {
        fprintf(stderr, "Invalid n (must be between 1 and 100,000,000)\n");
        return 1;
    }

    printf("Enter %ld pairs as: <number> <color (R/B/Y)>\n", n);
    printf("(Input must already be sorted by number, as per the problem.)\n");

    Node *head = NULL, *tail = NULL;
    for (long i = 0; i < n; i++) {
        char token[16];
        int  num;
        Color c;
        int r = scanf("%d %15s", &num, token);
        if (r != 2 || !parse_color(token, &c)) {
            if (feof(stdin))
                fprintf(stderr, "Unexpected end of input at pair %ld\n", i + 1);
            else
                fprintf(stderr, "Invalid input at pair %ld (expected: <number> <R|B|Y>)\n", i + 1);
            free_list(head);
            return 1;
        }
        Node *node = (Node *)malloc(sizeof(Node));
        if (!node) { fprintf(stderr, "Memory allocation failed\n"); free_list(head); return 1; }
        node->number = num;
        node->color  = c;
        node->next   = NULL;

        if (head == NULL) { head = node; tail = node; }
        else { tail->next = node; tail = node; }
    }

    printf("\nInput (sorted by number):\n");
    print_list(head);

    sort_by_color_inplace(&head);   /* in-place: same nodes, O(1) extra space */

    printf("\nOutput (sorted by color; numbers stable within each color):\n");
    print_list(head);

    printf("\nValidity check: %s\n",
           is_valid_result(head) ? "PASSED (colors grouped R<B<Y, numbers sorted within each)"
                                  : "FAILED");

    free_list(head);
    return 0;
}