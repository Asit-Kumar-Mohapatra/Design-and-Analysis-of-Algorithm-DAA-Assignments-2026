#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

// ============================================================================
// DATA STRUCTURE DEFINITIONS
// ============================================================================
typedef struct { int *arr; int size; int capacity; } ArrayDS;

typedef struct SNode { int key; struct SNode *next; } SNode;
typedef struct DNode { int key; struct DNode *prev; struct DNode *next; } DNode;

typedef struct { SNode *head; int size; } SinglyUnsorted;
typedef struct { SNode *head; SNode *tail; int size; } SinglySorted;

typedef struct { DNode *head; int size; } DoublyUnsorted;
typedef struct { DNode *head; DNode *tail; int size; } DoublySorted;

// ============================================================================
// OPERATION IMPLEMENTATIONS (All 42 Operations)
// ============================================================================

// 1. Unsorted Array
int UA_Search(ArrayDS *d, int k) {
    for (int i = 0; i < d->size; i++) if (d->arr[i] == k) return i;
    return -1;
}
void UA_Insert(ArrayDS *d, int x) { d->arr[d->size++] = x; }
void UA_Delete(ArrayDS *d, int idx) { d->arr[idx] = d->arr[--d->size]; }
int UA_Max(ArrayDS *d) {
    int m = 0; for (int i = 1; i < d->size; i++) if (d->arr[i] > d->arr[m]) m = i; return m;
}
int UA_Min(ArrayDS *d) {
    int m = 0; for (int i = 1; i < d->size; i++) if (d->arr[i] < d->arr[m]) m = i; return m;
}
int UA_Predecessor(ArrayDS *d, int idx) {
    int t = d->arr[idx], b = -1, max_v = INT_MIN;
    for (int i = 0; i < d->size; i++) if (d->arr[i] < t && d->arr[i] > max_v) { max_v = d->arr[i]; b = i; }
    return b;
}
int UA_Successor(ArrayDS *d, int idx) {
    int t = d->arr[idx], b = -1, min_v = INT_MAX;
    for (int i = 0; i < d->size; i++) if (d->arr[i] > t && d->arr[i] < min_v) { min_v = d->arr[i]; b = i; }
    return b;
}

// 2. Sorted Array
int SA_Search(ArrayDS *d, int k) {
    int l = 0, r = d->size - 1;
    while (l <= r) {
        int m = l + (r - l) / 2;
        if (d->arr[m] == k) return m;
        if (d->arr[m] < k) l = m + 1; else r = m - 1;
    }
    return -1;
}
void SA_Insert(ArrayDS *d, int x) {
    int i = d->size - 1;
    while (i >= 0 && d->arr[i] > x) { d->arr[i + 1] = d->arr[i]; i--; }
    d->arr[i + 1] = x; d->size++;
}
void SA_Delete(ArrayDS *d, int idx) {
    for (int i = idx; i < d->size - 1; i++) d->arr[i] = d->arr[i + 1];
    d->size--;
}
int SA_Max(ArrayDS *d) { return d->size - 1; }
int SA_Min(ArrayDS *d) { return 0; }
int SA_Predecessor(ArrayDS *d, int idx) { return (idx > 0) ? idx - 1 : -1; }
int SA_Successor(ArrayDS *d, int idx) { return (idx < d->size - 1) ? idx + 1 : -1; }

// 3. Singly Linked Unsorted List
SNode* SU_Search(SinglyUnsorted *d, int k) {
    SNode *c = d->head; while (c && c->key != k) c = c->next; return c;
}
SNode* SU_Insert(SinglyUnsorted *d, int x) {
    SNode *n = (SNode*)malloc(sizeof(SNode)); n->key = x; n->next = d->head; d->head = n; d->size++;
    return n;
}
// Benchmark-only helper: inserts at the TAIL (O(n) traversal), used so we can
// force a genuine worst-case (mid/tail) node position for the Delete benchmark
// instead of always re-inserting at the O(1) head.
SNode* SU_InsertTail(SinglyUnsorted *d, int x) {
    SNode *n = (SNode*)malloc(sizeof(SNode));
    n->key = x; n->next = NULL;
    if (!d->head) { d->head = n; d->size++; return n; }
    SNode *c = d->head;
    while (c->next) c = c->next;   // O(n) walk to the end — intentional
    c->next = n;
    d->size++;
    return n;
}
void SU_Delete(SinglyUnsorted *d, SNode *ptr) {
    if (!d->head || !ptr) return;
    if (d->head == ptr) { d->head = ptr->next; free(ptr); d->size--; return; }
    SNode *c = d->head; while (c->next && c->next != ptr) c = c->next;
    if (c->next == ptr) { c->next = ptr->next; free(ptr); d->size--; }
}
SNode* SU_Max(SinglyUnsorted *d) {
    SNode *m = d->head, *c = d->head;
    while (c) { if (c->key > m->key) m = c; c = c->next; } return m;
}
SNode* SU_Min(SinglyUnsorted *d) {
    SNode *m = d->head, *c = d->head;
    while (c) { if (c->key < m->key) m = c; c = c->next; } return m;
}
SNode* SU_Predecessor(SinglyUnsorted *d, SNode *ptr) {
    SNode *c = d->head, *b = NULL; int max_v = INT_MIN;
    while (c) { if (c->key < ptr->key && c->key > max_v) { max_v = c->key; b = c; } c = c->next; }
    return b;
}
SNode* SU_Successor(SinglyUnsorted *d, SNode *ptr) {
    SNode *c = d->head, *b = NULL; int min_v = INT_MAX;
    while (c) { if (c->key > ptr->key && c->key < min_v) { min_v = c->key; b = c; } c = c->next; }
    return b;
}

// 4. Singly Linked Sorted List
SNode* SS_Search(SinglySorted *d, int k) {
    SNode *c = d->head; while (c && c->key != k) c = c->next; return c;
}
SNode* SS_Insert(SinglySorted *d, int x) {
    SNode *n = (SNode*)malloc(sizeof(SNode)); n->key = x; n->next = NULL;
    if (!d->head || d->head->key >= x) { n->next = d->head; d->head = n; if (!d->tail) d->tail = n; }
    else {
        SNode *c = d->head; while (c->next && c->next->key < x) c = c->next;
        n->next = c->next; c->next = n; if (!n->next) d->tail = n;
    }
    d->size++;
    return n;
}
void SS_Delete(SinglySorted *d, SNode *ptr) {
    if (!d->head || !ptr) return;
    if (d->head == ptr) { d->head = ptr->next; if (d->tail == ptr) d->tail = NULL; free(ptr); d->size--; return; }
    SNode *c = d->head; while (c->next && c->next != ptr) c = c->next;
    if (c->next == ptr) { c->next = ptr->next; if (d->tail == ptr) d->tail = c; free(ptr); d->size--; }
}
SNode* SS_Max(SinglySorted *d) { return d->tail; }
SNode* SS_Min(SinglySorted *d) { return d->head; }
SNode* SS_Predecessor(SinglySorted *d, SNode *ptr) {
    if (d->head == ptr) return NULL; SNode *c = d->head;
    while (c && c->next != ptr) c = c->next; return c;
}
SNode* SS_Successor(SinglySorted *d, SNode *ptr) { return ptr ? ptr->next : NULL; }

// 5. Doubly Linked Unsorted List
DNode* DU_Search(DoublyUnsorted *d, int k) {
    DNode *c = d->head; while (c && c->key != k) c = c->next; return c;
}
DNode* DU_Insert(DoublyUnsorted *d, int x) {
    DNode *n = (DNode*)malloc(sizeof(DNode)); n->key = x; n->prev = NULL; n->next = d->head;
    if (d->head) d->head->prev = n; d->head = n; d->size++;
    return n;
}
void DU_Delete(DoublyUnsorted *d, DNode *ptr) {
    if (!ptr) return;
    if (ptr->prev) ptr->prev->next = ptr->next; else d->head = ptr->next;
    if (ptr->next) ptr->next->prev = ptr->prev; free(ptr); d->size--;
}
DNode* DU_Max(DoublyUnsorted *d) {
    DNode *m = d->head, *c = d->head;
    while (c) { if (c->key > m->key) m = c; c = c->next; } return m;
}
DNode* DU_Min(DoublyUnsorted *d) {
    DNode *m = d->head, *c = d->head;
    while (c) { if (c->key < m->key) m = c; c = c->next; } return m;
}
DNode* DU_Predecessor(DoublyUnsorted *d, DNode *ptr) {
    DNode *c = d->head, *b = NULL; int max_v = INT_MIN;
    while (c) { if (c->key < ptr->key && c->key > max_v) { max_v = c->key; b = c; } c = c->next; }
    return b;
}
DNode* DU_Successor(DoublyUnsorted *d, DNode *ptr) {
    DNode *c = d->head, *b = NULL; int min_v = INT_MAX;
    while (c) { if (c->key > ptr->key && c->key < min_v) { min_v = c->key; b = c; } c = c->next; }
    return b;
}

// 6. Doubly Linked Sorted List
DNode* DS_Search(DoublySorted *d, int k) {
    DNode *c = d->head; while (c && c->key != k) c = c->next; return c;
}
DNode* DS_Insert(DoublySorted *d, int x) {
    DNode *n = (DNode*)malloc(sizeof(DNode)); n->key = x;
    if (!d->head) { n->prev = n->next = NULL; d->head = d->tail = n; }
    else if (d->head->key >= x) { n->prev = NULL; n->next = d->head; d->head->prev = n; d->head = n; }
    else if (d->tail->key <= x) { n->next = NULL; n->prev = d->tail; d->tail->next = n; d->tail = n; }
    else {
        DNode *c = d->head; while (c->next && c->next->key < x) c = c->next;
        n->next = c->next; n->prev = c; c->next->prev = n; c->next = n;
    }
    d->size++;
    return n;
}
void DS_Delete(DoublySorted *d, DNode *ptr) {
    if (!ptr) return;
    if (ptr->prev) ptr->prev->next = ptr->next; else d->head = ptr->next;
    if (ptr->next) ptr->next->prev = ptr->prev; else d->tail = ptr->prev;
    free(ptr); d->size--;
}
DNode* DS_Max(DoublySorted *d) { return d->tail; }
DNode* DS_Min(DoublySorted *d) { return d->head; }
DNode* DS_Predecessor(DoublySorted *d, DNode *ptr) { return ptr ? ptr->prev : NULL; }
DNode* DS_Successor(DoublySorted *d, DNode *ptr) { return ptr ? ptr->next : NULL; }

// ============================================================================
// CONSOLE OUTPUT FUNCTIONS
// ============================================================================
void print_complexities() {
    printf("\n=========================================================================================\n");
    printf("                  ASYMPTOTIC WORST-CASE COMPLEXITIES (TIME & SPACE)                      \n");
    printf("=========================================================================================\n");
    printf("%-20s | %-8s | %-6s | %-6s | %-6s | %-6s | %-6s | %-6s | %-6s\n",
           "Data Structure", "Search", "Insert", "Delete", "Max", "Min", "Pred", "Succ", "Space");
    printf("-----------------------------------------------------------------------------------------\n");
    printf("%-20s | %-8s | %-6s | %-6s | %-6s | %-6s | %-6s | %-6s | %-6s\n", "Unsorted Array", "O(n)", "O(1)", "O(1)", "O(n)", "O(n)", "O(n)", "O(n)", "O(n)");
    printf("%-20s | %-8s | %-6s | %-6s | %-6s | %-6s | %-6s | %-6s | %-6s\n", "Sorted Array", "O(log n)", "O(n)", "O(n)", "O(1)", "O(1)", "O(1)", "O(1)", "O(n)");
    printf("%-20s | %-8s | %-6s | %-6s | %-6s | %-6s | %-6s | %-6s | %-6s\n", "Singly Unsorted", "O(n)", "O(1)", "O(n)", "O(n)", "O(n)", "O(n)", "O(n)", "O(n)");
    printf("%-20s | %-8s | %-6s | %-6s | %-6s | %-6s | %-6s | %-6s | %-6s\n", "Singly Sorted", "O(n)", "O(n)", "O(n)", "O(1)*", "O(1)", "O(n)", "O(1)", "O(n)");
    printf("%-20s | %-8s | %-6s | %-6s | %-6s | %-6s | %-6s | %-6s | %-6s\n", "Doubly Unsorted", "O(n)", "O(1)", "O(1)", "O(n)", "O(n)", "O(n)", "O(n)", "O(n)");
    printf("%-20s | %-8s | %-6s | %-6s | %-6s | %-6s | %-6s | %-6s | %-6s\n", "Doubly Sorted", "O(n)", "O(n)", "O(1)", "O(1)", "O(1)", "O(1)", "O(1)", "O(n)");
    printf("=========================================================================================\n");
    printf("* Note: Singly Sorted Max is O(1) assuming a maintained tail pointer; otherwise O(n).\n\n");
}

void print_conclusion() {
    printf("=========================================================================================\n");
    printf("                   EXHAUSTIVE ASYMPTOTIC & THEORETICAL ANALYSIS                          \n");
    printf("=========================================================================================\n\n");

    printf("1. SEARCH OPERATION MECHANICS:\n");
    printf("   - Unsorted Array & Linked Lists: Must perform Linear Search O(n). Every element must be\n");
    printf("     inspected in the worst case because keys are stored without ordering invariants.\n");
    printf("   - Singly/Doubly Sorted Lists: Still incur O(n) worst-case search complexity despite\n");
    printf("     being sorted! Linked structures lack random access (O(1) indexing), making Binary\n");
    printf("     Search impossible because calculating and jumping to the middle node requires O(n) steps.\n");
    printf("   - Sorted Array: The solitary structure achieving O(log n) search runtime via Binary Search,\n");
    printf("     enabled by contiguous physical memory addresses allowing O(1) mid-point lookup.\n\n");

    printf("2. INSERTION OPERATION DYNAMICS:\n");
    printf("   - Unsorted Array: O(1) constant time by appending directly to array[size].\n");
    printf("   - Unsorted Linked Lists (Singly/Doubly): O(1) constant time by pushing nodes onto\n");
    printf("     the head in continuous O(1) pointer updates without traversing the list.\n");
    printf("   - Sorted Array: O(n) linear time worst case, e.g. inserting a new minimum forces every\n");
    printf("     existing element to shift one slot to the right to keep the array sorted.\n");
    printf("   - Sorted Linked Lists: O(n) worst-case traversal time to find the correct sorted node\n");
    printf("     position (head/tail shortcuts only help when the new key is the new min/max).\n\n");

    printf("3. DELETION OPERATION ADVANTAGES (Given Node Pointer / Index):\n");
    printf("   - Unsorted Array: O(1) via swap-with-last trick (array[idx] = array[--size]).\n");
    printf("   - Doubly Linked Lists (Unsorted & Sorted): O(1) constant time because node->prev and\n");
    printf("     node->next pointers allow immediate pointer bypass and re-linking without traversal.\n");
    printf("   - Sorted Array: O(n) linear time because deleting requires shifting all subsequent elements\n");
    printf("     leftward to eliminate memory gaps.\n");
    printf("   - Singly Linked Lists (Unsorted & Sorted): O(n) worst-case because updating the\n");
    printf("     predecessor node's 'next' pointer requires searching from head to locate the\n");
    printf("     preceding node -- the lack of a 'prev' pointer cannot be avoided even with a\n");
    printf("     pointer to the node itself.\n\n");

    printf("4. EXTREMA RETRIEVAL (MAX / MIN):\n");
    printf("   - Unsorted Structures: Must execute an full O(n) scan to compare every element.\n");
    printf("   - Sorted Structures: Offer instant O(1) lookup. Min is at head/index 0; Max is at\n");
    printf("     tail/index n-1 (provided a tail pointer is maintained in Singly Sorted List).\n\n");

    printf("5. NEIGHBOR RELATIONSHIPS (PREDECESSOR / SUCCESSOR):\n");
    printf("   - Unsorted Structures: O(n) scan required to compare all elements against target key.\n");
    printf("   - Sorted Array: O(1) direct neighbor indexing (array[idx - 1] and array[idx + 1]).\n");
    printf("   - Doubly Linked Sorted List: O(1) via node->prev and node->next pointers.\n");
    printf("   - Singly Linked Sorted List: Successor is O(1) via node->next, but Predecessor is O(n)\n");
    printf("     due to the lack of backward pointers.\n\n");

    printf("6. SPACE COMPLEXITY & HARDWARE OVERHEAD:\n");
    printf("   - Theoretical Space: All structures maintain O(n) auxiliary space complexity.\n");
    printf("   - Physical Overhead: Linked lists require significantly higher memory footprint than\n");
    printf("     contiguous arrays due to per-node pointer structures (8-16 bytes per node on 64-bit).\n");
    printf("   - Cache Locality: Contiguous Arrays maximize L1/L2 CPU cache hit rates. Linked lists\n");
    printf("     cause frequent CPU cache misses due to heap allocation fragmentations.\n\n");

    printf("7. HOW BENCHMARK DATA (CSV & GRAPHS) VALIDATES BIG-O THEORY:\n");
    printf("   - O(1) Growth: Empirical execution times remain horizontal across all N = 2000 to 20000.\n");
    printf("   - O(n) Growth: Runtime scales linearly with N. Doubling N doubles execution time.\n");
    printf("   - O(log n) Growth: Graph line hugs near-zero seconds, scaling logarithmically flat.\n");
    printf("=========================================================================================\n\n");
}

// ============================================================================
// MAIN BENCHMARK DRIVER
// ============================================================================
int main() {
    print_complexities();
    print_conclusion();

    FILE *fp = fopen("dictionary_operations.csv", "w");
    if (!fp) return 1;
    fprintf(fp, "N,Structure,Operation,Time\n");

    int REPS = 1000; // Repetitions to average out CPU noise
    printf("Running empirical benchmarks across N = 2000 to 20000... Please wait.\n");

    for (int N = 2000; N <= 20000; N += 2000) {
        ArrayDS ua = {(int*)malloc(sizeof(int)*(N+REPS+10)), 0, N+REPS+10};
        ArrayDS sa = {(int*)malloc(sizeof(int)*(N+REPS+10)), 0, N+REPS+10};
        SinglyUnsorted su = {NULL, 0}; SinglySorted ss = {NULL, NULL, 0};
        DoublyUnsorted du = {NULL, 0}; DoublySorted ds = {NULL, NULL, 0};

        // Populate Data Structures
        for (int i = 0; i < N; i++) {
            int val = i * 2;
            UA_Insert(&ua, val); SA_Insert(&sa, val);
            SU_Insert(&su, val); SS_Insert(&ss, val);
            DU_Insert(&du, val); DS_Insert(&ds, val);
        }

        int missing_k = -1; // Force worst case (guaranteed absent: all stored keys are >= 0)
        int mid_idx = N / 2;
        int max_key = (N - 1) * 2;
        SNode *su_mid = SU_Search(&su, mid_idx * 2); SNode *ss_mid = SS_Search(&ss, mid_idx * 2);
        DNode *du_mid = DU_Search(&du, mid_idx * 2); DNode *ds_mid = DS_Search(&ds, mid_idx * 2);

        clock_t start;
        #define BENCHMARK(DS_NAME, OP_NAME, CALL_EXPR) \
            start = clock(); \
            for(int r=0; r<REPS; r++) { CALL_EXPR; } \
            fprintf(fp, "%d,%s,%s,%.8f\n", N, DS_NAME, OP_NAME, ((double)(clock()-start))/CLOCKS_PER_SEC / REPS);

        // 1. Benchmark Unsorted Array
        BENCHMARK("Unsorted Array", "Search", UA_Search(&ua, missing_k));
        BENCHMARK("Unsorted Array", "Insert", UA_Insert(&ua, 999); ua.size--);
        BENCHMARK("Unsorted Array", "Delete", UA_Delete(&ua, mid_idx); UA_Insert(&ua, mid_idx*2));
        BENCHMARK("Unsorted Array", "Max", UA_Max(&ua));
        BENCHMARK("Unsorted Array", "Min", UA_Min(&ua));
        BENCHMARK("Unsorted Array", "Predecessor", UA_Predecessor(&ua, mid_idx));
        BENCHMARK("Unsorted Array", "Successor", UA_Successor(&ua, mid_idx));

        // 2. Benchmark Sorted Array
        // True worst case = insert smaller than the current minimum, forcing a shift
        // of every element. Pair with an O(1) delete-from-end to keep size stable.
        BENCHMARK("Sorted Array", "Search", SA_Search(&sa, missing_k));
        BENCHMARK("Sorted Array", "Insert", SA_Insert(&sa, -2); SA_Delete(&sa, sa.size - 1));
        BENCHMARK("Sorted Array", "Delete", SA_Delete(&sa, 0); SA_Insert(&sa, -2));
        BENCHMARK("Sorted Array", "Max", SA_Max(&sa));
        BENCHMARK("Sorted Array", "Min", SA_Min(&sa));
        BENCHMARK("Sorted Array", "Predecessor", SA_Predecessor(&sa, mid_idx));
        BENCHMARK("Sorted Array", "Successor", SA_Successor(&sa, mid_idx));

        // 3. Benchmark Singly Unsorted
        // Insert (head push) is genuinely O(1), no fix needed.
        // FIX (Delete): previously re-inserted at the head after every delete, which
        // made su_mid point at the head node from rep 1 onward -- collapsing ~999 of
        // the 1000 reps into an O(1) head-delete and hiding the true O(n) cost. Now
        // we re-insert at the TAIL every time via SU_InsertTail, so su_mid stays at
        // the far end of the list and every rep pays the full O(n) traversal for
        // both the delete (walking to find the predecessor) and the re-insert.
        BENCHMARK("Singly Unsorted", "Search", SU_Search(&su, missing_k));
        BENCHMARK("Singly Unsorted", "Insert", SU_Insert(&su, 999); SU_Delete(&su, su.head));
        BENCHMARK("Singly Unsorted", "Delete", SU_Delete(&su, su_mid); su_mid = SU_InsertTail(&su, mid_idx*2));
        BENCHMARK("Singly Unsorted", "Max", SU_Max(&su));
        BENCHMARK("Singly Unsorted", "Min", SU_Min(&su));
        BENCHMARK("Singly Unsorted", "Predecessor", SU_Predecessor(&su, su_mid));
        BENCHMARK("Singly Unsorted", "Successor", SU_Successor(&su, su_mid));

        // 4. Benchmark Singly Sorted
        // Pair the O(n) traversal-insert with an O(1) head-delete instead of an
        // O(n) tail-delete, so the "Insert" timing isn't doubled up with a second O(n) op.
        BENCHMARK("Singly Sorted", "Search", SS_Search(&ss, missing_k));
        BENCHMARK("Singly Sorted", "Insert", SS_Insert(&ss, N*3); SS_Delete(&ss, ss.head));
        BENCHMARK("Singly Sorted", "Delete", SS_Delete(&ss, ss_mid); ss_mid = SS_Insert(&ss, mid_idx*2));
        BENCHMARK("Singly Sorted", "Max", SS_Max(&ss));
        BENCHMARK("Singly Sorted", "Min", SS_Min(&ss));
        BENCHMARK("Singly Sorted", "Predecessor", SS_Predecessor(&ss, ss_mid));
        BENCHMARK("Singly Sorted", "Successor", SS_Successor(&ss, ss_mid));

        // 5. Benchmark Doubly Unsorted
        // Use the pointer returned by DU_Insert instead of re-searching -- no O(n)
        // search needed to refresh the pointer, so the true O(1) delete cost shows.
        BENCHMARK("Doubly Unsorted", "Search", DU_Search(&du, missing_k));
        BENCHMARK("Doubly Unsorted", "Insert", DU_Insert(&du, 999); DU_Delete(&du, du.head));
        BENCHMARK("Doubly Unsorted", "Delete", DU_Delete(&du, du_mid); du_mid = DU_Insert(&du, mid_idx*2));
        BENCHMARK("Doubly Unsorted", "Max", DU_Max(&du));
        BENCHMARK("Doubly Unsorted", "Min", DU_Min(&du));
        BENCHMARK("Doubly Unsorted", "Predecessor", DU_Predecessor(&du, du_mid));
        BENCHMARK("Doubly Unsorted", "Successor", DU_Successor(&du, du_mid));

        // 6. Benchmark Doubly Sorted
        // FIX 1: N*3 hit the O(1) tail-append shortcut (best case). Use a key just below
        // the current maximum so it falls into the O(n) traversal branch (true worst case),
        // paired with an O(1) head-delete to keep size stable.
        // FIX 2: Use the pointer returned by DS_Insert instead of searching for it.
        // Note DS_Delete is O(1) regardless of a node's position, but DS_Insert is NOT --
        // reinserting at mid_idx*2 (a mid-list value) would force an O(n) traversal and
        // silently reintroduce the same class of bug, so we reinsert as a new minimum,
        // which hits the O(1) head-shortcut every time.
        BENCHMARK("Doubly Sorted", "Search", DS_Search(&ds, missing_k));
        BENCHMARK("Doubly Sorted", "Insert", DS_Insert(&ds, max_key - 1); DS_Delete(&ds, ds.head));
        BENCHMARK("Doubly Sorted", "Delete", DS_Delete(&ds, ds_mid); ds_mid = DS_Insert(&ds, -1000000 - r));
        BENCHMARK("Doubly Sorted", "Max", DS_Max(&ds));
        BENCHMARK("Doubly Sorted", "Min", DS_Min(&ds));
        BENCHMARK("Doubly Sorted", "Predecessor", DS_Predecessor(&ds, ds_mid));
        BENCHMARK("Doubly Sorted", "Successor", DS_Successor(&ds, ds_mid));

        free(ua.arr); free(sa.arr);
    }

    fclose(fp);
    printf("Benchmark complete! All empirical timing data successfully saved to 'dictionary_operations.csv'.\n");
    printf("Run the Python script to plot the 7 individual validation graphs.\n");
    return 0;
}