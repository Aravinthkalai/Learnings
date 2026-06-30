/*
 * =============================================================================
 * FILE    : struct_array_concepts.c
 * AUTHOR  : Aravinth K
 * PURPOSE : Complete reference — Structures & Arrays (all priority levels)
 *           One file. Run it, read the comments, understand everything.
 *
 * Compile : gcc -O0 -Wall -Wextra -std=c11 -o sa struct_array_concepts.c
 * Run     : ./sa
 * Leak chk: valgrind --leak-check=full ./sa
 * =============================================================================
 *
 * ── STRUCTURES ───────────────────────────────────────────────────────────────
 * HIGH PRIORITY
 *  S1.  Declaration & Initialization (positional, designated, partial)
 *  S2.  Padding & Alignment (sizeof trap)
 *  S3.  Struct Packing (#pragma pack / __attribute__((packed)))
 *  S4.  Pass by Value vs by Pointer
 *  S5.  Nested Struct (embedded + separate)
 *  S6.  Bit Fields (unsigned vs signed trap)
 *  S7.  Self-Referential Struct (linked list + binary tree)
 *  S8.  Struct with Function Pointers (vtable / OOP in C)
 *  S9.  typedef struct patterns
 *  S10. Flexible Array Member (FAM / struct hack)
 *
 * MEDIUM PRIORITY
 *  S11. Shallow Copy vs Deep Copy
 *  S12. Array of Structs (AoS) vs Struct of Arrays (SoA)
 *  S13. Struct as Function Return Value
 *  S14. Anonymous Struct (inside union / outer struct)
 *  S15. offsetof & Struct Memory Layout Visualization
 *
 * LOW PRIORITY
 *  S16. Struct Comparison (no == operator → memcmp)
 *  S17. Struct Serialization (memcpy to byte buffer)
 *  S18. Bit Field Across Storage Units & Zero-Width Field
 *
 * ── ARRAYS ───────────────────────────────────────────────────────────────────
 * HIGH PRIORITY
 *  A1.  Array Decay to Pointer (the fundamental rule)
 *  A2.  sizeof Array vs sizeof Pointer (classic trap)
 *  A3.  Passing Arrays to Functions (decay + size loss)
 *  A4.  Array of Structs Initialization
 *  A5.  2D Array Memory Layout (row-major) & Pointer Arithmetic
 *  A6.  Dynamic Array (malloc + realloc grow pattern)
 *  A7.  VLA — Variable Length Array (C99)
 *  A8.  Array Bounds — No Bounds Checking (UB demo)
 *
 * MEDIUM PRIORITY
 *  A9.  String Arrays — char*[] vs char[][] (argv pattern)
 *  A10. Array Initialization Tricks (partial, {0}, designated)
 *  A11. Multi-dim Array Passing (all dims except first mandatory)
 *  A12. const Array vs Array of const
 *
 * LOW PRIORITY
 *  A13. Array of Function Pointers (dispatch table)
 *  A14. Compound Literals as Arrays
 *  A15. restrict with Array Parameters
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>   /* offsetof */
#include <stdalign.h> /* alignof — C11 */

/* ── helpers ──────────────────────────────────────────────────────────────── */
static void sec(const char *t)
{
    printf("\n╔══════════════════════════════════════════════════════════╗\n");
    printf("║  %-56s║\n", t);
    printf("╚══════════════════════════════════════════════════════════╝\n");
}
static void sub(const char *t) { printf("\n  ┌─ %s\n", t); }
static void sp(void)           { printf("\n"); }

/* ╔══════════════════════════════════════════════════════════════════════════╗
 * ║                        STRUCTURES                                       ║
 * ╚══════════════════════════════════════════════════════════════════════════╝ */

/* =============================================================================
 * S1. DECLARATION & INITIALIZATION
 *
 * Three styles:
 *   Positional   : values in declared order — order-sensitive, fragile
 *   Designated   : .member = value  (C99+) — order-free, self-documenting
 *   Partial      : unset members auto-zeroed when ANY member is initialized
 * =========================================================================== */
typedef struct {
    char  name[32];
    int   age;
    float salary;
    int   id;
} Employee;

void s1_declaration_init(void)
{
    sec("S1 [HIGH] STRUCT DECLARATION & INITIALIZATION");

    /* Positional — must match exact declaration order */
    Employee e1 = { "Alice", 30, 75000.0f, 1001 };

    /* Designated (C99+) — order-independent, recommended */
    Employee e2 = { .id = 1002, .name = "Bob", .salary = 85000.0f, .age = 35 };

    /* Partial — unspecified members are ZERO-INITIALIZED automatically */
    Employee e3 = { .name = "Charlie", .id = 1003 };
    /* e3.age = 0, e3.salary = 0.0f — guaranteed zeros */

    /* Zero-initialize ALL members */
    Employee e4 = { 0 };
    /* e4.name = "", e4.age = 0, e4.salary = 0.0f, e4.id = 0 */

    /* Runtime assignment — must do field-by-field (no = literal after decl) */
    Employee e5;
    strncpy(e5.name, "Dave", sizeof(e5.name) - 1);
    e5.age    = 28;
    e5.salary = 60000.0f;
    e5.id     = 1004;

    printf("  Positional : %-10s age=%-3d salary=%.0f id=%d\n",
           e1.name, e1.age, e1.salary, e1.id);
    printf("  Designated : %-10s age=%-3d salary=%.0f id=%d\n",
           e2.name, e2.age, e2.salary, e2.id);
    printf("  Partial    : %-10s age=%-3d salary=%.0f id=%d (unset=0)\n",
           e3.name, e3.age, e3.salary, e3.id);
    printf("  Zero-init  : name='%s' age=%d salary=%.0f id=%d\n",
           e4.name, e4.age, e4.salary, e4.id);
    printf("  Runtime    : %-10s age=%-3d salary=%.0f id=%d\n",
           e5.name, e5.age, e5.salary, e5.id);

    /*
     * INTERVIEW NOTES:
     *   - Designated initializer is C99+ — not in C89
     *   - Designated initializer NOT supported in C++ (use constructor instead)
     *   - Partial init: if ANY member initialized, REST are zeroed (not garbage)
     *   - {0} zeros everything — safest default initialization
     *   - Cannot assign struct literal after declaration: e5 = {"x",1,2.0f,3}; OK
     *     but only at init time. After declaration: field-by-field or use memcpy
     */
}

/* =============================================================================
 * S2. STRUCT PADDING & ALIGNMENT
 *
 * CPU reads memory aligned to its natural boundary:
 *   char   → 1-byte aligned  (any address)
 *   short  → 2-byte aligned  (even address)
 *   int    → 4-byte aligned  (multiple of 4)
 *   double → 8-byte aligned  (multiple of 8)
 *
 * Compiler inserts PADDING bytes between members to satisfy alignment.
 * sizeof(struct) ≠ sum of member sizes — always use sizeof().
 *
 * RULE: struct size is multiple of its LARGEST member's alignment.
 * =========================================================================== */

/* Poorly ordered — maximum padding */
struct BadOrder {
    char   a;  /* 1 byte  + 3 pad */
    int    b;  /* 4 bytes */
    char   c;  /* 1 byte  + 3 pad */
    int    d;  /* 4 bytes */
};             /* total = 16 bytes (wasted 6 bytes of padding) */

/* Well ordered — largest to smallest — minimum padding */
struct GoodOrder {
    int    b;  /* 4 bytes */
    int    d;  /* 4 bytes */
    char   a;  /* 1 byte  */
    char   c;  /* 1 byte  + 2 pad (struct size must be multiple of 4) */
};             /* total = 12 bytes */

/* Mixed types — complex alignment */
struct Mixed {
    char   a;   /* offset 0,  size 1  + 7 pad */
    double b;   /* offset 8,  size 8  */
    char   c;   /* offset 16, size 1  + 3 pad */
    int    d;   /* offset 20, size 4  */
};              /* total = 24 bytes */

void s2_padding_alignment(void)
{
    sec("S2 [HIGH] STRUCT PADDING & ALIGNMENT");

    printf("  struct BadOrder  { char,int,char,int }  sizeof = %zu  (wasted padding)\n",
           sizeof(struct BadOrder));
    printf("  struct GoodOrder { int,int,char,char  } sizeof = %zu  (optimized)\n",
           sizeof(struct GoodOrder));
    printf("  struct Mixed     { char,double,char,int} sizeof = %zu\n",
           sizeof(struct Mixed));

    sp();
    printf("  BadOrder layout:\n");
    printf("    [char a][pad pad pad][int b][char c][pad pad pad][int d]\n");
    printf("     1B      3B          4B      1B       3B          4B  = 16B\n");

    sp();
    printf("  GoodOrder layout:\n");
    printf("    [int b][int d][char a][char c][pad pad]\n");
    printf("     4B     4B     1B      1B      2B      = 12B\n");

    sp();
    /* Prove it with offsetof */
    printf("  Mixed offsets via offsetof:\n");
    printf("    a @ offset %-2zu  (1B + 7B pad before b)\n", offsetof(struct Mixed, a));
    printf("    b @ offset %-2zu  (8B double)\n",            offsetof(struct Mixed, b));
    printf("    c @ offset %-2zu  (1B + 3B pad before d)\n", offsetof(struct Mixed, c));
    printf("    d @ offset %-2zu  (4B int)\n",               offsetof(struct Mixed, d));
    printf("    total sizeof = %zu\n", sizeof(struct Mixed));

    sp();
    /* alignof tells natural alignment requirement of a type */
    printf("  alignof(char)  =%zu  alignof(int)  =%zu\n",
           _Alignof(char), _Alignof(int));
    printf("  alignof(double)=%zu  alignof(float)=%zu\n",
           _Alignof(double), _Alignof(float));

    /*
     * INTERVIEW NOTES:
     *   - NEVER assume sizeof(struct) = sum of members
     *   - Order members largest→smallest to reduce padding
     *   - offsetof() — compile-time, zero overhead
     *   - """""" %%% Struct size is always multiple of its largest member's alignment :)""""
     *     Alignment is always based on next member data type + current offset
     *   - Padding can be up to (alignment-1) bytes
     *   - Rule: each member starts at offset that is multiple of its alignment
     */
}

/* =============================================================================
 * S3. STRUCT PACKING
 *
 * Removing compiler padding:
 *   Method 1: #pragma pack(1)  — before struct, restore after
 *   Method 2: __attribute__((packed))  — per-struct (GCC/Clang)
 *
 * Use case: network protocol headers, file formats, hardware registers
 *           where exact byte layout MUST match specification.
 *
 * WARNING: Unaligned access on some CPUs = CRASH or performance penalty.
 *          ARM Cortex-M0/M0+ fault on unaligned access.
 *          x86 handles it but slower.
 * =========================================================================== */

/* Unpacked — compiler adds padding */
struct Unpacked {
    uint8_t  type;    /* 1 byte + 1 pad */
    uint16_t length;  /* 2 bytes */
    uint32_t value;   /* 4 bytes */
    uint8_t  flags;   /* 1 byte + 3 pad */
};

/* Packed method 1 — pragma pack */
#pragma pack(push, 1)       /* save current alignment, set to 1 */
struct PackedPragma {
    uint8_t  type;    /* 1 byte — no padding inserted */
    uint16_t length;  /* 2 bytes — may be unaligned! */
    uint32_t value;   /* 4 bytes — may be unaligned! */
    uint8_t  flags;   /* 1 byte */
};
#pragma pack(pop)           /* restore previous alignment */

/* Packed method 2 — GCC attribute */
struct PackedAttr {
    uint8_t  type;
    uint16_t length;
    uint32_t value;
    uint8_t  flags;
} __attribute__((packed));

/* Network packet header — real use case */
#pragma pack(push, 1)
struct UdpHeader {
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t length;
    uint16_t checksum;
};
/*It restores the packing for the next structures, not the one that was already defined.*/
#pragma pack(pop)


void s3_struct_packing(void)
{
    sec("S3 [HIGH] STRUCT PACKING (#pragma pack / __attribute__((packed)))");

    printf("  struct Unpacked     sizeof = %zu  (has padding)\n",
           sizeof(struct Unpacked));
    printf("  struct PackedPragma sizeof = %zu  (no padding)\n",
           sizeof(struct PackedPragma));
    printf("  struct PackedAttr   sizeof = %zu  (no padding)\n",
           sizeof(struct PackedAttr));
    printf("  struct UdpHeader    sizeof = %zu  (exact 8 bytes — matches spec)\n",
           sizeof(struct UdpHeader));

    sp();
    printf("  Unpacked layout:\n");
    printf("    type(1)+pad(1) | length(2) | value(4) | flags(1)+pad(3) = %zu\n",
           sizeof(struct Unpacked));
    printf("  Packed layout:\n");
    printf("    type(1) | length(2) | value(4) | flags(1) = %zu\n",
           sizeof(struct PackedPragma));

    /* Simulate network packet parsing */
    uint8_t raw_packet[] = { 0x08, 0x00,  /* src_port = 2048 */
                              0x00, 0x50,  /* dst_port = 80   */
                              0x00, 0x1C,  /* length   = 28   */
                              0xAB, 0xCD   /* checksum         */ };
    struct UdpHeader *hdr = (struct UdpHeader *)raw_packet;
    printf("\n  Parsed UDP header from raw bytes:\n");
    printf("    src_port = %u\n", hdr->src_port);
    printf("    dst_port = %u\n", hdr->dst_port);
    printf("    length   = %u\n", hdr->length);

    /*
     * INTERVIEW NOTES:
     *   - Always use #pragma pack(push,1) + #pragma pack(pop) — never forget pop
     *   - Packed structs: reading uint32 at unaligned addr = fault on ARM Cortex-M0
     *   - On x86/Cortex-M4+: unaligned OK but slower
     *   - Use packed ONLY for serialization/protocol — never for general structs
     *   - After pragma pack: normal code RESUMES normal alignment
     *   - Safer alternative: manual byte arrays + bit shifting (portable)
     */
}

/* =============================================================================
 * S4. PASS BY VALUE vs BY POINTER
 *
 * By value  : entire struct COPIED onto stack — expensive for large structs
 * By pointer: only 8-byte pointer copied — original modified via ->
 * By const* : pointer efficiency + read-only guarantee
 * =========================================================================== */

typedef struct {
    int   data[256];   /* 1024 bytes — large struct */
    int   id;
    float score;
} BigStruct;

/* By value — 1024+ bytes copied every call — EXPENSIVE */
void process_by_value(BigStruct s)
{
    s.score += 10.0f;   /* modifies LOCAL copy only — original unchanged */
    (void)s;
}

/* By pointer — 8 bytes copied (the pointer) — EFFICIENT */
void process_by_pointer(BigStruct *s)
{
    s->score += 10.0f;  /* modifies ORIGINAL via pointer */
}

/* By const pointer — efficient + read-only guarantee */
void process_by_const_ptr(const BigStruct *s)
{
    printf("    id=%d score=%.1f (read-only access)\n", s->id, s->score);
    /* s->score = 0;  // COMPILE ERROR — const prevents modification */
}

void s4_pass_by_value_vs_pointer(void)
{
    sec("S4 [HIGH] PASS STRUCT BY VALUE vs BY POINTER");

    BigStruct obj = { .id = 42, .score = 100.0f };

    printf("  sizeof(BigStruct) = %zu bytes\n", sizeof(BigStruct));
    sp();

    printf("  By value  : passes %zu bytes on stack — original unchanged\n",
           sizeof(BigStruct));
    process_by_value(obj);
    printf("    After call: score = %.1f (unchanged)\n", obj.score);

    printf("  By pointer: passes 8 bytes — modifies original\n");
    process_by_pointer(&obj);
    printf("    After call: score = %.1f (changed!)\n", obj.score);

    printf("  By const* : passes 8 bytes — read-only access\n");
    process_by_const_ptr(&obj);

    /*
     * INTERVIEW NOTES:
     *   - Pass by value: safe (no aliasing), expensive (full copy on stack)
     *   - Pass by pointer: efficient, but caller's data can be modified
     *   - Pass by const*: best of both — efficient + prevents modification
     *   - Rule of thumb: struct > 16 bytes → always pass by pointer
     *   - Structs returned by value → compiler may use Return Value Optimization
     *   - In embedded: stack is limited — large by-value structs = stack overflow
     */
}

/* =============================================================================
 * S5. NESTED STRUCT
 *
 * Two styles:
 *   Embedded  : inner struct declared INSIDE outer struct definition
 *   Separate  : both declared independently — preferred (reusable)
 * =========================================================================== */

/* Separate style — preferred, reusable */
typedef struct {
    float x;
    float y;
} Point;

typedef struct {
    float width;
    float height;
} Size;

typedef struct {
    Point origin;   /* nested struct as member */
    Size  size;     /* another nested struct */
    int   color;
} Rect;

/* Embedded style — inner struct only visible inside outer */
struct Window {
    char title[32];
    struct {           /* anonymous inner struct — members accessed directly */
        int x, y;
        int w, h;
    } frame;           /* named, so accessed as win.frame.x */
    int z_order;
};

void s5_nested_struct(void)
{
    sec("S5 [HIGH] NESTED STRUCT (SEPARATE + EMBEDDED)");

    sub("Separate declaration — recommended");
    /* Designated initializer with nested struct braces */
    Rect r = {
        .origin = { .x = 10.0f, .y = 20.0f },
        .size   = { .width = 800.0f, .height = 600.0f },
        .color  = 0xFF0000
    };
    printf("  Rect: origin(%.0f,%.0f)  size(%.0fx%.0f)  color=0x%06X\n",
           r.origin.x, r.origin.y,
           r.size.width, r.size.height,
           r.color);

    /* Modify nested member via pointer */
    Rect *rp = &r;
    rp->origin.x = 50.0f;   /* arrow then dot */
    printf("  After move: origin(%.0f,%.0f)\n", r.origin.x, r.origin.y);

    sub("Embedded struct — members accessed via outer.inner.member");
    struct Window win = {
        .title   = "Main Window",
        .frame   = { .x=0, .y=0, .w=1920, .h=1080 },
        .z_order = 1
    };
    printf("  Window: '%s'  frame(%d,%d %dx%d)  z=%d\n",
           win.title, win.frame.x, win.frame.y,
           win.frame.w, win.frame.h, win.z_order);

    /* sizeof nested */
    printf("\n  sizeof(Point)=%zu  sizeof(Size)=%zu  sizeof(Rect)=%zu\n",
           sizeof(Point), sizeof(Size), sizeof(Rect));

    /*
     * INTERVIEW NOTES:
     *   - Separate declaration preferred — reusable across structs
     *   - Embedded declaration — inner type not accessible outside outer struct
     *   - Access: outer_var.inner_member.field (two dots for two levels)
     *   - Via pointer: ptr->inner_member.field (arrow then dot)
     *   - Initialization: nested braces {.inner={.f=v}} or flat {.inner.f=v} (C99)
     */
}

/* =============================================================================
 * S6. BIT FIELDS
 *
 * Pack multiple small values into one integer — saves memory.
 * Hardware registers, protocol flags, tight embedded structures.
 *
 * KEY TRAPS:
 *   1. Signed bit field near boundary → two's complement sign extension
 *   2. Cannot take address of bit field (&bf.field = ERROR)
 *   3. Cannot have array of bit fields
 *   4. Width 0 = force next field to start at next type boundary
 * =========================================================================== */

/* Status register — hardware style */
typedef struct {
    unsigned int enabled  : 1;   /* bit 0   — 0 or 1 */
    unsigned int mode     : 2;   /* bits 1-2 — 0 to 3 */
    unsigned int priority : 4;   /* bits 3-6 — 0 to 15 */
    unsigned int error    : 1;   /* bit 7   — 0 or 1 */
    unsigned int reserved : 24;  /* bits 8-31 — unused */
} StatusReg;                     /* sizeof = 4 (fits in uint32_t) */

/* Signed bit field TRAP */
struct SignedTrap {
    int value : 4;   /* signed 4-bit: range -8 to +7 */
};

/* Date using bit fields */
typedef struct {
    unsigned int day   : 5;   /* 1-31  needs 5 bits */
    unsigned int month : 4;   /* 1-12  needs 4 bits */
    unsigned int year  : 12;  /* 0-4095 needs 12 bits */
} PackedDate;                 /* sizeof = 4 (21 bits used, fits uint32) */

void s6_bit_fields(void)
{
    sec("S6 [HIGH] BIT FIELDS (unsigned vs signed TRAP)");

    sub("Hardware status register — unsigned bit fields");
    StatusReg sr = { 0 };
    sr.enabled  = 1;
    sr.mode     = 3;   /* 0b11 — max value for 2 bits */
    sr.priority = 7;
    sr.error    = 0;

    printf("  StatusReg: enabled=%u mode=%u priority=%u error=%u\n",
           sr.enabled, sr.mode, sr.priority, sr.error);
    printf("  sizeof(StatusReg) = %zu  (packed into 4 bytes)\n",
           sizeof(StatusReg));

    /* Overflow — silently wraps */
    sr.mode = 5;   /* 5 = 0b101 — truncated to 2 bits → 0b01 = 1 */
    printf("  mode=5 (overflow → truncated to %u)\n", sr.mode);

    sub("Signed bit field TRAP — two's complement sign extension");
    struct SignedTrap t;
    t.value = 7;    /* max positive for signed 4-bit: 0b0111 = 7 */
    printf("  signed 4-bit: value=7   → stored as %d  (OK)\n", t.value);
    t.value = 8;    /* 8 = 0b1000 — MSB=1 → NEGATIVE in signed! */
    printf("  signed 4-bit: value=8   → stored as %d  (SIGN EXTENDED!)\n", t.value);
    t.value = 15;   /* 15 = 0b1111 → stored as -1 */
    printf("  signed 4-bit: value=15  → stored as %d  (TRAP!)\n", t.value);
    printf("  FIX: always use 'unsigned int' for bit fields storing counts/flags\n");

    sub("PackedDate — bit fields for date");
    PackedDate d = { .day=25, .month=6, .year=2026 };
    printf("  PackedDate: %u/%u/%u  sizeof=%zu (vs 12 for 3 ints)\n",
           d.day, d.month, d.year, sizeof(PackedDate));

    sub("Bit field rules");
    printf("  CANNOT: &sr.enabled      (no address of bit field)\n");
    printf("  CANNOT: unsigned int x[4]:2 (no array of bit fields)\n");
    printf("  CAN   : unsigned int :0   (zero-width = force next boundary)\n");
    printf("  CAN   : unsigned int :3   (unnamed = skip/pad 3 bits)\n");

    /*
     * INTERVIEW NOTES:
     *   - Always unsigned int for flag/count fields — avoid sign trap
     *   - signed int field with all 1s = negative (two's complement)
     *   - Bit field order (MSB→LSB or LSB→MSB) is implementation-defined
     *   - Bit fields across multiple storage units = implementation-defined
     *   - Width-0 unnamed field: unsigned int :0; forces alignment to next int
     *   - sizeof bit-field struct may not equal sum of bit widths / 8
     */
}

/* =============================================================================
 * S7. SELF-REFERENTIAL STRUCT (LINKED LIST + BINARY TREE)
 *
 * A struct containing a pointer to another object of its OWN type.
 * Foundation of: linked lists, trees, graphs, stacks, queues.
 * =========================================================================== */

/* Singly linked list */
typedef struct SLNode {
    int            data;
    struct SLNode *next;   /* pointer to same type — self-referential */
} SLNode;

/* Binary tree node */
typedef struct TreeNode {
    int              val;
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

SLNode *sl_push(SLNode *head, int data)
{
    SLNode *n = malloc(sizeof(SLNode));
    if (!n) return head;
    n->data = data;
    n->next = head;   /* new node points to old head */
    return n;         /* new head */
}

void sl_print(const SLNode *head)
{
    printf("  List: ");
    for (const SLNode *n = head; n; n = n->next)
        printf("%d%s", n->data, n->next ? " → " : " → NULL\n");
    if (!head) printf("(empty)\n");
}

void sl_free(SLNode *head)
{
    while (head) { SLNode *t = head->next; free(head); head = t; }
}

TreeNode *tree_insert(TreeNode *root, int val)
{
    if (!root) {
        TreeNode *n = malloc(sizeof(TreeNode));
        if (!n) return NULL;
        n->val = val; n->left = n->right = NULL;
        return n;
    }
    if (val < root->val) root->left  = tree_insert(root->left,  val);
    else                 root->right = tree_insert(root->right, val);
    return root;
}

void tree_inorder(const TreeNode *root)  /* prints sorted */
{
    if (!root) return;
    tree_inorder(root->left);
    printf("%d ", root->val);
    tree_inorder(root->right);
}

void tree_free(TreeNode *root)
{
    if (!root) return;
    tree_free(root->left);
    tree_free(root->right);
    free(root);
}

void s7_self_referential(void)
{
    sec("S7 [HIGH] SELF-REFERENTIAL STRUCT (LINKED LIST + BINARY TREE)");

    sub("Singly linked list");
    SLNode *list = NULL;
    list = sl_push(list, 30);
    list = sl_push(list, 20);
    list = sl_push(list, 10);
    sl_print(list);
    sl_free(list); list = NULL;

    sub("Binary search tree — inorder = sorted");
    int vals[] = {50, 30, 70, 20, 40, 60, 80};
    TreeNode *root = NULL;
    for (int i = 0; i < 7; i++) root = tree_insert(root, vals[i]);
    printf("  Inorder (sorted): ");
    tree_inorder(root);
    printf("\n");
    tree_free(root); root = NULL;

    printf("\n  KEY RULES:\n");
    printf("    struct Node { Node member; }   — ILLEGAL (infinite size)\n");
    printf("    struct Node { Node *ptr;    }  — LEGAL   (ptr = fixed 8 bytes)\n");
    printf("    Always: node->next = NULL;     — init before use\n");
    printf("    Free: save next BEFORE freeing current node\n");

    /*
     * INTERVIEW NOTES:
     *   - Cannot embed a struct inside itself — infinite size
     *   - CAN embed a POINTER to itself — pointer is always 8 bytes
     *   - Always initialize self-ptr to NULL — uninitialized = UB
     *   - Free: for(cur=head; cur; ) { next=cur->next; free(cur); cur=next; }
     *   - Tree free: post-order (free children before parent)
     */
}

/* =============================================================================
 * S8. STRUCT WITH FUNCTION POINTERS (vtable / OOP in C)
 *
 * Simulates polymorphism — different "objects" share the same API interface
 * but have different implementations via function pointer tables.
 * Used in: Zephyr device drivers, Linux file_operations, HAL layers.
 * =========================================================================== */

/* "Base class" — shape interface */
typedef struct Shape Shape;

typedef struct {
    float (*area)    (const Shape *s);
    float (*perimeter)(const Shape *s);
    void  (*print)   (const Shape *s);
    void  (*destroy) (Shape *s);
} ShapeOps;   /* vtable — function pointer table */

struct Shape {
    const ShapeOps *ops;   /* pointer to vtable */
    char            name[16];
};

/* Circle "subclass" */
typedef struct {
    Shape  base;    /* MUST be first — enables Shape* ↔ Circle* casting */
    float  radius;
} Circle;

static float  circle_area     (const Shape *s) { const Circle *c=(const Circle*)s; return 3.14159f*c->radius*c->radius; }
static float  circle_perimeter(const Shape *s) { const Circle *c=(const Circle*)s; return 2*3.14159f*c->radius; }
static void   circle_print    (const Shape *s) { const Circle *c=(const Circle*)s; printf("  Circle r=%.1f\n",c->radius); }
static void   circle_destroy  (Shape *s)       { free(s); }

static const ShapeOps circle_ops = {
    .area      = circle_area,
    .perimeter = circle_perimeter,
    .print     = circle_print,
    .destroy   = circle_destroy,
};

Shape *circle_create(float radius)
{
    Circle *c    = malloc(sizeof(Circle));
    if (!c) return NULL;
    c->base.ops  = &circle_ops;   /* point to vtable */
    strncpy(c->base.name, "Circle", sizeof(c->base.name)-1);
    c->radius    = radius;
    return (Shape *)c;            /* return as base type */
}

/* Rectangle "subclass" */
typedef struct {
    Shape base;
    float w, h;
} Rectangle;

static float  rect_area     (const Shape *s) { const Rectangle *r=(const Rectangle*)s; return r->w*r->h; }
static float  rect_perimeter(const Shape *s) { const Rectangle *r=(const Rectangle*)s; return 2*(r->w+r->h); }
static void   rect_print    (const Shape *s) { const Rectangle *r=(const Rectangle*)s; printf("  Rect   w=%.1f h=%.1f\n",r->w,r->h); }
static void   rect_destroy  (Shape *s)       { free(s); }

static const ShapeOps rect_ops = {
    .area=rect_area, .perimeter=rect_perimeter,
    .print=rect_print, .destroy=rect_destroy
};

Shape *rect_create(float w, float h)
{
    Rectangle *r = malloc(sizeof(Rectangle));
    if (!r) return NULL;
    r->base.ops  = &rect_ops;
    strncpy(r->base.name, "Rect", sizeof(r->base.name)-1);
    r->w = w; r->h = h;
    return (Shape *)r;
}

/* Generic function — works with ANY shape via vtable */
void print_shape_info(const Shape *s)
{
    s->ops->print(s);
    printf("    area=%.2f  perimeter=%.2f\n",
           s->ops->area(s), s->ops->perimeter(s));
}

void s8_struct_function_pointers(void)
{
    sec("S8 [HIGH] STRUCT WITH FUNCTION POINTERS (vtable / OOP in C)");

    Shape *shapes[3];
    shapes[0] = circle_create(5.0f);
    shapes[1] = rect_create(4.0f, 6.0f);
    shapes[2] = circle_create(3.0f);

    printf("  Polymorphic dispatch via vtable:\n");
    for (int i = 0; i < 3; i++) {
        print_shape_info(shapes[i]);        /* same call, different behavior */
        shapes[i]->ops->destroy(shapes[i]); /* virtual destructor */
        shapes[i] = NULL;
    }

    printf("\n  How vtable works:\n");
    printf("    Shape *s → s->ops → ShapeOps table → ops->area(s)\n");
    printf("    Circle and Rect have DIFFERENT ops tables\n");
    printf("    Same API (print_shape_info) → different behavior\n");
    printf("    Equivalent to C++ virtual functions\n");

    /*
     * INTERVIEW NOTES:
     *   - base struct MUST be first member — enables safe pointer casting
     *   - ops table is const — vtable never changes at runtime
     *   - void* or base* as first member = C-style inheritance
     *   - This exact pattern: Zephyr struct device, Linux file_operations
     *   - destroy() = virtual destructor pattern
     */
}

/* =============================================================================
 * S9. typedef STRUCT PATTERNS
 * =========================================================================== */

/* Pattern 1 — anonymous struct + typedef (most common in embedded) */
typedef struct {
    int x, y;
} Vec2;

/* Pattern 2 — named struct + typedef (needed for self-reference) */
typedef struct Node_ {
    int          data;
    struct Node_ *next;   /* must use struct Node_ here, not Node */
} Node;

/* Pattern 3 — forward declaration (for opaque pointers in headers) */
typedef struct Config_ Config;   /* incomplete type — size hidden */
struct Config_ { int baud; int parity; int stop_bits; };   /* defined here */

/* Pattern 4 — typedef for function pointer (readability) */
typedef int (*CompareFn)(const void *, const void *);

int int_compare(const void *a, const void *b)
{
    return *(const int*)a - *(const int*)b;
}

void s9_typedef_patterns(void)
{
    sec("S9 [HIGH] typedef STRUCT PATTERNS");

    sub("Pattern 1: anonymous struct + typedef");
    Vec2 v = { .x = 3, .y = 4 };
    printf("  Vec2: (%d, %d)\n", v.x, v.y);

    sub("Pattern 2: named struct + typedef (self-referential)");
    Node n2 = { 20, NULL };
    Node n1 = { 10, &n2 };
    printf("  Node list: %d → %d → NULL\n", n1.data, n1.next->data);

    sub("Pattern 3: forward declaration (opaque)");
    Config cfg = { .baud = 115200, .parity = 0, .stop_bits = 1 };
    printf("  Config: baud=%d parity=%d stop=%d\n",
           cfg.baud, cfg.parity, cfg.stop_bits);

    sub("Pattern 4: typedef for function pointer");
    int arr[] = {5, 2, 8, 1, 9};
    CompareFn cmp = int_compare;
    qsort(arr, 5, sizeof(int), cmp);
    printf("  Sorted: ");
    for (int i = 0; i < 5; i++) printf("%d ", arr[i]);
    printf("\n");

    printf("\n  Self-referential rule:\n");
    printf("    typedef struct Foo_ { struct Foo_ *next; } Foo;\n");
    printf("    Inside struct body: use 'struct Foo_' not 'Foo'\n");
    printf("    (typedef not visible yet inside its own definition)\n");

    /*
     * INTERVIEW NOTES:
     *   - Anonymous struct typedef: most compact, cannot self-reference
     *   - Named struct typedef: necessary for self-referential structs
     *   - Convention: struct Tag_ with typedef Tag (underscore suffix for tag)
     *   - C++ note: in C++, struct Foo also defines type Foo — no typedef needed
     *   - typedef for fn ptr: makes complex declarations readable
     */
}

/* =============================================================================
 * S10. FLEXIBLE ARRAY MEMBER (FAM)
 *
 * C99 feature: last member of struct can be an incomplete array type [].
 * Allows variable-length data appended after the struct header.
 * Used in: network packets, message queues, kernel objects.
 *
 * struct Header { int len; char data[]; };  // data[] has no size
 * size = sizeof(Header) + n * sizeof(char)  // allocate enough for n elements
 * =========================================================================== */

typedef struct {
    int    id;
    int    length;    /* number of elements in data */
    float  data[];    /* FAM — zero size in struct, real data follows in heap */
} FloatMsg;

FloatMsg *msg_create(int id, const float *vals, int n)
{
    /* Allocate header + n floats in ONE malloc */
    FloatMsg *m = malloc(sizeof(FloatMsg) + n * sizeof(float));
    if (!m) return NULL;
    m->id     = id;
    m->length = n;
    memcpy(m->data, vals, n * sizeof(float));   /* copy into trailing array */
    return m;
}

void s10_flexible_array_member(void)
{
    sec("S10 [HIGH] FLEXIBLE ARRAY MEMBER (FAM)");

    printf("  sizeof(FloatMsg) = %zu  (header only — FAM has zero size)\n",
           sizeof(FloatMsg));

    float sensor_data[] = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f};
    FloatMsg *msg = msg_create(7, sensor_data, 5);

    if (msg) {
        printf("  Message id=%d len=%d data: ", msg->id, msg->length);
        for (int i = 0; i < msg->length; i++)
            printf("%.1f ", msg->data[i]);
        printf("\n");
        printf("  Total allocated = %zu bytes\n",
               sizeof(FloatMsg) + msg->length * sizeof(float));
        free(msg);
        msg = NULL;
    }

    printf("\n  FAM RULES:\n");
    printf("    Must be LAST member of struct\n");
    printf("    Struct must have at least one other member\n");
    printf("    sizeof() excludes FAM — always zero\n");
    printf("    Cannot stack-allocate FAM struct (size unknown at compile time)\n");
    printf("    Cannot copy FAM struct with = (only copies header!)\n");
    printf("    One malloc covers header + data — better cache locality\n");

    /*
     * INTERVIEW NOTES:
     *   - C99+ standard. Not in C89 (old compilers used char data[1] hack)
     *   - sizeof(struct_with_FAM) excludes the FAM
     *   - Allocation: malloc(sizeof(S) + n * sizeof(element))
     *   - One allocation = better performance than separate header+data mallocs
     *   - Cannot have array of FAM structs — size undefined
     *   - Cannot copy: s2 = s1 only copies header, not FAM data — use memcpy
     */
}

/* =============================================================================
 * S11. SHALLOW COPY vs DEEP COPY (MEDIUM)
 * =========================================================================== */

typedef struct {
    char  name[32];
    int  *scores;    /* pointer to heap — shallow copy problem! */
    int   count;
} Student;

Student student_shallow_copy(const Student *src)
{
    Student dst = *src;   /* copies pointer value — both point to SAME heap */
    return dst;
    /* dst.scores == src->scores — same address! */
    /* freeing one frees the shared memory — other becomes dangling */
}

Student student_deep_copy(const Student *src)
{
    Student dst = *src;   /* copy all fields including pointer value */
    /* Allocate NEW heap memory for dst */
    dst.scores = malloc(src->count * sizeof(int));
    if (dst.scores) {
        memcpy(dst.scores, src->scores, src->count * sizeof(int));
    }
    return dst;   /* dst.scores → independent copy of data */
}

void s11_shallow_vs_deep_copy(void)
{
    sec("S11 [MEDIUM] SHALLOW COPY vs DEEP COPY");

    Student orig;
    strncpy(orig.name, "Alice", sizeof(orig.name)-1);
    orig.count  = 4;
    orig.scores = malloc(4 * sizeof(int));
    int temp[] = {90, 85, 92, 88};
    memcpy(orig.scores, temp, sizeof(temp));

    sub("Shallow copy — shared pointer (DANGEROUS)");
    Student sc = student_shallow_copy(&orig);
    printf("  orig.scores  @ %p\n", (void*)orig.scores);
    printf("  sc.scores    @ %p  (SAME address — shared!)\n", (void*)sc.scores);
    printf("  Modifying sc.scores[0]=0 also changes orig!\n");
    sc.scores[0] = 0;
    printf("  orig.scores[0] = %d  (unexpectedly changed)\n", orig.scores[0]);
    orig.scores[0] = 90; /* restore */
    /* sc.scores intentionally NOT freed — same as orig.scores */

    sub("Deep copy — independent heap allocation (SAFE)");
    Student dc = student_deep_copy(&orig);
    printf("  orig.scores  @ %p\n", (void*)orig.scores);
    printf("  dc.scores    @ %p  (DIFFERENT address — independent copy)\n",
           (void*)dc.scores);
    dc.scores[0] = 0;
    printf("  orig.scores[0] = %d  (unchanged — truly independent)\n",
           orig.scores[0]);

    free(orig.scores); orig.scores = NULL;
    free(dc.scores);   dc.scores   = NULL;

    /*
     * INTERVIEW NOTES:
     *   - struct assignment (=) always does shallow copy
     *   - Shallow copy safe ONLY if struct has no pointer members
     *   - Deep copy: allocate new heap, memcpy the pointed-to data
     *   - Always free deep copies independently
     *   - Design: document ownership — who allocates, who frees
     */
}

/* =============================================================================
 * S12. ARRAY OF STRUCTS (AoS) vs STRUCT OF ARRAYS (SoA) — MEDIUM
 *
 * AoS: struct { x,y,z } arr[N]  — natural, easy to use
 * SoA: struct { x[N], y[N], z[N] } — better cache performance for
 *      operations on single field across all elements (SIMD, DSP)
 * =========================================================================== */

#define N_PARTICLES 6

/* AoS — Array of Structs */
typedef struct { float x, y, z, vx, vy, vz; } Particle;
typedef struct { Particle p[N_PARTICLES]; } AoS;

/* SoA — Struct of Arrays */
typedef struct {
    float x[N_PARTICLES];
    float y[N_PARTICLES];
    float z[N_PARTICLES];
    float vx[N_PARTICLES];
    float vy[N_PARTICLES];
    float vz[N_PARTICLES];
} SoA;

void s12_aos_vs_soa(void)
{
    sec("S12 [MEDIUM] ARRAY OF STRUCTS (AoS) vs STRUCT OF ARRAYS (SoA)");

    sub("AoS — natural layout, all fields of one particle together");
    AoS aos = {{{ 1,2,3,0.1f,0.2f,0.3f },
                 { 4,5,6,0.4f,0.5f,0.6f },
                 { 7,8,9,0.7f,0.8f,0.9f }}};

    /* Update x position — accesses x field but skips y,z,vx,vy,vz in cache */
    printf("  AoS update x: ");
    for (int i = 0; i < 3; i++) {
        aos.p[i].x += aos.p[i].vx;
        printf("%.2f ", aos.p[i].x);
    }
    printf("\n");
    printf("  AoS memory: [x,y,z,vx,vy,vz|x,y,z,vx,vy,vz|...]\n");
    printf("  Cache: loading x also loads y,z,vx... (wasted cache lines)\n");

    sub("SoA — all x values contiguous, cache-friendly for x-only ops");
    SoA soa;
    for (int i = 0; i < N_PARTICLES; i++) {
        soa.x[i] = (float)(i*3+1);
        soa.vx[i]= (float)(i+1) * 0.1f;
        soa.y[i] = soa.z[i] = soa.vy[i] = soa.vz[i] = 0;
    }

    /* Update x — accesses ONLY x[] and vx[] arrays — perfectly cache-friendly */
    printf("  SoA update x: ");
    for (int i = 0; i < N_PARTICLES; i++) {
        soa.x[i] += soa.vx[i];
        printf("%.2f ", soa.x[i]);
    }
    printf("\n");
    printf("  SoA memory: [x0,x1,x2,...|y0,y1,y2,...|vx0,vx1,...]\n");
    printf("  Cache: loading x[] loads x0,x1,x2... (perfect for SIMD)\n");

    sub("When to use which");
    printf("  AoS: access ALL fields of ONE entity   → natural, readable\n");
    printf("  SoA: access ONE field across ALL entities → cache/SIMD optimal\n");
    printf("  Examples:\n");
    printf("    Game objects (position+color+health together) → AoS\n");
    printf("    Physics sim  (update all x, then all y)      → SoA\n");
    printf("    DSP/SIMD     (process array of same field)   → SoA\n");
}

/* =============================================================================
 * S13. STRUCT AS FUNCTION RETURN VALUE — MEDIUM
 * =========================================================================== */

typedef struct { float real; float imag; } Complex;

Complex complex_add(Complex a, Complex b)
{
    return (Complex){ a.real + b.real, a.imag + b.imag };
    /* Compiler uses Return Value Optimization (RVO) — no copy in practice */
}

Complex complex_mul(Complex a, Complex b)
{
    return (Complex){
        .real = a.real*b.real - a.imag*b.imag,
        .imag = a.real*b.imag + a.imag*b.real
    };
}

void s13_struct_return_value(void)
{
    sec("S13 [MEDIUM] STRUCT AS FUNCTION RETURN VALUE");

    Complex a = { 3.0f, 4.0f };
    Complex b = { 1.0f, 2.0f };

    Complex sum = complex_add(a, b);
    Complex prd = complex_mul(a, b);

    printf("  a = %.1f + %.1fi\n", a.real, a.imag);
    printf("  b = %.1f + %.1fi\n", b.real, b.imag);
    printf("  a+b = %.1f + %.1fi\n", sum.real, sum.imag);
    printf("  a*b = %.1f + %.1fi\n", prd.real, prd.imag);

    /* Compound literal — create struct inline without named variable */
    Complex c = complex_add((Complex){2.0f, 0.0f}, (Complex){0.0f, 1.0f});
    printf("  (2+0i)+(0+1i) = %.1f + %.1fi\n", c.real, c.imag);

    printf("\n  Return value notes:\n");
    printf("    Small structs (<=16 bytes): returned in registers — zero copy\n");
    printf("    Large structs: compiler uses hidden output ptr (RVO/NRVO)\n");
    printf("    Compound literal: (Type){...} — C99, creates unnamed struct\n");

    /*
     * INTERVIEW NOTES:
     *   - Returning struct by value is clean and often free (RVO)
     *   - Compound literal (Type){.f=v} creates temporary struct — C99
     *   - On x86-64: structs <= 16 bytes returned in registers (rax:rdx)
     *   - Large struct return: compiler passes hidden pointer as first arg
     *   - Prefer return-by-value for small result structs (clean API)
     */
}

/* =============================================================================
 * S14. ANONYMOUS STRUCT — MEDIUM
 * =========================================================================== */

void s14_anonymous_struct(void)
{
    sec("S14 [MEDIUM] ANONYMOUS STRUCT");

    /* Anonymous struct inside union — common pattern */
    typedef union {
        uint32_t raw;        /* access all 32 bits at once */
        struct {             /* anonymous struct — no struct name */
            uint8_t byte0;   /* members accessed directly via union var */
            uint8_t byte1;
            uint8_t byte2;
            uint8_t byte3;
        };                   /* no member name — members promoted to union scope */
    } U32Bytes;

    U32Bytes u;
    u.raw = 0x12345678;
    printf("  raw = 0x%08X\n", u.raw);
    printf("  byte0=0x%02X byte1=0x%02X byte2=0x%02X byte3=0x%02X\n",
           u.byte0, u.byte1, u.byte2, u.byte3);
    /* byte order depends on endianness — little-endian: byte0=0x78 */

    /* Anonymous struct inside struct — fields promoted to outer scope */
    typedef struct {
        int id;
        struct {         /* anonymous — no member name needed */
            int x, y;   /* accessed as outer.x, outer.y directly */
        };               /* NOT outer.inner.x */
        int color;
    } Sprite;

    Sprite sp = { .id=1, .x=100, .y=200, .color=0xFF };
    printf("  Sprite: id=%d pos=(%d,%d) color=0x%02X\n",
           sp.id, sp.x, sp.y, sp.color);
    /* sp.x directly — not sp.inner.x — that's the point of anonymous */

    /*
     * INTERVIEW NOTES:
     *   - Anonymous struct: no tag, no member name — members promoted to parent scope
     *   - Common in: union with raw + structured access, embedded register maps
     *   - C11 feature formally, but GCC/Clang supported it earlier as extension
     *   - Access: outer.member (not outer.inner.member)
     *   - Cannot have anonymous struct as standalone variable (no type name)
     */
}

/* =============================================================================
 * S15. offsetof & MEMORY LAYOUT VISUALIZATION — MEDIUM
 * =========================================================================== */

void s15_offsetof_layout(void)
{
    sec("S15 [MEDIUM] offsetof & MEMORY LAYOUT VISUALIZATION");

    typedef struct {
        uint8_t  a;    /* 1 byte */
        uint16_t b;    /* 2 bytes */
        uint32_t c;    /* 4 bytes */
        uint8_t  d;    /* 1 byte */
        uint64_t e;    /* 8 bytes */
    } Demo;

    printf("  struct Demo { uint8 a; uint16 b; uint32 c; uint8 d; uint64 e; }\n\n");
    printf("  Member  | offset | size | alignment\n");
    printf("  --------|--------|------|----------\n");
    printf("  a       | %-6zu | %-4zu | %zu\n", offsetof(Demo,a), sizeof(uint8_t),  _Alignof(uint8_t));
    printf("  b       | %-6zu | %-4zu | %zu\n", offsetof(Demo,b), sizeof(uint16_t), _Alignof(uint16_t));
    printf("  c       | %-6zu | %-4zu | %zu\n", offsetof(Demo,c), sizeof(uint32_t), _Alignof(uint32_t));
    printf("  d       | %-6zu | %-4zu | %zu\n", offsetof(Demo,d), sizeof(uint8_t),  _Alignof(uint8_t));
    printf("  e       | %-6zu | %-4zu | %zu\n", offsetof(Demo,e), sizeof(uint64_t), _Alignof(uint64_t));
    printf("  total   | sizeof = %zu\n", sizeof(Demo));

    printf("\n  Memory map (each cell = 1 byte):\n");
    printf("  [a][ pad][b  b][c  c  c  c][d][ pad pad pad][e  e  e  e  e  e  e  e]\n");
    printf("   0    1   2  3  4  5  6  7  8   9  10  11   12 13 14 15 16 17 18 19\n");

    /* Prove with actual addresses */
    Demo d = {0};
    char *base = (char *)&d;
    printf("\n  Actual offsets (computed at runtime):\n");
    printf("    &d.a - &d = %ld\n", (char*)&d.a - base);
    printf("    &d.b - &d = %ld\n", (char*)&d.b - base);
    printf("    &d.c - &d = %ld\n", (char*)&d.c - base);
    printf("    &d.d - &d = %ld\n", (char*)&d.d - base);
    printf("    &d.e - &d = %ld\n", (char*)&d.e - base);
}

/* =============================================================================
 * S16. STRUCT COMPARISON — LOW
 * =========================================================================== */

typedef struct { int x; int y; } Point2;

int point_equal(const Point2 *a, const Point2 *b)
{
    return (a->x == b->x) && (a->y == b->y);   /* field-by-field */
}

void s16_struct_comparison(void)
{
    sec("S16 [LOW] STRUCT COMPARISON (no == operator → use field-by-field or memcmp)");

    Point2 p1 = {3, 4};
    Point2 p2 = {3, 4};
    Point2 p3 = {5, 6};

    /* p1 == p2  → COMPILE ERROR: no == for structs */

    printf("  Field-by-field: p1==p2? %s\n", point_equal(&p1,&p2) ? "YES":"NO");
    printf("  Field-by-field: p1==p3? %s\n", point_equal(&p1,&p3) ? "YES":"NO");

    /* memcmp — DANGEROUS for structs with padding */
    /* padding bytes are UNINITIALIZED — memcmp may return non-zero for equal structs */
    int cmp = memcmp(&p1, &p2, sizeof(Point2));
    printf("  memcmp(p1,p2)=%d  (OK here — no padding in Point2)\n", cmp);
    printf("  WARNING: memcmp UNRELIABLE if struct has padding bytes!\n");
    printf("  SAFE: always compare field by field for structs with padding\n");

    /* Force zero padding before memcmp (memset trick) */
    Point2 pa, pb;
    memset(&pa, 0, sizeof(pa));  /* zero ALL bytes including padding */
    memset(&pb, 0, sizeof(pb));
    pa.x=3; pa.y=4;
    pb.x=3; pb.y=4;
    printf("  memcmp after memset: %d (reliable — padding zeroed)\n",
           memcmp(&pa,&pb,sizeof(Point2)));

    /*
     * INTERVIEW NOTES:
     *   - C has no == for struct — must compare field by field
     *   - memcmp on struct with padding = UNRELIABLE (padding = uninitialized)
     *   - Fix: memset struct to 0 before filling fields, then memcmp is safe
     *   - For union types: memcmp also unreliable (only active member valid)
     */
}

/* =============================================================================
 * S17. STRUCT SERIALIZATION (memcpy to byte buffer) — LOW
 * =========================================================================== */

#pragma pack(push, 1)
typedef struct {
    uint8_t  cmd;
    uint16_t length;
    uint32_t payload;
    uint8_t  checksum;
} Packet;   /* packed — exact wire format */
#pragma pack(pop)

uint8_t compute_checksum(const uint8_t *data, size_t n)
{
    uint8_t sum = 0;
    for (size_t i = 0; i < n; i++) sum ^= data[i];
    return sum;
}

void s17_struct_serialization(void)
{
    sec("S17 [LOW] STRUCT SERIALIZATION (memcpy to/from byte buffer)");

    Packet pkt = {
        .cmd      = 0xA5,
        .length   = 4,
        .payload  = 0xDEADBEEF,
        .checksum = 0
    };
    /* Compute checksum over all fields except checksum itself */
    pkt.checksum = compute_checksum((uint8_t*)&pkt, sizeof(pkt)-1);

    /* Serialize to byte buffer */
    uint8_t buf[sizeof(Packet)];
    memcpy(buf, &pkt, sizeof(Packet));

    printf("  Serialized packet (%zu bytes): ", sizeof(Packet));
    for (size_t i = 0; i < sizeof(Packet); i++) printf("%02X ", buf[i]);
    printf("\n");

    /* Deserialize — reconstruct struct from bytes */
    Packet received;
    memcpy(&received, buf, sizeof(Packet));
    printf("  Deserialized: cmd=0x%02X len=%u payload=0x%08X chk=0x%02X\n",
           received.cmd, received.length, received.payload, received.checksum);

    printf("\n  Serialization rules:\n");
    printf("    Always #pragma pack(1) for wire-format structs\n");
    printf("    Use fixed-width types: uint8_t, uint16_t, uint32_t\n");
    printf("    Handle endianness: htons()/ntohs() for network byte order\n");
    printf("    memcpy into packed struct — do NOT cast raw ptr to unpacked struct\n");
}

/* =============================================================================
 * S18. BIT FIELD ACROSS STORAGE UNITS & ZERO-WIDTH — LOW
 * =========================================================================== */

void s18_bit_field_advanced(void)
{
    sec("S18 [LOW] BIT FIELD — ACROSS STORAGE UNITS & ZERO-WIDTH FIELD");

    /* Zero-width field — forces next field to next storage unit boundary */
    struct ZeroWidth {
        unsigned int a : 4;   /* bits 0-3 of first int */
        unsigned int   : 0;   /* STOP — next field starts fresh int */
        unsigned int b : 4;   /* bits 0-3 of SECOND int */
    };
    printf("  Without :0 — {4bit,4bit}   sizeof would be 4 (packed in 1 int)\n");
    printf("  With    :0 — {4bit,:0,4bit} sizeof = %zu (forced to 2 ints)\n",
           sizeof(struct ZeroWidth));

    /* Unnamed bit field — skip/pad bits */
    struct WithSkip {
        unsigned int flag0 : 1;
        unsigned int       : 6;   /* unnamed — skip 6 bits (reserved) */
        unsigned int flag7 : 1;   /* bit 7 */
    };
    struct WithSkip ws = { .flag0=1, .flag7=1 };
    printf("  flag0=%u  (unnamed 6 bits skipped)  flag7=%u\n",
           ws.flag0, ws.flag7);

    printf("\n  Bit field rules summary:\n");
    printf("    :0   = force alignment to next storage unit boundary\n");
    printf("    :n   (unnamed) = skip n bits (reserved/padding)\n");
    printf("    Order inside storage unit: implementation-defined (LSB vs MSB first)\n");
    printf("    Bit field cannot span more than its storage unit size\n");
    printf("    sizeof(struct with bit fields) always multiple of storage type\n");
}

/* ╔══════════════════════════════════════════════════════════════════════════╗
 * ║                          ARRAYS                                         ║
 * ╚══════════════════════════════════════════════════════════════════════════╝ */

/* =============================================================================
 * A1. ARRAY DECAY TO POINTER — HIGH
 *
 * THE fundamental rule:
 *   In most expressions, array name DECAYS to pointer to its first element.
 *   Exceptions: sizeof(), &arr, _Alignof() — these see the array as a whole.
 * =========================================================================== */

void a1_array_decay(void)
{
    sec("A1 [HIGH] ARRAY DECAY TO POINTER");

    int arr[5] = {10, 20, 30, 40, 50};

    /* arr in expression context = &arr[0] */
    int *p = arr;           /* decay — p = &arr[0] */
    printf("  arr    = %p\n", (void*)arr);
    printf("  &arr[0]= %p  (same address)\n", (void*)&arr[0]);
    printf("  p      = %p  (same address after decay)\n", (void*)p);
    printf("  arr == &arr[0]: %s\n", (arr == &arr[0]) ? "YES" : "NO");

    /* sizeof does NOT decay */
    printf("\n  sizeof(arr) = %zu  (full array — no decay)\n", sizeof(arr));
    printf("  sizeof(p)   = %zu  (pointer — decayed)\n", sizeof(p));

    /* &arr vs arr — different TYPES, same ADDRESS */
    printf("\n  arr   = %p  type: int*\n",    (void*)arr);
    printf("  &arr  = %p  type: int(*)[5]\n", (void*)&arr);
    printf("  Same address but DIFFERENT pointer arithmetic:\n");
    printf("  arr+1   = %p  (+%zu bytes = sizeof int)\n",
           (void*)(arr+1), sizeof(int));
    printf("  &arr+1  = %p  (+%zu bytes = sizeof int[5])\n",
           (void*)(&arr+1), sizeof(arr));

    /* arr[i] == *(arr+i) — always */
    printf("\n  arr[2]     = %d\n", arr[2]);
    printf("  *(arr+2)   = %d  (identical)\n", *(arr+2));
    printf("  *(p+2)     = %d  (same through pointer)\n", *(p+2));

    /*
     * INTERVIEW NOTES:
     *   - Array decay means an array automatically converts ("decays (destroyed or losses identity)") into a pointer to 
     *     its first element in most expressions.
     *   - Array → pointer decay: array name becomes &arr[0] in expressions
     *   - Exceptions where array does NOT decay: sizeof(), &arr, _Alignof()
     *   - arr and &arr[0]: same address, same type (int*)
     *   - &arr: same address but type int(*)[5] — pointer to whole array
     *   - &arr+1 skips entire array (20 bytes). arr+1 skips one element (4 bytes)
     *   - Decay is one-way — pointer does NOT carry array size information
     */
}

/* =============================================================================
 * A2. sizeof ARRAY vs sizeof POINTER — HIGH (classic interview trap)
 * =========================================================================== */

void check_size(int arr[], int n)   /* arr[] here = int* — pointer, not array */
{
    printf("  Inside function: sizeof(arr)=%zu  (pointer!)  n=%d\n",
           sizeof(arr), n);
    printf("  sizeof(*arr)=%zu  (one element)\n", sizeof(*arr));
}

void a2_sizeof_array_vs_pointer(void)
{
    sec("A2 [HIGH] sizeof ARRAY vs sizeof POINTER (the classic trap)");

    int  arr[10];
    int *ptr = arr;

    printf("  At declaration site:\n");
    printf("    sizeof(arr)       = %zu  (full array: 10*4=40)\n", sizeof(arr));
    printf("    sizeof(ptr)       = %zu  (pointer only)\n", sizeof(ptr));
    printf("    sizeof(*ptr)      = %zu  (one int element)\n", sizeof(*ptr));
    printf("    ARRAY_SIZE(arr)   = %zu  (elements: 40/4=10)\n",
           sizeof(arr)/sizeof(arr[0]));

    printf("\n  After passing to function:\n");
    check_size(arr, 10);

    printf("\n  The fundamental rule:\n");
    printf("    Array DECAYS to pointer when passed to function\n");
    printf("    sizeof() inside function sees POINTER, not array\n");
    printf("    ALWAYS pass array size as separate argument\n");

    printf("\n  Types and sizes:\n");
    printf("    int arr[10] : type=int[10]  sizeof=%zu\n", sizeof(arr));
    printf("    int *ptr    : type=int*     sizeof=%zu\n", sizeof(ptr));
    printf("    int arr[]   : as param = int* (same!)\n");

    /*
     * INTERVIEW NOTES:
     *   - sizeof(arr) at declaration = total bytes
     *   - sizeof(arr) after decay (in function) = sizeof pointer = 8
     *   - #define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0])) — only at declaration
     *   - Common bug: for(i=0; i<sizeof(arr)/sizeof(arr[0]); i++) inside function
     *     → sizeof gives 8/4=2, not 10 — wrong loop count
     */
}

/* =============================================================================
 * A3. PASSING ARRAYS TO FUNCTIONS — HIGH
 * =========================================================================== */

/* All three declarations are IDENTICAL — all receive int* */
void fn_style1(int arr[],    int n) { printf("  style1: arr[2]=%d n=%d\n", arr[2], n); }
void fn_style2(int arr[10],  int n) { printf("  style2: arr[2]=%d n=%d\n", arr[2], n); }
void fn_style3(int *arr,     int n) { printf("  style3: arr[2]=%d n=%d\n", arr[2], n); }

/* Modifying through pointer — affects original */
void double_array(int *arr, int n)
{
    for (int i = 0; i < n; i++) arr[i] *= 2;
}

void a3_passing_arrays(void)
{
    sec("A3 [HIGH] PASSING ARRAYS TO FUNCTIONS");

    int arr[] = {10, 20, 30, 40, 50};
    int n = 5;

    printf("  All three function signatures are equivalent:\n");
    fn_style1(arr, n);
    fn_style2(arr, n);
    fn_style3(arr, n);

    printf("\n  Array passed by pointer — modifications affect original:\n");
    printf("  Before: ");
    for(int i=0;i<n;i++) printf("%d ",arr[i]);
    printf("\n");
    double_array(arr, n);
    printf("  After : ");
    for(int i=0;i<n;i++) printf("%d ",arr[i]);
    printf("\n");

    printf("\n  To protect array from modification: use const:\n");
    printf("    void read_only(const int *arr, int n);\n");
    printf("    const prevents writing through arr inside function\n");

    /*
     * INTERVIEW NOTES:
     *   - int arr[], int arr[10], int *arr as params: identical to compiler
     *   - Array SIZE in declaration (arr[10]) is decorative — ignored
     *   - Always pass n separately — no way to recover size from pointer
     *   - Arrays are always passed by reference (pointer) — never copied
     *   - Use const int* to express read-only intent
     */
}

/* =============================================================================
 * A4. ARRAY OF STRUCTS INITIALIZATION — HIGH
 * =========================================================================== */

typedef struct {
    int   id;
    char  name[16];
    float value;
} Item;

void a4_array_of_structs(void)
{
    sec("A4 [HIGH] ARRAY OF STRUCTS INITIALIZATION");

    /* Static initialization — array of struct */
    Item inventory[] = {
        { 1, "Resistor", 0.05f },          /* positional */
        { .id=2, .name="Capacitor", .value=0.10f },  /* designated */
        { 3, "Inductor" },                  /* partial — value=0.0f */
    };
    int n = sizeof(inventory)/sizeof(inventory[0]);

    printf("  Static init (%d items):\n", n);
    for (int i = 0; i < n; i++) {
        printf("    [%d] id=%-3d name=%-12s value=%.2f\n",
               i, inventory[i].id, inventory[i].name, inventory[i].value);
    }

    /* Zero-initialize entire array */
    Item sensors[4] = { 0 };   /* all fields of all elements = 0 */
    sensors[0].id = 10;
    strncpy(sensors[0].name, "Temp", sizeof(sensors[0].name)-1);
    sensors[0].value = 25.0f;
    printf("\n  Zero-init + runtime fill: id=%d name=%s val=%.1f\n",
           sensors[0].id, sensors[0].name, sensors[0].value);

    /* Dynamic array of structs */
    int count = 3;
    Item *dyn = malloc(count * sizeof(Item));
    if (dyn) {
        for (int i = 0; i < count; i++) {
            dyn[i].id    = i + 100;
            dyn[i].value = (float)(i+1) * 1.5f;
            snprintf(dyn[i].name, sizeof(dyn[i].name), "Sensor%d", i);
        }
        printf("\n  Dynamic array of structs:\n");
        for (int i = 0; i < count; i++) {
            printf("    id=%-3d name=%-10s val=%.2f\n",
                   dyn[i].id, dyn[i].name, dyn[i].value);
        }
        free(dyn); dyn = NULL;
    }
}

/* =============================================================================
 * A5. 2D ARRAY MEMORY LAYOUT & POINTER ARITHMETIC — HIGH
 * =========================================================================== */

void a5_2d_array_layout(void)
{
    sec("A5 [HIGH] 2D ARRAY MEMORY LAYOUT (ROW-MAJOR) & POINTER ARITHMETIC");

    int m[3][4] = {
        {1,  2,  3,  4},   /* row 0 */
        {5,  6,  7,  8},   /* row 1 */
        {9, 10, 11, 12}    /* row 2 */
    };

    printf("  int m[3][4] stored in row-major order:\n");
    printf("  [1][2][3][4][5][6][7][8][9][10][11][12] — contiguous\n\n");

    /* Prove contiguous with addresses */
    printf("  Address proof:\n");
    printf("    &m[0][0]=%p\n", (void*)&m[0][0]);
    printf("    &m[0][1]=%p  (+%ld)\n", (void*)&m[0][1], (char*)&m[0][1]-(char*)&m[0][0]);
    printf("    &m[1][0]=%p  (+%ld from m[0][0])\n",
           (void*)&m[1][0], (char*)&m[1][0]-(char*)&m[0][0]);

    /* Element address formula */
    printf("\n  Element m[i][j] address = base + (i*COLS + j) * sizeof(int)\n");
    int ROWS=3, COLS=4;
    for (int i=0;i<ROWS;i++) for (int j=0;j<COLS;j++) {
        int *formula = (int*)m + i*COLS + j;
        if (i==1 && j==2) {  /* spot check m[1][2] */
            printf("  m[1][2]=%d  via formula=%d  match=%s\n",
                   m[1][2], *formula, m[1][2]==*formula?"YES":"NO");
        }
    }

    /* Pointer to row */
    int (*row_ptr)[4] = m;     /* pointer to row — type int(*)[4] */
    printf("\n  Via row pointer int(*row_ptr)[4] = m:\n");
    for (int i = 0; i < 3; i++) {
        printf("    row %d: ", i);
        for (int j = 0; j < 4; j++) printf("%3d ", row_ptr[i][j]);
        printf("\n");
    }

    /* Pointer arithmetic on 2D array */
    int *flat = (int*)m;   /* treat 2D as flat 1D */
    printf("\n  Flat traversal (cast to int*):\n  ");
    for (int i = 0; i < 12; i++) printf("%3d ", flat[i]);
    printf("\n");

    /*
     * INTERVIEW NOTES:
     *   - C 2D arrays: ROW-MAJOR — entire row0 then row1 then row2
     *   - m[i][j] address = base + (i*COLS + j) * sizeof(element)
     *   - Passing to function: void f(int arr[][4], int rows) — COLS mandatory
     *   - int (*p)[4] = m — pointer to row, p+1 skips entire row (16 bytes)
     *   - (int*)m — flat view of 2D array — valid in C
     */
}

/* =============================================================================
 * A6. DYNAMIC ARRAY (malloc + realloc GROW PATTERN) — HIGH
 * =========================================================================== */

typedef struct {
    int   *data;
    int    size;      /* current number of elements */
    int    capacity;  /* allocated slots */
} DynArray;

void dynarray_init(DynArray *a, int initial_cap)
{
    a->data     = malloc(initial_cap * sizeof(int));
    a->size     = 0;
    a->capacity = a->data ? initial_cap : 0;
}

int dynarray_push(DynArray *a, int val)
{
    if (a->size == a->capacity) {
        /* Double capacity when full — amortized O(1) push */
        int new_cap   = a->capacity ? a->capacity * 2 : 1;
        int *tmp      = realloc(a->data, new_cap * sizeof(int));
        if (!tmp) return -1;        /* realloc failed — original intact */
        a->data     = tmp;
        a->capacity = new_cap;
        printf("    [realloc: cap %d → %d]\n", new_cap/2, new_cap);
    }
    a->data[a->size++] = val;
    return 0;
}

void dynarray_free(DynArray *a)
{
    free(a->data);
    a->data = NULL;
    a->size = a->capacity = 0;
}

void a6_dynamic_array(void)
{
    sec("A6 [HIGH] DYNAMIC ARRAY (malloc + realloc GROW PATTERN)");

    DynArray arr;
    dynarray_init(&arr, 2);   /* start with capacity 2 */
    printf("  Initial capacity: %d\n", arr.capacity);

    for (int i = 1; i <= 8; i++) {
        dynarray_push(&arr, i * 10);
    }

    printf("  Final: size=%d capacity=%d  data: ", arr.size, arr.capacity);
    for (int i = 0; i < arr.size; i++) printf("%d ", arr.data[i]);
    printf("\n");

    dynarray_free(&arr);

    printf("\n  Growth strategy:\n");
    printf("    Doubling capacity: amortized O(1) per push\n");
    printf("    Total copies across N pushes = N (geometric series)\n");
    printf("    NEVER: realloc one element at a time — O(N²) copies\n");
    printf("    Safe realloc: always use tmp ptr, not original ptr\n");

    /*
     * INTERVIEW NOTES:
     *   - Double capacity on full: amortized O(1) push — standard technique
     *   - realloc with tmp ptr: if realloc fails, original data preserved
     *   - Never: a->data = realloc(a->data, n) — on failure a->data=NULL, leak
     *   - Track both size (used) and capacity (allocated) separately
     *   - free + NULL the pointer after free — prevent dangling
     */
}

/* =============================================================================
 * A7. VLA — VARIABLE LENGTH ARRAY (C99) — HIGH
 * =========================================================================== */

void vla_matrix_multiply(int n, int a[n][n], int b[n][n], int result[n][n])
{
    /* n is a runtime value — VLA size determined at call time */
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            result[i][j] = 0;
            for (int k = 0; k < n; k++)
                result[i][j] += a[i][k] * b[k][j];
        }
}

void a7_vla(void)
{
    sec("A7 [HIGH] VLA — VARIABLE LENGTH ARRAY (C99)");

    int n = 3;   /* runtime value */

    /* Stack-allocated VLA — size known only at runtime */
    int matrix[n][n];   /* VLA — illegal in C89, valid C99 */
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            matrix[i][j] = i*n + j + 1;

    printf("  VLA int matrix[%d][%d] on STACK:\n", n, n);
    for (int i = 0; i < n; i++) {
        printf("    ");
        for (int j = 0; j < n; j++) printf("%3d ", matrix[i][j]);
        printf("\n");
    }
    printf("  sizeof(matrix) = %zu  (computed at RUNTIME for VLA)\n",
           sizeof(matrix));

    /* VLA in function */
    int identity[n][n];
    memset(identity, 0, sizeof(identity));
    for (int i = 0; i < n; i++) identity[i][i] = 1;

    int result[n][n];
    vla_matrix_multiply(n, matrix, identity, result);
    printf("  matrix * identity = matrix: match=%s\n",
           memcmp(matrix,result,sizeof(matrix))==0 ? "YES":"NO");

    printf("\n  VLA rules:\n");
    printf("    C99: mandatory. C11: optional (check __STDC_NO_VLA__)\n");
    printf("    Allocated on STACK — stack overflow risk for large n\n");
    printf("    sizeof(VLA) is a RUNTIME expression (unlike normal arrays)\n");
    printf("    Cannot initialize VLA: int a[n] = {0}; — ILLEGAL\n");
    printf("    VLA cannot be static or global — stack only\n");
    printf("    Alternative for large arrays: malloc on heap\n");

    /*
     * INTERVIEW NOTES:
     *   - VLA: C99 mandatory, C11 optional — Microsoft MSVC does NOT support it
     *   - MISRA C / safety standards often PROHIBIT VLA (stack overflow risk)
     *   - sizeof(vla) evaluated at runtime — unlike compile-time for normal arrays
     *   - Linux kernel does not use VLA (banned in kernel since v4.20)
     *   - Use for: small matrices/buffers where size varies, temporary workspace
     */
}

/* =============================================================================
 * A8. ARRAY BOUNDS — NO BOUNDS CHECKING IN C (UB DEMO) — HIGH
 * =========================================================================== */

void a8_array_bounds(void)
{
    sec("A8 [HIGH] ARRAY BOUNDS — NO BOUNDS CHECKING IN C");

    int arr[5] = {10, 20, 30, 40, 50};

    printf("  Valid: arr[0..4]:\n  ");
    for (int i = 0; i < 5; i++) printf("%d ", arr[i]);
    printf("\n");

    printf("\n  C provides ZERO bounds checking:\n");
    printf("    arr[-1]  = reading before array = UB (no error/exception)\n");
    printf("    arr[5]   = reading past end      = UB (may read garbage)\n");
    printf("    arr[5]=99= writing past end      = UB (heap/stack corruption)\n");

    printf("\n  Why dangerous:\n");
    printf("    Stack adjacent vars may be corrupted silently\n");
    printf("    Heap metadata corrupted → crash elsewhere\n");
    printf("    Security exploit: attacker overwrites return address\n");

    printf("\n  Safe access patterns:\n");
    printf("    Always check: if (i >= 0 && i < n) arr[i];\n");
    printf("    Use ARRAY_SIZE macro: for(i=0; i<ARRAY_SIZE(arr); i++)\n");
    printf("    Compile with: -fsanitize=address (AddressSanitizer)\n");
    printf("    Or:           -fsanitize=bounds  (Bounds Sanitizer)\n");

    /* Safe bounds check pattern */
    int idx = 3;
    int n   = 5;
    if (idx >= 0 && idx < n) {
        printf("\n  Safe access arr[%d] = %d\n", idx, arr[idx]);
    }

    /*
     * INTERVIEW NOTES:
     *   - C has NO bounds checking — performance design choice
     *   - Out-of-bounds access = Undefined Behavior (not a defined error)
     *   - -fsanitize=address: runtime bounds checking (debug only, ~2x slower)
     *   - valgrind: detects invalid reads/writes
     *   - Common bugs: off-by-one (i<=n instead of i<n), loop going too far
     *   - strcmp/strcpy also have no bounds — prefer strncmp/strncpy/strlcpy
     */
}

/* =============================================================================
 * A9. STRING ARRAYS — char*[] vs char[][] — MEDIUM
 * =========================================================================== */

void a9_string_arrays(void)
{
    sec("A9 [MEDIUM] STRING ARRAYS — char*[] vs char[][] (argv pattern)");

    /* char *arr[] — array of pointers to string literals (read-only) */
    const char *days_ptr[] = {
        "Monday", "Tuesday", "Wednesday", "Thursday",
        "Friday", "Saturday", "Sunday"
    };
    /* days_ptr[0] → literal "Monday" in read-only .rodata */
    /* sizeof(days_ptr) = 7 * 8 = 56 (7 pointers) */

    /* char arr[][] — 2D array, ALL strings same length, MODIFIABLE copies */
    char days_arr[][12] = {   /* 12 = longest + 1 for \0 */
        "Monday", "Tuesday", "Wednesday", "Thursday",
        "Friday", "Saturday", "Sunday"
    };
    /* sizeof(days_arr) = 7 * 12 = 84 bytes — all on stack, all modifiable */

    printf("  char *days_ptr[]  sizeof=%zu  (7 pointers × 8)\n",
           sizeof(days_ptr));
    printf("  char days_arr[][12] sizeof=%zu  (7 × 12 fixed slots)\n",
           sizeof(days_arr));
    sp();

    /* Modify days_arr — allowed (stack copy) */
    days_arr[0][0] = 'm';   /* lowercase Monday */
    printf("  days_arr[0] modified: %s\n", days_arr[0]);

    /* days_ptr[0][0] = 'm'; — UB! points to read-only literal */

    /* argv-style — simulate main(int argc, char *argv[]) */
    const char *argv_sim[] = { "program", "--verbose", "--output", "file.txt", NULL };
    int argc = 0;
    while (argv_sim[argc]) argc++;
    printf("\n  Simulated argv (%d args):\n", argc);
    for (int i = 0; i < argc; i++)
        printf("    argv[%d] = \"%s\"\n", i, argv_sim[i]);

    printf("\n  Comparison:\n");
    printf("    char *p[]     : array of ptrs → different lengths, read-only literals\n");
    printf("    char a[][N]   : 2D array → same max length N, modifiable copies\n");
    printf("    char **pp     : ptr to ptr → can reassign rows, dynamic\n");

    /*
     * INTERVIEW NOTES:
     *   - argv is char *argv[] — array of pointers to modifiable strings
     *   - char *a[] = {"x","y"} — literals are read-only; pointers can change
     *   - char a[][5] = {"x","y"} — copies on stack; modifiable; fixed col width
     *   - char **pp: neither pointer is const — fully mutable
     *   - argv[argc] is guaranteed NULL — safe sentinel termination
     */
}

/* =============================================================================
 * A10. ARRAY INITIALIZATION TRICKS — MEDIUM
 * =========================================================================== */

void a10_array_init_tricks(void)
{
    sec("A10 [MEDIUM] ARRAY INITIALIZATION TRICKS");

    sub("Partial init — rest zeroed");
    int a[10] = {1, 2, 3};   /* a[3..9] = 0 automatically */
    printf("  int a[10]={1,2,3}: ");
    for(int i=0;i<10;i++) printf("%d ",a[i]);
    printf("\n");

    sub("{0} — zero everything");
    int b[10] = {0};   /* all elements = 0 */
    printf("  int b[10]={0}:     ");
    for(int i=0;i<10;i++) printf("%d ",b[i]);
    printf("\n");

    sub("Designated initializer — set specific indices");
    int c[10] = { [0]=1, [5]=5, [9]=9 };   /* rest = 0 */
    printf("  [0]=1,[5]=5,[9]=9: ");
    for(int i=0;i<10;i++) printf("%d ",c[i]);
    printf("\n");

    sub("Infer size from initializer");
    int d[] = {10, 20, 30, 40, 50};   /* compiler counts: size=5 */
    printf("  int d[]={...}: size=%zu\n", sizeof(d)/sizeof(d[0]));

    sub("String array initialization");
    char str1[10] = "Hello";     /* remaining bytes = 0 */
    char str2[10] = {'H','i',0}; /* explicit null terminator */
    char str3[]   = "World";     /* size inferred = 6 */
    printf("  str1='%s' str2='%s' str3='%s' sizeof(str3)=%zu\n",
           str1, str2, str3, sizeof(str3));

    sub("memset patterns");
    int arr[8];
    memset(arr, 0, sizeof(arr));     /* zero-fill */
    memset(arr, 0xFF, sizeof(arr));  /* fill all bits 1 */
    printf("  After memset(0xFF): arr[0]=0x%X (all bits set)\n", (unsigned)arr[0]);
    memset(arr, 0, sizeof(arr));
    printf("  After memset(0):    arr[0]=%d\n", arr[0]);

    printf("\n  memset(arr, 1, n): NOT arr[i]=1 — sets each BYTE to 1\n");
    printf("    int arr[2]; memset(arr,1,8) → arr[0]=0x01010101=16843009\n");

    /*
     * INTERVIEW NOTES:
     *   - Partial init: any initialization zeros the rest — guaranteed by C
     *   - {0} recommended for zero-init (may be optimized to memset)
     *   - Designated init for sparse arrays — clear which index gets what value
     *   - memset(arr,0,...): OK for integers (0=all zero bits)
     *   - memset(arr,1,...): fills BYTES not elements — gives 0x01010101 per int
     *   - memset for float/pointer: only 0 is guaranteed correct (NULL=0 on most)
     */
}

/* =============================================================================
 * A11. MULTI-DIM ARRAY PASSING TO FUNCTIONS — MEDIUM
 * =========================================================================== */

/* 2D: first dim optional, all others mandatory */
void print_2d(int rows, int cols, int arr[rows][cols])   /* VLA style — C99 */
{
    for (int i=0; i<rows; i++) {
        printf("    ");
        for (int j=0; j<cols; j++) printf("%3d ", arr[i][j]);
        printf("\n");
    }
}

/* Fixed cols — classic style */
void print_2d_fixed(int arr[][4], int rows)
{
    for (int i=0;i<rows;i++) {
        printf("    ");
        for(int j=0;j<4;j++) printf("%3d ",arr[i][j]);
        printf("\n");
    }
}

/* 3D — all dims except first mandatory */
void print_3d(int a[][2][3], int layers)
{
    for (int i=0;i<layers;i++) {
        printf("    Layer %d: ",i);
        for(int j=0;j<2;j++) for(int k=0;k<3;k++) printf("%d ",a[i][j][k]);
        printf("\n");
    }
}

void a11_multidim_passing(void)
{
    sec("A11 [MEDIUM] MULTI-DIM ARRAY PASSING TO FUNCTIONS");

    int m[3][4] = {{1,2,3,4},{5,6,7,8},{9,10,11,12}};

    printf("  2D VLA-style (C99): void f(int rows, int cols, int arr[rows][cols])\n");
    print_2d(3, 4, m);

    printf("  2D fixed cols:      void f(int arr[][4], int rows)\n");
    print_2d_fixed(m, 3);

    int cube[2][2][3] = {{{1,2,3},{4,5,6}},{{7,8,9},{10,11,12}}};
    printf("  3D: void f(int a[][2][3], int layers)\n");
    print_3d(cube, 2);

    printf("\n  Rules:\n");
    printf("    void f(int arr[][C], int rows)  — C must be compile-time constant\n");
    printf("    void f(int R, int C, int arr[R][C]) — VLA C99, both dims as args\n");
    printf("    Outermost dim: ALWAYS omitted (it's the 'count of rows' passed as n)\n");
    printf("    Inner dims: MANDATORY (compiler needs them for address arithmetic)\n");
    printf("    Wrong: void f(int **arr) for int arr[R][C] — NOT interchangeable\n");

    /*
     * INTERVIEW NOTES:
     *   - int arr[][4]: compiler needs [4] to compute arr[i][j] = base + i*4*4 + j*4
     *   - Without inner dims, compiler cannot compute element address
     *   - int ** and int[][C] are DIFFERENT types — cannot interchange
     *   - VLA style (int arr[rows][cols]) cleanest for variable dims
     *   - Fortran and MATLAB use COLUMN-major — C is ROW-major (important!)
     */
}

/* =============================================================================
 * A12. const ARRAY vs ARRAY OF const — MEDIUM
 * =========================================================================== */

void a12_const_array(void)
{
    sec("A12 [MEDIUM] const ARRAY vs ARRAY OF const");

    sub("const int arr[] — elements are const, cannot modify values");
    const int grades[] = {90, 85, 92, 78, 95};
    /* grades[0] = 100;  // COMPILE ERROR — cannot modify const element */
    printf("  const int grades[]: %d %d %d %d %d\n",
           grades[0],grades[1],grades[2],grades[3],grades[4]);

    sub("const int *arr as parameter — read-only view of passed array");
    /* void f(const int *arr, int n) — function promises not to modify arr */
    /* Can be called with both const and non-const arrays */

    sub("int * const arr — const pointer (array), elements modifiable");
    /* Rarely used for arrays — const pointer = cannot reassign arr, but arr[i] = ok */
    int data[] = {1, 2, 3};
    int * const fixed_ptr = data;   /* pointer cannot be reassigned */
    fixed_ptr[0] = 99;              /* but values CAN be changed */
    printf("  int * const: fixed_ptr[0]=%d (value changed, ptr fixed)\n", fixed_ptr[0]);

    sub("Read-only lookup table — common embedded pattern");
    static const char *const status_names[] = {
        "IDLE", "RUNNING", "PAUSED", "ERROR", "DONE"
    };
    /* status_names[i]    — cannot reassign pointers */
    /* status_names[i][j] — cannot modify string characters */
    printf("  const char *const table: ");
    for (int i = 0; i < 5; i++) printf("%s ", status_names[i]);
    printf("\n");

    printf("\n  Summary:\n");
    printf("    const int a[]     : elements const, perfect for ROM tables\n");
    printf("    const int *p      : read-only view, pointer movable\n");
    printf("    int * const p     : fixed pointer, values mutable\n");
    printf("    const char *const : both ptr and chars fully locked\n");

    /*
     * INTERVIEW NOTES:
     *   - const arrays: compiler may put in .rodata (flash on embedded)
     *   - static const lookup tables: evaluated at compile time, no RAM usage
     *   - const int *p vs const int arr[]: as param — pointer more explicit
     *   - const char *const names[]: fullest protection for string tables
     *   - Embedded: const global arrays → stored in flash, saves RAM
     */
}

/* =============================================================================
 * A13. ARRAY OF FUNCTION POINTERS (DISPATCH TABLE) — LOW
 * =========================================================================== */

typedef void (*HandlerFn)(int);

void handler_idle   (int v) { printf("  IDLE handler    val=%d\n", v); }
void handler_running(int v) { printf("  RUNNING handler val=%d\n", v); }
void handler_error  (int v) { printf("  ERROR handler   val=%d — resetting\n", v); }
void handler_done   (int v) { printf("  DONE handler    val=%d — cleaning up\n", v); }

void a13_array_fn_pointers(void)
{
    sec("A13 [LOW] ARRAY OF FUNCTION POINTERS (DISPATCH / STATE TABLE)");

    /* State machine dispatch table */
    HandlerFn dispatch[] = {
        handler_idle,
        handler_running,
        handler_error,
        handler_done
    };
    int n_states = sizeof(dispatch)/sizeof(dispatch[0]);

    printf("  State machine via dispatch table:\n");
    for (int state = 0; state < n_states; state++) {
        dispatch[state](state * 10);   /* call handler for each state */
    }

    /* vs switch-case — same logic but switch grows, table stays constant */
    printf("\n  dispatch[state](val) vs switch(state){case ...:}\n");
    printf("    Table: O(1) dispatch, easy to extend, data-driven\n");
    printf("    Switch: slightly more readable, compiler may optimize to table anyway\n");

    /*
     * INTERVIEW NOTES:
     *   - Dispatch table: O(1) lookup vs O(n) switch
     *   - Easy to add new handlers without modifying dispatch logic
     *   - Used in: state machines, command parsers, plugin systems
     *   - NULL entry can mark invalid/unimplemented states
     *   - Combine with enum: enum State{IDLE,RUNNING,...}; dispatch[IDLE](val);
     */
}

/* =============================================================================
 * A14. COMPOUND LITERALS AS ARRAYS — LOW
 * =========================================================================== */

int sum_n(const int *arr, int n)
{
    int s=0; for(int i=0;i<n;i++) s+=arr[i]; return s;
}

void a14_compound_literals(void)
{
    sec("A14 [LOW] COMPOUND LITERALS AS ARRAYS (C99)");

    /* Compound literal — unnamed temporary array */
    /* Lifetime: current block scope */
    int total = sum_n((int[]){1, 2, 3, 4, 5}, 5);
    printf("  sum of (int[]){1,2,3,4,5} = %d\n", total);

    /* Useful for: one-time use arrays, test values, default parameters */
    printf("  sum of (int[]){10,20,30} = %d\n",
           sum_n((int[]){10, 20, 30}, 3));

    /* Compound literal struct */
    typedef struct { int x, y; } Pt;
    Pt p = (Pt){ .x=3, .y=4 };   /* compound literal struct */
    printf("  (Pt){3,4}: x=%d y=%d\n", p.x, p.y);

    /* Modifiable — unlike string literals */
    int *tmp = (int[]){5, 10, 15};
    tmp[0] = 99;
    printf("  Compound literal is modifiable: tmp[0]=%d\n", tmp[0]);

    printf("\n  Compound literal lifetime = enclosing block scope\n");
    printf("  Storing pointer to compound literal after scope exits = dangling\n");

    /*
     * INTERVIEW NOTES:
     *   - C99 feature — not in C89
     *   - (Type){...} — creates unnamed object of given type
     *   - Lifetime: current block scope (like a local variable)
     *   - Modifiable unlike string literals
     *   - Useful for passing temporary arrays/structs to functions cleanly
     *   - Do NOT take address and use after scope exits — dangling
     */
}

/* =============================================================================
 * A15. restrict WITH ARRAY PARAMETERS — LOW
 * =========================================================================== */

/* Without restrict — compiler must assume overlap */
void copy_no_restrict(int *dst, const int *src, int n)
{
    for (int i=0; i<n; i++) dst[i] = src[i];
}

/* With restrict — compiler knows no overlap — can vectorize */
void copy_restrict(int * restrict dst, const int * restrict src, int n)
{
    for (int i=0; i<n; i++) dst[i] = src[i];
    /* Same as memcpy semantics — undefined if dst and src overlap */
}

void a15_restrict_arrays(void)
{
    sec("A15 [LOW] restrict WITH ARRAY PARAMETERS");

    int src[] = {1, 2, 3, 4, 5, 6, 7, 8};
    int dst[8] = {0};

    copy_no_restrict(dst, src, 8);
    printf("  No restrict: ");
    for(int i=0;i<8;i++) printf("%d ",dst[i]);
    printf("\n");

    memset(dst, 0, sizeof(dst));
    copy_restrict(dst, src, 8);
    printf("  Restrict   : ");
    for(int i=0;i<8;i++) printf("%d ",dst[i]);
    printf("\n");

    printf("\n  Output identical — difference is ONLY in optimization:\n");
    printf("    Without restrict: compiler re-reads src[i] after each dst[i] write\n");
    printf("    With restrict   : compiler can vectorize — load all, store all\n");
    printf("    Breaking restrict promise (aliased ptrs) = UB with -O2\n");
    printf("    memcpy declaration: void *memcpy(void *restrict, const void *restrict, size_t)\n");

    /*
     * INTERVIEW NOTES:
     *   - restrict = programmer's promise of no aliasing
     *   - Enables SIMD vectorization for array loops
     *   - C99 only — use __restrict in C++ (non-standard extension)
     *   - Violating restrict = UB — compiler generates incorrect code
     *   - memcpy, strcpy, sprintf all use restrict in their signatures
     */
}

/* ============================================================================
 * MAIN
 * ========================================================================== */
int main(void)
{
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║   STRUCTURES & ARRAYS — COMPLETE CONCEPT REFERENCE      ║\n");
    printf("║   Aravinth K — Interview Preparation                    ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");

    /* ── STRUCTURES ──────────────────────────────────────────────────── */
    printf("\n\n▶▶▶  STRUCTURES  ◀◀◀\n");

    /* HIGH */
    s1_declaration_init();
    s2_padding_alignment();
    s3_struct_packing();
    s4_pass_by_value_vs_pointer();
    s5_nested_struct();
    s6_bit_fields();
    s7_self_referential();
    s8_struct_function_pointers();
    s9_typedef_patterns();
    s10_flexible_array_member();

    /* MEDIUM */
    s11_shallow_vs_deep_copy();
    s12_aos_vs_soa();
    s13_struct_return_value();
    s14_anonymous_struct();
    s15_offsetof_layout();

    /* LOW */
    s16_struct_comparison();
    s17_struct_serialization();
    s18_bit_field_advanced();

    /* ── ARRAYS ──────────────────────────────────────────────────────── */
    printf("\n\n▶▶▶  ARRAYS  ◀◀◀\n");

    /* HIGH */
    a1_array_decay();
    a2_sizeof_array_vs_pointer();
    a3_passing_arrays();
    a4_array_of_structs();
    a5_2d_array_layout();
    a6_dynamic_array();
    a7_vla();
    a8_array_bounds();

    /* MEDIUM */
    a9_string_arrays();
    a10_array_init_tricks();
    a11_multidim_passing();
    a12_const_array();

    /* LOW */
    a13_array_fn_pointers();
    a14_compound_literals();
    a15_restrict_arrays();

    printf("\n╔══════════════════════════════════════════════════════════╗\n");
    printf("║  ALL DEMOS COMPLETE                                      ║\n");
    printf("║  Compile: gcc -O0 -Wall -Wextra -std=c11 -o sa \\        ║\n");
    printf("║           struct_array_concepts.c                        ║\n");
    printf("║  Check  : valgrind --leak-check=full ./sa                ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    return 0;
}
