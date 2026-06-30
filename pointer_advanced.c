/*
 * =============================================================================
 * FILE    : pointer_advanced.c
 * AUTHOR  : Aravinth K
 * PURPOSE : Advanced pointer concepts — High & Medium Priority
 *           Compile: gcc -O2 -Wall -Wextra -std=c11 -o pointer_advanced pointer_advanced.c
 * =============================================================================
 *
 * HIGH PRIORITY:
 *  1.  restrict pointer
 *  2.  const in function parameters (4 combinations)
 *  3.  NULL vs 0 vs (void*)0
 *  4.  Opaque Pointer (incomplete type / handle pattern)
 *  5.  sizeof pointer vs sizeof pointed-to type
 *  6.  near/far pointer (conceptual — 16-bit legacy)
 *
 * MEDIUM PRIORITY:
 *  7.  Function pointer in struct (vtable / driver API pattern)
 *  8.  Pointer aliasing & strict aliasing rule
 *  9.  memcpy vs pointer cast for type punning
 * 10.  Stack vs heap pointer lifetime rules
 * 11.  Pointer to function returning pointer
 * 12.  offsetof macro + CONTAINER_OF pattern
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>   /* offsetof */

/* ============================================================================
 * HELPER
 * ========================================================================== */
static void section(const char *title)
{
    printf("\n");
    printf("=============================================================\n");
    printf("  %s\n", title);
    printf("=============================================================\n");
}

static void subsection(const char *title)
{
    printf("\n  --- %s ---\n", title);
}

/* ============================================================================
 * TOPIC 1: restrict POINTER
 *
 * Definition:
 *   'restrict' is a PROMISE to the compiler by the programmer:
 *   "For the lifetime of this pointer, no other pointer will access
 *    the same memory. There is NO ALIASING."
 *
 * Effect:
 *   Compiler can generate faster code — it doesn't have to re-read memory
 *   after every write because it knows no other pointer can change it.
 *
 * Where you see it:
 *   Standard library: memcpy, strcpy, sprintf all use restrict.
 *   void *memcpy(void * restrict dst, const void * restrict src, size_t n);
 *
 * KEY RULE:
 *   If you lie (two restrict ptrs DO alias) → Undefined Behavior.
 *   restrict is ONLY a hint/promise — compiler does not verify it.
 * ========================================================================== */

/* Without restrict — compiler must assume a and b may alias (overlap) */
/* It must re-read a[i] after each write to b[i], just in case a==b   */
void add_arrays_no_restrict(int *a, int *b, int *result, int n)
{
    for (int i = 0; i < n; i++) {
        result[i] = a[i] + b[i];
        /* compiler cannot hoist a[i] or b[i] out of loop —
         * result might be the same array as a or b */
    }
}

/* With restrict — compiler KNOWS a, b, result don't overlap */
/* Can optimize: vectorize, hoist loads, reorder instructions freely  */
void add_arrays_restrict(int * restrict a,
                         int * restrict b,
                         int * restrict result,
                         int n)
{
    for (int i = 0; i < n; i++) {
        result[i] = a[i] + b[i];
        /* compiler can safely vectorize — knows no aliasing */
    }
}

/* restrict with const — most common real pattern (like memcpy) */
void my_memcpy(void * restrict dst, const void * restrict src, size_t n)
{
    /*
     * restrict: dst and src do NOT overlap — your promise.
     * const src: we won't modify source through this pointer.
     * This is exactly how the C standard declares memcpy.
     * Using memcpy on overlapping buffers = UB. Use memmove instead.
     */
    unsigned char       *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    while (n--) *d++ = *s++;
}

void demo_restrict_pointer(void)
{
    section("TOPIC 1: restrict POINTER");

    int a[]      = {1, 2, 3, 4, 5};
    int b[]      = {10, 20, 30, 40, 50};
    int result[5] = {0};

    add_arrays_no_restrict(a, b, result, 5);
    printf("No restrict: ");
    for (int i = 0; i < 5; i++) printf("%d ", result[i]);
    printf("\n");

    memset(result, 0, sizeof(result));

    add_arrays_restrict(a, b, result, 5);
    printf("Restrict   : ");
    for (int i = 0; i < 5; i++) printf("%d ", result[i]);
    printf("\n");
    /* Both produce identical output — difference is only in optimization */

    /* my_memcpy demonstration */
    char src[] = "Embedded Systems";
    char dst[32] = {0};
    my_memcpy(dst, src, strlen(src) + 1);
    printf("my_memcpy  : %s\n", dst);

    /*
     * INTERVIEW NOTES:
     *   - restrict is C99 — not in C89, not in C++ (some compilers add __restrict)
     *   - Violating restrict (aliased ptrs) = UB — compiler may generate wrong code
     *   - memcpy requires non-overlapping buffers (restrict). memmove handles overlap.
     *   - In embedded DSP code, restrict on array params can give 2-4x speedup
     *   - Compiler cannot check restrict — it trusts your promise
     */
}

/* ============================================================================
 * TOPIC 2: const IN FUNCTION PARAMETERS — ALL 4 COMBINATIONS
 *
 * This is one of the most common interview questions on const + pointers.
 *
 *  A) int *p               → can change *p AND p  (no protection)
 *  B) const int *p         → can NOT change *p, CAN change p
 *  C) int * const p        → CAN change *p, can NOT change p
 *  D) const int * const p  → can NOT change *p OR p (full protection)
 *
 * TRICK to remember: read right-to-left from variable name.
 *   "p is a [const] pointer [const] to int"
 * ========================================================================== */

/* A) No const — function can do anything: move ptr and modify value */
void variant_A(int *p, int n)
{
    /* can modify value AND move pointer */
    for (int i = 0; i < n; i++) {
        *p = *p * 2;   /* modify value at address */
        p++;           /* move pointer */
    }
}

/* B) const int *p — read-only data, pointer can move */
/* Use when: function should READ array but NOT modify it */
/* This is what printf, strlen, strcpy(src) use */
int sum_array(const int *p, int n)
{
    int total = 0;
    while (n--) {
        total += *p;   /* read OK */
        /* *p = 0;     // COMPILE ERROR — cannot modify through const int* */
        p++;           /* moving pointer is fine */
    }
    return total;
}

/* C) int * const p — pointer fixed, value modifiable */
/* Use when: function works on a fixed buffer, mustn't redirect pointer */
/* Less common in practice */
void fill_buffer(int * const p, int n, int val)
{
    for (int i = 0; i < n; i++) {
        p[i] = val;    /* modifying value — OK */
        /* p++;        // COMPILE ERROR — const pointer cannot move */
        /* p = NULL;   // COMPILE ERROR */
    }
}

/* D) const int * const p — maximum protection */
/* Use when: function only reads, and pointer itself must not change */
void print_first(const int * const p)
{
    printf("First element: %d\n", *p);
    /* *p = 0;   // COMPILE ERROR */
    /* p++;      // COMPILE ERROR */
}

void demo_const_in_params(void)
{
    section("TOPIC 2: const IN FUNCTION PARAMETERS (ALL 4 COMBINATIONS)");

    int arr[] = {5, 10, 15, 20, 25};
    int n = 5;

    subsection("A) int *p — no protection");
    variant_A(arr, n);
    printf("After x2 each: ");
    for (int i = 0; i < n; i++) printf("%d ", arr[i]);
    printf("\n");

    subsection("B) const int *p — read-only data, ptr can move");
    int arr2[] = {1, 2, 3, 4, 5};
    printf("Sum = %d\n", sum_array(arr2, 5));

    subsection("C) int * const p — fixed ptr, modifiable value");
    int buf[5] = {0};
    fill_buffer(buf, 5, 99);
    printf("Filled: ");
    for (int i = 0; i < 5; i++) printf("%d ", buf[i]);
    printf("\n");

    subsection("D) const int * const p — full protection");
    int val = 42;
    print_first(&val);

    printf("\n  Summary table:\n");
    printf("  Variant | Change *p | Change p  | Use case\n");
    printf("  --------|-----------|-----------|---------------------------\n");
    printf("  A: int*            |    YES    |    YES    | general in/out buffer\n");
    printf("  B: const int*      |    NO     |    YES    | read-only input (most common)\n");
    printf("  C: int* const      |    YES    |    NO     | fixed buffer, vary content\n");
    printf("  D: const int*const |    NO     |    NO     | fully immutable view\n");

    /*
     * INTERVIEW NOTES:
     *   - B is by far the most common: const int* for read-only parameters
     *   - If you see void f(const char *s) — function promises not to modify string
     *   - C is rarely needed in practice
     *   - Passing non-const pointer where const is expected: always OK (safe)
     *   - Passing const pointer where non-const expected: COMPILE WARNING/ERROR (unsafe)
     */
}

/* ============================================================================
 * TOPIC 3: NULL vs 0 vs (void*)0
 *
 * All three are used for null pointers but they are NOT identical in type.
 *
 * NULL      : implementation-defined null pointer constant. Typically defined
 *             as ((void*)0) or 0 or 0L depending on compiler/platform.
 * 0         : integer zero — becomes null pointer constant in pointer context
 * (void*)0  : explicitly typed null pointer (void pointer to address 0)
 *
 * KEY: In C, any of these can initialize a pointer to "null".
 *      In C++, only NULL or nullptr (C++11) — not (void*)0 for typed ptrs.
 * ========================================================================== */

void demo_null_vs_zero(void)
{
    section("TOPIC 3: NULL vs 0 vs (void*)0");

    /* All three create a null pointer — equivalent in C for pointers */
    int *p1 = NULL;       /* most readable — use this */
    int *p2 = 0;          /* valid in C — 0 in pointer context = null ptr */
    int *p3 = (void *)0;  /* explicit cast — what NULL usually expands to */

    printf("p1 (NULL)    : %s\n", p1 == NULL ? "null" : "non-null");
    printf("p2 (0)       : %s\n", p2 == NULL ? "null" : "non-null");
    printf("p3 ((void*)0): %s\n", p3 == NULL ? "null" : "non-null");
    printf("All equal    : %s\n", (p1==p2 && p2==p3) ? "YES" : "NO");

    /* NULL is typically defined as one of these: */
    /* #define NULL ((void*)0)   — most common in C  */
    /* #define NULL 0            — also valid in C    */
    /* #define NULL 0L           — on some platforms  */

    /* Size of NULL pointer — always pointer-width, not int-width */
    printf("sizeof(NULL) = %zu\n", sizeof(void *));  /* 8 on 64-bit */
    printf("sizeof(0)    = %zu\n", sizeof(0));        /* 4 — it's an int! */

    /* NULL pointer is NOT address zero on all platforms */
    /* On most modern OS: address 0 is unmapped — accessing it = segfault */
    /* On some embedded: address 0 IS valid memory (interrupt vectors!) */

    /* Safe null check patterns */
    int *p = NULL;
    if (p)          printf("non-null\n");   /* idiomatic C — treats ptr as bool */
    if (p != NULL)  printf("non-null\n");   /* explicit — clearer intent */
    if (!p)         printf("null ptr\n");   /* idiomatic null check */

    /*
     * INTERVIEW NOTES:
     *   - sizeof(NULL) != sizeof(0): NULL is void* (8B), 0 is int (4B)
     *   - In C, 0 converts to null pointer in pointer context — valid
     *   - In C++: (void*)0 cannot implicitly convert to int* — use nullptr
     *   - Null pointer != zero address on all platforms (embedded exception)
     *   - Dereferencing NULL = segfault on modern OS, UB by C standard
     *   - Always initialize pointers to NULL — never leave them wild
     */
}

/* ============================================================================
 * TOPIC 4: OPAQUE POINTER (INCOMPLETE TYPE / HANDLE PATTERN)
 *
 * Definition:
 *   A pointer to a struct whose definition is HIDDEN from the user.
 *   The user gets a handle (pointer) but cannot see or touch the internals.
 *
 * Why it matters:
 *   - Encapsulation in C (like private members in C++)
 *   - ABI stability: internals can change without recompiling user code
 *   - Used in: FILE* (stdio), pthread_t, device handles, RTOS objects
 *
 * Pattern:
 *   header.h  → declares:  typedef struct Sensor_ Sensor;  (incomplete type)
 *   sensor.c  → defines:   struct Sensor_ { int id; float val; };
 *   User only sees Sensor* — cannot access members directly
 * ========================================================================== */

/* ---- Simulating sensor.h (what user sees) ---- */
/* Forward declaration only — struct body is HIDDEN */
typedef struct Sensor_ Sensor;  /* incomplete type — size unknown to user */

/* API functions — user calls these, cannot touch internals */
Sensor *sensor_create(int id, float init_val);
void    sensor_update(Sensor *s, float new_val);
float   sensor_read(const Sensor *s);
int     sensor_get_id(const Sensor *s);
void    sensor_destroy(Sensor *s);

/* ---- Simulating sensor.c (implementation — hidden from user) ---- */
/* Full struct definition — ONLY visible in this translation unit */
struct Sensor_ {
    int   id;
    float value;
    int   read_count;   /* internal bookkeeping — user never knows this exists */
    float calibration;  /* internal — user cannot accidentally corrupt this */
};

Sensor *sensor_create(int id, float init_val)
{
    Sensor *s = (Sensor *)malloc(sizeof(struct Sensor_));
    if (!s) return NULL;
    s->id          = id;
    s->value       = init_val;
    s->read_count  = 0;
    s->calibration = 1.0f;
    return s;  /* return opaque handle — user sees only Sensor* */
}

void sensor_update(Sensor *s, float new_val)
{
    if (!s) return;
    s->value = new_val * s->calibration;
}

float sensor_read(const Sensor *s)
{
    if (!s) return -1.0f;
    /* cast away const to update internal count — common but be careful */
    ((Sensor *)s)->read_count++;
    return s->value;
}

int sensor_get_id(const Sensor *s)
{
    return s ? s->id : -1;
}

void sensor_destroy(Sensor *s)
{
    if (s) free(s);
    /* s = NULL here only affects local copy — caller must null their ptr */
}

void demo_opaque_pointer(void)
{
    section("TOPIC 4: OPAQUE POINTER (INCOMPLETE TYPE / HANDLE PATTERN)");

    /* User code — only knows Sensor* exists, not what's inside */
    Sensor *temp_sensor = sensor_create(1, 25.0f);
    Sensor *pres_sensor = sensor_create(2, 101.3f);

    if (!temp_sensor || !pres_sensor) {
        printf("Sensor creation failed\n");
        return;
    }

    printf("Sensor ID: %d  Initial reading: %.2f\n",
           sensor_get_id(temp_sensor), sensor_read(temp_sensor));

    sensor_update(temp_sensor, 27.5f);
    printf("After update: %.2f\n", sensor_read(temp_sensor));

    printf("Pressure sensor ID: %d  Val: %.2f\n",
           sensor_get_id(pres_sensor), sensor_read(pres_sensor));

    /* User CANNOT do this — struct internals are hidden: */
    /* temp_sensor->value = 100.0f;   // COMPILE ERROR: incomplete type */
    /* temp_sensor->id    = 99;       // COMPILE ERROR */
    /* sizeof(*temp_sensor);          // COMPILE ERROR — size unknown */

    sensor_destroy(temp_sensor);
    temp_sensor = NULL;   /* null after destroy — prevent dangling */
    sensor_destroy(pres_sensor);
    pres_sensor = NULL;

    printf("Both sensors destroyed safely\n");

    /*
     * INTERVIEW NOTES:
     *   - FILE* in stdio.h is the classic opaque pointer — you never see FILE internals
     *   - pthread_t, HANDLE (Windows), sd_t (Zephyr) — all opaque handles
     *   - Forward declaration: struct Foo; creates incomplete type
     *   - Can declare Foo* (pointer to incomplete type) — size of ptr is always known
     *   - Cannot: sizeof(Foo), Foo obj, access Foo->member without full definition
     *   - Enables information hiding, ABI stability, safer APIs
     */
}

/* ============================================================================
 * TOPIC 5: sizeof POINTER vs sizeof POINTED-TO TYPE
 *
 * Classic interview trap: sizeof(ptr) always gives POINTER SIZE (4 or 8),
 * NOT the size of what it points to.
 *
 * sizeof(T*)   = 4 (32-bit) or 8 (64-bit) — ALL pointer types same size
 * sizeof(*ptr) = size of the type being pointed to
 * sizeof(arr)  = total array size in bytes (ONLY works on real array name)
 * ========================================================================== */

/* Array size calculation FAILS when array decays to pointer in function */
void show_sizeof_in_function(int *arr, int real_n)
{
    /* arr here is just a pointer — sizeof gives pointer size, NOT array size */
    printf("  Inside function: sizeof(arr)  = %zu  (ptr size — WRONG for array!)\n",
           sizeof(arr));
    printf("  Inside function: sizeof(*arr) = %zu  (size of one element)\n",
           sizeof(*arr));
    printf("  Correct count: use passed n = %d\n", real_n);
}

void demo_sizeof_pointer(void)
{
    section("TOPIC 5: sizeof POINTER vs sizeof POINTED-TO TYPE");

    /* All pointers — same size regardless of what they point to */
    char    *pc  = NULL;
    int     *pi  = NULL;
    double  *pd  = NULL;
    int   **ppi  = NULL;
    void    *pv  = NULL;

    printf("sizeof(char*)   = %zu\n", sizeof(pc));   /* 8 */
    printf("sizeof(int*)    = %zu\n", sizeof(pi));   /* 8 */
    printf("sizeof(double*) = %zu\n", sizeof(pd));   /* 8 */
    printf("sizeof(int**)   = %zu\n", sizeof(ppi));  /* 8 */
    printf("sizeof(void*)   = %zu\n", sizeof(pv));   /* 8 */
    printf("All pointers same size on this platform: %zu bytes\n\n", sizeof(void*));

    /* sizeof pointed-to type — different per type */
    printf("sizeof(*pc) = sizeof(char)   = %zu\n", sizeof(*pc));   /* 1 */
    printf("sizeof(*pi) = sizeof(int)    = %zu\n", sizeof(*pi));   /* 4 */
    printf("sizeof(*pd) = sizeof(double) = %zu\n", sizeof(*pd));   /* 8 */

    /* Array sizeof — works ONLY on real array name, not pointer */
    int arr[10];
    printf("\nsizeof(arr)      = %zu  (total: 10 * 4 = 40)\n", sizeof(arr));
    printf("sizeof(arr[0])   = %zu  (one element)\n", sizeof(arr[0]));
    printf("Elements in arr  = %zu  (sizeof(arr)/sizeof(arr[0]))\n",
           sizeof(arr) / sizeof(arr[0]));

    /* The decay trap — array passed to function loses size info */
    printf("\nArray passed to function:\n");
    show_sizeof_in_function(arr, 10);

    /* Struct pointer sizeof */
    typedef struct { int x; int y; double z; } Point3D;
    Point3D  obj = {1, 2, 3.0};
    Point3D *ptr = &obj;
    printf("\nsizeof(Point3D)  = %zu  (struct size with padding)\n", sizeof(Point3D));
    printf("sizeof(ptr)      = %zu  (just the pointer)\n", sizeof(ptr));
    printf("sizeof(*ptr)     = %zu  (struct size — same as sizeof(Point3D))\n",
           sizeof(*ptr));

    /*
     * INTERVIEW NOTES:
     *   - sizeof(ptr) = 8 always on 64-bit — does NOT tell you array/struct size
     *   - sizeof(*ptr) = sizeof the type pointed to — this is what you usually want
     *   - ARRAY_SIZE(arr) = sizeof(arr)/sizeof(arr[0]) — only valid on real array name
     *   - malloc pattern: malloc(n * sizeof(*ptr)) — type-agnostic, safer than sizeof(int)
     *   - sizeof is evaluated at COMPILE TIME — no runtime cost
     *   - sizeof does NOT evaluate its argument: sizeof(i++) — i is NOT incremented
     */
}

/* ============================================================================
 * TOPIC 6: near/far POINTER (CONCEPTUAL — 16-BIT LEGACY)
 *
 * This is a 16-bit segmented memory model concept (x86 real mode, 8051, etc.)
 * Not directly compilable on modern 64-bit systems but ASKED IN INTERVIEWS,
 * especially automotive (8051, RL78) and legacy embedded.
 *
 * 16-bit segmented model:
 *   Memory divided into 64KB segments.
 *   near ptr: 16-bit offset — only within current segment (fast, small)
 *   far  ptr: 32-bit (segment:offset) — anywhere in memory (slow, large)
 *   huge ptr: like far but normalized — arithmetic works across segments
 * ========================================================================== */

void demo_near_far_pointer(void)
{
    section("TOPIC 6: near/far POINTER (CONCEPTUAL — 16-BIT LEGACY)");

    printf("  This is a conceptual topic — not directly compilable on 64-bit.\n\n");

    printf("  MEMORY MODEL (16-bit x86 real mode / 8051 style):\n");
    printf("  ┌─────────────────────────────────────────────────────┐\n");
    printf("  │ Segment 0  [0x0000 - 0xFFFF]  64KB                 │\n");
    printf("  │ Segment 1  [0x1000 - 0x1FFFF] 64KB (overlapping)   │\n");
    printf("  │ ...                                                  │\n");
    printf("  │ Total addressable: 1MB (20-bit address bus)         │\n");
    printf("  └─────────────────────────────────────────────────────┘\n\n");

    printf("  near pointer:\n");
    printf("    int near *p;  // 16-bit offset only\n");
    printf("    Size  : 2 bytes\n");
    printf("    Range : current 64KB segment only\n");
    printf("    Speed : FAST — no segment register reload\n");
    printf("    Use   : stack vars, local data in same segment\n\n");

    printf("  far pointer:\n");
    printf("    int far *p;   // 32-bit: 16-bit segment + 16-bit offset\n");
    printf("    Size  : 4 bytes\n");
    printf("    Range : any segment (full 1MB on x86 real mode)\n");
    printf("    Speed : SLOWER — must load segment register\n");
    printf("    Use   : cross-segment access, large data models\n\n");

    printf("  huge pointer:\n");
    printf("    int huge *p;  // like far but normalized\n");
    printf("    Arithmetic works correctly across segment boundaries\n");
    printf("    far ptr arithmetic wraps within segment — HUGE does not\n\n");

    printf("  Modern relevance:\n");
    printf("    8051 microcontroller: near = internal RAM (fast)\n");
    printf("                          far  = external RAM / xdata (slower)\n");
    printf("    Keil compiler: __near, __far, __huge keywords\n");
    printf("    RL78 (Renesas): __near, __far pragmas\n");
    printf("    On modern 32/64-bit: flat memory model — no near/far distinction\n");

    /*
     * INTERVIEW NOTES:
     *   - near/far not in C standard — compiler extensions (__near, __far)
     *   - Asked in automotive embedded interviews (8051 based ECUs)
     *   - 8051: internal data (near) is faster than xdata (far)
     *   - Modern ARM Cortex-M: flat 32-bit address space — no near/far needed
     *   - If asked: explain segmented vs flat memory model
     */
}

/* ============================================================================
 * TOPIC 7: FUNCTION POINTER IN STRUCT (vtable / DRIVER API PATTERN)
 *
 * This is HOW C fakes object-oriented polymorphism.
 * Used in:
 *   - Linux kernel: struct file_operations, struct net_device_ops
 *   - Zephyr RTOS:  struct uart_driver_api, struct sensor_driver_api
 *   - HAL layers:   function tables for hardware abstraction
 *
 * Pattern: struct holds function pointers as "methods"
 *          Different "objects" have different function tables
 *          Caller uses same interface — different behavior per device
 * ========================================================================== */

/* Generic UART driver API — the "interface" (like an abstract class) */
typedef struct {
    int  (*init)  (void *dev, uint32_t baud);
    int  (*send)  (void *dev, const uint8_t *data, size_t len);
    int  (*recv)  (void *dev, uint8_t *buf, size_t len);
    void (*close) (void *dev);
} UartDriverAPI;

/* Simulated UART-A hardware context */
typedef struct {
    uint32_t     baud_rate;
    int          is_open;
    char         name[16];
} UartDevice;

/* UART-A implementation (imagine: real hardware registers behind these) */
static int uart_a_init(void *dev, uint32_t baud)
{
    UartDevice *d = (UartDevice *)dev;
    d->baud_rate  = baud;
    d->is_open    = 1;
    printf("  [UART-A] init  baud=%u\n", baud);
    return 0;
}

static int uart_a_send(void *dev, const uint8_t *data, size_t len)
{
    UartDevice *d = (UartDevice *)dev;
    if (!d->is_open) return -1;
    printf("  [UART-A] send  %zu bytes: \"%.*s\"\n", len, (int)len, data);
    return (int)len;
}

static int uart_a_recv(void *dev, uint8_t *buf, size_t len)
{
    (void)dev; (void)buf; (void)len;
    printf("  [UART-A] recv  (simulated: no data)\n");
    return 0;
}

static void uart_a_close(void *dev)
{
    UartDevice *d = (UartDevice *)dev;
    d->is_open = 0;
    printf("  [UART-A] close\n");
}

/* UART-A driver API table — the vtable */
static const UartDriverAPI uart_a_api = {
    .init  = uart_a_init,
    .send  = uart_a_send,
    .recv  = uart_a_recv,
    .close = uart_a_close,
};

/* UART-B — different implementation, same API interface */
static int uart_b_init(void *dev, uint32_t baud)
{
    UartDevice *d = (UartDevice *)dev;
    d->baud_rate  = baud;
    d->is_open    = 1;
    printf("  [UART-B] init  baud=%u (DMA-capable variant)\n", baud);
    return 0;
}

static int uart_b_send(void *dev, const uint8_t *data, size_t len)
{
    UartDevice *d = (UartDevice *)dev;
    if (!d->is_open) return -1;
    printf("  [UART-B] send  %zu bytes via DMA: \"%.*s\"\n", len, (int)len, data);
    return (int)len;
}

static int uart_b_recv(void *dev, uint8_t *buf, size_t len)
{
    (void)dev; (void)buf; (void)len;
    printf("  [UART-B] recv  (DMA channel, simulated)\n");
    return 0;
}

static void uart_b_close(void *dev)
{
    UartDevice *d = (UartDevice *)dev;
    d->is_open = 0;
    printf("  [UART-B] close (flushed DMA)\n");
}

static const UartDriverAPI uart_b_api = {
    .init  = uart_b_init,
    .send  = uart_b_send,
    .recv  = uart_b_recv,
    .close = uart_b_close,
};

/* Generic application code — works with ANY uart via the API pointer */
/* This is the power of vtable: caller doesn't know which UART it uses */
void app_send_message(void *dev, const UartDriverAPI *api,
                      uint32_t baud, const char *msg)
{
    api->init(dev, baud);                                          /* init  */
    api->send(dev, (const uint8_t *)msg, strlen(msg));            /* send  */
    api->recv(dev, NULL, 0);                                      /* recv  */
    api->close(dev);                                              /* close */
}

void demo_vtable_pattern(void)
{
    section("TOPIC 7: FUNCTION POINTER IN STRUCT (vtable / DRIVER API)");

    UartDevice dev_a = { 0, 0, "UART-A" };
    UartDevice dev_b = { 0, 0, "UART-B" };

    printf("Using UART-A:\n");
    /* Pass device + its API table — polymorphic call */
    app_send_message(&dev_a, &uart_a_api, 115200, "Hello UART-A");

    printf("\nUsing UART-B (same app code, different driver):\n");
    app_send_message(&dev_b, &uart_b_api, 921600, "Hello UART-B");

    /*
     * INTERVIEW NOTES:
     *   - This is exactly how Zephyr's device model works:
     *     struct device { const void *api; void *data; };
     *     uart_send(dev, buf, len) → dev->api->send(dev, buf, len)
     *   - Linux kernel: struct file_operations {read, write, open, ...}
     *     Every filesystem/driver provides its own function table
     *   - OOP equivalent: abstract base class with virtual functions
     *   - const UartDriverAPI — vtable is usually const (never changes at runtime)
     *   - void *dev — generic device context (like 'this' pointer in C++)
     */
}

/* ============================================================================
 * TOPIC 8: POINTER ALIASING & STRICT ALIASING RULE
 *
 * Strict Aliasing Rule (C99 §6.5):
 *   "An object shall have its stored value accessed only by an lvalue
 *    expression that has one of the following types: the compatible type,
 *    a qualified version, a signed/unsigned variant, char, or aggregate
 *    containing the above."
 *
 * SIMPLIFIED: Accessing the same memory through incompatible pointer types
 *             is UNDEFINED BEHAVIOR. Compiler ASSUMES no aliasing between
 *             incompatible types and may optimize accordingly.
 *
 * EXCEPTION: char* can alias ANYTHING (always legal to read bytes via char*)
 * ========================================================================== */

/* BAD — violates strict aliasing — compiler may generate WRONG output */
/* with -O2 optimizations enabled */
float alias_bad(float *f, int *i)
{
    *i = 0x3F800000;   /* IEEE 754 bits for 1.0f */
    return *f;
    /*
     * Compiler with strict aliasing sees: f is float*, i is int*
     * They CANNOT point to same memory (different types).
     * So compiler may cache *f from before the *i write.
     * Result: may return old *f value, not 1.0f — UB!
     * Compile with -fno-strict-aliasing to disable this optimization.
     */
}

/* GOOD — use char* to alias — char* is always allowed */
float alias_via_char(float *f)
{
    uint32_t bits = 0x3F800000;           /* IEEE 754 bits for 1.0f */
    char *fp = (char *)f;
    char *bp = (char *)&bits;
    /* char* can legally alias any type — copy bytes safely */
    for (size_t i = 0; i < sizeof(float); i++) fp[i] = bp[i];
    return *f;   /* now legally reads the updated value */
}

/* BEST — use memcpy for type punning — always correct, compiler optimizes */
float type_pun_via_memcpy(uint32_t bits)
{
    float result;
    memcpy(&result, &bits, sizeof(float));
    /* memcpy is always legal — no aliasing violation */
    /* With -O2, compiler turns this into a register move (zero overhead) */
    return result;
}

void demo_strict_aliasing(void)
{
    section("TOPIC 8: POINTER ALIASING & STRICT ALIASING RULE");

    subsection("Strict aliasing violation (BAD pattern — shown for learning)");
    float f = 0.0f;
    /* alias_bad(&f, (int*)&f) — UB: float* and int* alias same memory */
    printf("  Calling alias_bad — result is UNDEFINED BEHAVIOR with -O2\n");
    printf("  (not called here — showing the concept only)\n");

    subsection("Legal: char* aliasing (bytes access)");
    float x = 3.14f;
    unsigned char *bytes = (unsigned char *)&x;
    printf("  float 3.14f bytes: ");
    for (size_t i = 0; i < sizeof(float); i++) {
        printf("0x%02X ", bytes[i]);   /* legal — char* can alias anything */
    }
    printf("\n");

    subsection("Type punning via char* (legal)");
    float result_char = alias_via_char(&f);
    printf("  Via char* aliasing: %.6f (should be 1.0)\n", result_char);

    subsection("Type punning via memcpy (BEST — always legal + optimized)");
    float result_memcpy = type_pun_via_memcpy(0x3F800000);
    printf("  Via memcpy: %.6f (should be 1.0)\n", result_memcpy);

    uint32_t check_bits;
    float    check_val = 3.14f;
    memcpy(&check_bits, &check_val, sizeof(uint32_t));
    printf("  float 3.14 as uint32: 0x%08X\n", check_bits);  /* 0x4048F5C3 */

    printf("\n  Aliasing rule summary:\n");
    printf("    int*   ↔ float*  : ILLEGAL (different types)\n");
    printf("    int*   ↔ char*   : LEGAL   (char* can alias anything)\n");
    printf("    int*   ↔ int*    : LEGAL   (same type)\n");
    printf("    int*   ↔ uint32* : LEGAL   (signed/unsigned variant)\n");
    printf("    float* ↔ uint32* via memcpy : ALWAYS LEGAL\n");
    printf("    -fno-strict-aliasing : disables this optimization (use carefully)\n");

    /*
     * INTERVIEW NOTES:
     *   - Strict aliasing is C99 — allows compiler to assume int* and float*
     *     never point to same memory → aggressive optimization
     *   - Classic bug: casting float* to int* to inspect bits — UB with -O2
     *   - Safe alternatives: memcpy, union (C99 allows union type punning)
     *   - Linux kernel uses -fno-strict-aliasing to avoid dealing with this
     *   - char/unsigned char: the universal exception — can alias anything
     */
}

/* ============================================================================
 * TOPIC 9: memcpy vs POINTER CAST FOR TYPE PUNNING
 *
 * Type punning = reading the bit pattern of one type as another type.
 * Common need: inspect float bits, convert between wire-format integers, etc.
 *
 * WRONG way: cast pointer directly (violates strict aliasing → UB with -O2)
 * RIGHT way: memcpy (always correct, compiler optimizes to register ops)
 * ALSO OK  : union (C99 permits union type punning explicitly)
 * ========================================================================== */

void demo_type_punning(void)
{
    section("TOPIC 9: memcpy vs POINTER CAST FOR TYPE PUNNING");

    float f = -0.15625f;   /* a value with known IEEE 754 representation */

    subsection("WRONG: direct pointer cast — UB with strict aliasing");
    /* uint32_t bad = *(uint32_t *)&f;  // UB — violates strict aliasing */
    printf("  Direct cast: UNDEFINED BEHAVIOR with optimizations — NOT shown\n");

    subsection("RIGHT: memcpy — always correct");
    uint32_t bits_memcpy;
    memcpy(&bits_memcpy, &f, sizeof(uint32_t));
    printf("  memcpy: float %.5f → 0x%08X\n", f, bits_memcpy);

    /* Reverse: bits back to float */
    uint32_t src_bits = 0xBE200000;   /* -0.15625 in IEEE 754 */
    float    recovered;
    memcpy(&recovered, &src_bits, sizeof(float));
    printf("  memcpy: 0x%08X → float %.5f\n", src_bits, recovered);

    subsection("ALSO RIGHT: union type punning (C99 explicit allowance)");
    union { float f; uint32_t u; } pun;
    pun.f = -0.15625f;
    printf("  union:  float %.5f → 0x%08X\n", pun.f, pun.u);
    pun.u = 0xBE200000;
    printf("  union:  0x%08X → float %.5f\n", pun.u, pun.f);
    /* C99 §6.5.2.3 note 95 explicitly permits reading union member
     * different from the one last written — UB in C++ but OK in C99 */

    subsection("Real use: extract IEEE 754 components");
    float val = 3.14f;
    uint32_t raw;
    memcpy(&raw, &val, sizeof(raw));
    uint32_t sign     = (raw >> 31) & 0x1;
    uint32_t exponent = (raw >> 23) & 0xFF;
    uint32_t mantissa =  raw        & 0x7FFFFF;
    printf("  3.14f → bits: 0x%08X\n", raw);
    printf("    sign=%u  exponent=%u (biased)  mantissa=0x%06X\n",
           sign, exponent, mantissa);

    /*
     * INTERVIEW NOTES:
     *   - memcpy for type punning: zero overhead with optimization — compiler
     *     turns it into a single MOV instruction
     *   - union type punning: C99 OK, C++ UB — know the difference
     *   - Direct pointer cast: UB with -O2 strict aliasing — never do this
     *   - Common real use: network protocol parsing (read uint32 from byte buffer),
     *     float inspection, serialization/deserialization
     *   - htonl/ntohl patterns use memcpy internally for portability
     */
}

/* ============================================================================
 * TOPIC 10: STACK vs HEAP POINTER LIFETIME RULES
 *
 * The most common source of dangling pointers — when does memory become invalid?
 *
 * STACK lifetime: until the scope (function/block) exits
 * HEAP  lifetime: until free() is called
 * STATIC lifetime: entire program execution
 * LITERAL lifetime: entire program (read-only segment)
 * ========================================================================== */

/* BAD — returns pointer to local (stack) variable */
int *bad_return_local(void)
{
    int local = 42;
    return &local;   /* local dies when function returns — DANGLING */
}

/* GOOD — returns pointer to heap (caller must free) */
int *good_return_heap(void)
{
    int *p = (int *)malloc(sizeof(int));
    if (p) *p = 42;
    return p;   /* heap persists — caller owns it, caller must free */
}

/* GOOD — returns pointer to static (lives forever, shared) */
int *good_return_static(void)
{
    static int val = 42;
    return &val;   /* static persists — but shared across all callers! */
}

/* GOOD — returns pointer to string literal (read-only, lives forever) */
const char *good_return_literal(void)
{
    return "Hello";   /* string literal in .rodata — always valid */
    /* BAD would be: char buf[] = "Hello"; return buf; — stack copy! */
}

void demo_pointer_lifetime(void)
{
    section("TOPIC 10: STACK vs HEAP POINTER LIFETIME RULES");

    subsection("Stack pointer lifetime — dies on scope exit");
    {
        int local = 100;
        int *p = &local;
        printf("  Inside scope: *p = %d  (valid)\n", *p);
        /* p is still in scope here — valid */
    }
    /* local is GONE here — any p saved from inside would be dangling */
    printf("  After scope: local destroyed — any saved ptr is dangling\n");

    subsection("BAD: return pointer to local (dangling)");
    int *bad = bad_return_local();
    /* *bad is UNDEFINED BEHAVIOR — stack frame destroyed */
    printf("  bad_return_local: returned ptr is DANGLING — do not deref!\n");
    (void)bad;  /* suppress unused warning — we intentionally don't use it */

    subsection("GOOD: return pointer to heap");
    int *heap_p = good_return_heap();
    if (heap_p) {
        printf("  good_return_heap: *p = %d  (valid until free)\n", *heap_p);
        free(heap_p);
        heap_p = NULL;
        printf("  After free + NULL: safe\n");
    }

    subsection("GOOD: return pointer to static");
    int *static_p = good_return_static();
    printf("  good_return_static: *p = %d  (valid always — shared!)\n", *static_p);
    /* modifying *static_p affects ALL callers — it's shared */

    subsection("GOOD: return string literal");
    const char *lit = good_return_literal();
    printf("  good_return_literal: \"%s\"  (valid always — read-only)\n", lit);

    subsection("Lifetime table");
    printf("  Memory   | Created when         | Dies when\n");
    printf("  ---------|----------------------|-------------------------\n");
    printf("  Stack    | function/block entry | function/block exits\n");
    printf("  Heap     | malloc/calloc        | free() called\n");
    printf("  Static   | program start        | program ends\n");
    printf("  Literal  | compile time         | program ends (read-only)\n");

    /*
     * INTERVIEW NOTES:
     *   - Most common bug category: returning ptr to local variable
     *   - Heap return: caller is responsible for free() — document this!
     *   - Static return: convenient but NOT thread-safe (shared state)
     *   - Literal return: always use const char* — never modify literal
     *   - Block scope: { int x; int *p=&x; } ... use p here → dangling
     */
}

/* ============================================================================
 * TOPIC 11: POINTER TO FUNCTION RETURNING POINTER
 *
 * Complex pointer declarations — use the "clockwise spiral rule" or
 * right-to-left reading to decode them.
 *
 * Examples:
 *   int   *fp(int)        → function taking int, returning int*
 *   int  (*fp)(int)       → pointer to function taking int, returning int
 *   int *(*fp)(int)       → pointer to function taking int, returning int*
 *   int *(*fp[5])(int)    → array of 5 ptrs to functions returning int*
 *   int *(*(*fp)(int))[5] → ptr to fn taking int, returning ptr to array of 5 int*
 *
 * TRICK: use cdecl.org or typedef to untangle complex declarations
 * ========================================================================== */

/* A function that takes int and returns a pointer to int */
int *get_element(int index)
{
    static int data[] = {10, 20, 30, 40, 50};
    if (index >= 0 && index < 5) return &data[index];
    return NULL;
}

/* Another function with same signature */
int *get_doubled(int index)
{
    static int doubled[] = {20, 40, 60, 80, 100};
    if (index >= 0 && index < 5) return &doubled[index];
    return NULL;
}

/* A function that RETURNS a function pointer */
/* Returns: pointer to function(int)->int* */
typedef int *(*GetterFn)(int);   /* typedef makes this readable */

GetterFn select_getter(int mode)
{
    if (mode == 0) return get_element;  /* return function pointer */
    else           return get_doubled;
}

void demo_ptr_fn_returning_ptr(void)
{
    section("TOPIC 11: POINTER TO FUNCTION RETURNING POINTER");

    subsection("int *fp(int) vs int (*fp)(int) — the key distinction");
    printf("  int  *fp(int)   → FUNCTION that returns int*   (not a ptr)\n");
    printf("  int (*fp)(int)  → POINTER to function returning int\n");
    printf("  int *(*fp)(int) → POINTER to function returning int*\n\n");

    /* Pointer to function that returns int* */
    int *(*getter)(int);         /* fp is ptr to fn(int)->int* */

    getter = get_element;
    int *elem = getter(2);       /* call via pointer */
    printf("  getter=get_element: getter(2) = %d\n", elem ? *elem : -1);

    getter = get_doubled;
    elem = getter(2);
    printf("  getter=get_doubled: getter(2) = %d\n", elem ? *elem : -1);

    subsection("Function returning a function pointer");
    GetterFn fn = select_getter(0);   /* fn is a function pointer */
    printf("  select_getter(0) → get_element: fn(3) = %d\n",
           fn ? *fn(3) : -1);

    fn = select_getter(1);
    printf("  select_getter(1) → get_doubled: fn(3) = %d\n",
           fn ? *fn(3) : -1);

    subsection("Array of function pointers to functions returning int*");
    int *(*arr[2])(int) = { get_element, get_doubled };
    for (int i = 0; i < 2; i++) {
        int *r = arr[i](1);
        printf("  arr[%d](1) = %d\n", i, r ? *r : -1);
    }

    subsection("Using typedef — always prefer for readability");
    /* typedef int *(*GetterFn)(int);  — declared above */
    GetterFn fns[] = { get_element, get_doubled };
    printf("  Via typedef array: fns[0](0)=%d  fns[1](0)=%d\n",
           *fns[0](0), *fns[1](0));

    printf("\n  Reading complex declarations (right-to-left rule):\n");
    printf("  int *(*fp)(int):\n");
    printf("    fp          → fp is\n");
    printf("    (*fp)       → a pointer\n");
    printf("    (*fp)(int)  → to a function taking int\n");
    printf("    int*(*fp)   → returning int*\n");

    /*
     * INTERVIEW NOTES:
     *   - int *fp(int)   → function (not a pointer!) — parentheses around *fp needed
     *   - int (*fp)(int) → pointer to function (parentheses mandatory)
     *   - int *(*fp)(int)→ pointer to function returning int*
     *   - Always use typedef for complex function pointer types — readable + safe
     *   - signal() in C stdlib: void (*signal(int, void(*)(int)))(int)
     *     — returns a function pointer! Notoriously complex declaration.
     *   - cdecl.org — paste declaration, get English explanation
     */
}

/* ============================================================================
 * TOPIC 12: offsetof MACRO + CONTAINER_OF PATTERN
 *
 * offsetof(type, member):
 *   Returns the byte offset of a member within a struct.
 *   Defined in <stddef.h>. Evaluated at compile time.
 *
 * CONTAINER_OF(ptr, type, member):
 *   Given a pointer to a MEMBER of a struct, recover the pointer to the
 *   CONTAINING struct. Core of Linux kernel linked lists, Zephyr work queues.
 *
 * How it works:
 *   If we know:  member is at offset N from start of struct
 *   And we have: pointer to member
 *   Then:        start of struct = (char*)member_ptr - N
 * ========================================================================== */

/* CONTAINER_OF macro — Linux kernel style */
#define CONTAINER_OF(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))
/*
 * Breakdown:
 *   offsetof(type, member)  → compile-time byte offset of member in type
 *   (char *)(ptr)           → treat ptr as byte pointer for arithmetic
 *   subtract offset         → walk back to start of struct
 *   (type *)                → cast result to pointer to containing struct
 */

/* Intrusive linked list node — embedded INSIDE the data struct */
typedef struct ListNode_ {
    struct ListNode_ *next;
    struct ListNode_ *prev;
} ListNode;

/* Task struct — node is embedded, not a pointer to separate allocation */
typedef struct {
    int      task_id;
    int      priority;
    ListNode node;          /* list node embedded directly inside Task */
    char     name[16];
} Task;

/* Generic list traversal — works without knowing Task internals */
void list_print_nodes(ListNode *head)
{
    printf("  Raw node addresses: ");
    for (ListNode *n = head; n != NULL; n = n->next) {
        printf("%p ", (void *)n);
    }
    printf("\n");
}

void demo_offsetof_container_of(void)
{
    section("TOPIC 12: offsetof MACRO + CONTAINER_OF PATTERN");

    subsection("offsetof — byte offset of struct member");

    typedef struct {
        char   a;    /* offset 0 */
        int    b;    /* offset 4 (3 bytes padding after a) */
        double c;    /* offset 8 */
        short  d;    /* offset 16 */
    } Example;

    printf("  offsetof(Example, a) = %zu\n", offsetof(Example, a));  /* 0  */
    printf("  offsetof(Example, b) = %zu\n", offsetof(Example, b));  /* 4  */
    printf("  offsetof(Example, c) = %zu\n", offsetof(Example, c));  /* 8  */
    printf("  offsetof(Example, d) = %zu\n", offsetof(Example, d));  /* 16 */
    printf("  sizeof(Example)      = %zu\n", sizeof(Example));        /* 24 */

    subsection("CONTAINER_OF — recover struct from member pointer");

    /* Create some tasks */
    Task t1 = { 1, 10, {NULL, NULL}, "idle"   };
    Task t2 = { 2, 20, {NULL, NULL}, "sensor" };
    Task t3 = { 3, 30, {NULL, NULL}, "comms"  };

    /* Link them via the embedded list node — no separate allocation */
    t1.node.next = &t2.node;  t2.node.prev = &t1.node;
    t2.node.next = &t3.node;  t3.node.prev = &t2.node;
    t3.node.next = NULL;      t1.node.prev = NULL;

    ListNode *head = &t1.node;   /* list head points to EMBEDDED node */

    printf("  offsetof(Task, node) = %zu bytes from Task start\n",
           offsetof(Task, node));

    printf("  Traversing task list via CONTAINER_OF:\n");
    for (ListNode *n = head; n != NULL; n = n->next) {
        /* n is a pointer to the embedded 'node' member */
        /* CONTAINER_OF recovers pointer to the enclosing Task */
        Task *t = CONTAINER_OF(n, Task, node);
        printf("    Task id=%d  priority=%d  name=%s\n",
               t->task_id, t->priority, t->name);
    }

    subsection("How CONTAINER_OF math works");
    Task *orig = &t2;
    ListNode *member_ptr = &t2.node;   /* pointer to embedded member */

    printf("  &t2            = %p\n", (void *)orig);
    printf("  &t2.node       = %p\n", (void *)member_ptr);
    printf("  offsetof       = %zu\n", offsetof(Task, node));
    printf("  member - offset= %p  (should equal &t2)\n",
           (void *)((char *)member_ptr - offsetof(Task, node)));

    Task *recovered = CONTAINER_OF(member_ptr, Task, node);
    printf("  CONTAINER_OF recovered: id=%d name=%s\n",
           recovered->task_id, recovered->name);
    printf("  recovered == orig: %s\n", recovered == orig ? "YES" : "NO");

    /*
     * INTERVIEW NOTES:
     *   - offsetof(type, member) is a compile-time constant — no runtime cost
     *   - CONTAINER_OF is the Linux kernel's list.h core macro
     *   - Zephyr RTOS: K_WORK, k_queue, k_sem all use intrusive lists
     *   - Intrusive list: node embedded IN the struct — no extra malloc per node
     *     vs. Non-intrusive: node holds a pointer to data — extra indirection
     *   - CONTAINER_OF requires that ptr actually points to the named member
     *     of the given type — using wrong type = UB (pointer arithmetic error)
     *   - typeof() GCC extension makes CONTAINER_OF type-safer:
     *     Linux version includes a type-check via typeof
     */
}

/* ============================================================================
 * MAIN
 * ========================================================================== */
int main(void)
{
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║   C ADVANCED POINTER CONCEPTS — HIGH & MEDIUM PRIORITY   ║\n");
    printf("║   Aravinth K — Interview Preparation (Part 2)            ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n");

    /* HIGH PRIORITY */
    demo_restrict_pointer();          /*  1 */
    demo_const_in_params();           /*  2 */
    demo_null_vs_zero();              /*  3 */
    demo_opaque_pointer();            /*  4 */
    demo_sizeof_pointer();            /*  5 */
    demo_near_far_pointer();          /*  6 */

    /* MEDIUM PRIORITY */
    demo_vtable_pattern();            /*  7 */
    demo_strict_aliasing();           /*  8 */
    demo_type_punning();              /*  9 */
    demo_pointer_lifetime();          /* 10 */
    demo_ptr_fn_returning_ptr();      /* 11 */
    demo_offsetof_container_of();     /* 12 */

    printf("\n");
    printf("=============================================================\n");
    printf("  ALL DEMOS COMPLETE\n");
    printf("  Compile: gcc -O2 -Wall -Wextra -std=c11 -o adv pointer_advanced.c\n");
    printf("  Check  : valgrind --leak-check=full ./adv\n");
    printf("=============================================================\n");

    return 0;
}
