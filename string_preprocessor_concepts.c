#define _POSIX_C_SOURCE 200809L
/*
 * =============================================================================
 * FILE    : string_preprocessor_concepts.c
 * AUTHOR  : Aravinth K
 * PURPOSE : Complete reference — Strings & Preprocessor Directives
 *           One file. Run it, read comments, understand everything.
 *
 * Compile : gcc -O0 -Wall -Wextra -std=c11 -o sp string_preprocessor_concepts.c
 * Run     : ./sp
 * =============================================================================
 *
 * ── STRINGS ──────────────────────────────────────────────────────────────────
 * HIGH PRIORITY
 *  ST1.  char* vs char[] — memory, modifiability, sizeof
 *  ST2.  String initialization patterns
 *  ST3.  strlen vs sizeof — the classic trap
 *  ST4.  strcpy / strncpy — overflow & null termination trap
 *  ST5.  strcat / strncat — destination space requirement
 *  ST6.  strcmp / strncmp — return value meaning
 *  ST7.  String input — scanf vs fgets vs scanset
 *  ST8.  String traversal — index vs pointer walk
 *  ST9.  Null terminator — what happens without '\0'
 *  ST10. Strings and pointers — pointer arithmetic on strings
 *
 * MEDIUM PRIORITY
 *  ST11. strchr / strrchr — find character
 *  ST12. strstr — find substring
 *  ST13. strtok / strtok_r — tokenizing
 *  ST14. sprintf / snprintf — format into buffer
 *  ST15. String to number — atoi, strtol, strtoul (error handling)
 *  ST16. Number to string — sprintf pattern
 *  ST17. String reversal — in-place two-pointer
 *  ST18. String as function parameter — const char* vs char*
 *
 * LOW PRIORITY
 *  ST19. memset / memcpy / memmove on strings
 *  ST20. String comparison pitfall — == vs strcmp
 *  ST21. Multi-line string literals — implicit concatenation
 *  ST22. Wide strings — wchar_t (brief mention)
 *
 * ── PREPROCESSOR ─────────────────────────────────────────────────────────────
 * HIGH PRIORITY
 *  PP1.  #define — object-like macro (constants)
 *  PP2.  #define — function-like macro + parenthesization rule
 *  PP3.  #include — <> vs "" and search order
 *  PP4.  #ifdef / #ifndef / #endif
 *  PP5.  #if / #elif / #else — expression-based conditional
 *  PP6.  #pragma once vs include guard
 *  PP7.  Macro pitfall — side effects (i++ evaluated twice)
 *  PP8.  Macro pitfall — missing parentheses (precedence)
 *  PP9.  Predefined macros — __FILE__ __LINE__ __func__ __DATE__ __TIME__
 *  PP10. #undef — removing and redefining macros
 *
 * MEDIUM PRIORITY
 *  PP11. do { } while(0) idiom — safe multi-statement macro
 *  PP12. Stringify operator # — token to string literal
 *  PP13. Token paste operator ## — joining tokens
 *  PP14. Variadic macros — __VA_ARGS__ (C99)
 *  PP15. #error / #warning — compile-time diagnostics
 *  PP16. #line — override line/file reporting
 *  PP17. #pragma pack — struct alignment
 *  PP18. Macro vs static inline — when to use which
 *
 * LOW PRIORITY
 *  PP19. #pragma GCC poison — ban identifier
 *  PP20. X-macro pattern — code generation
 *  PP21. __attribute__ — GCC attributes
 *  PP22. _Static_assert — compile-time assertion (C11)
 *  PP23. #pragma GCC optimize — per-function optimization
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define _POSIX_C_SOURCE 200809L
#include <wchar.h>
#include <errno.h>
#include <limits.h>

/* ── helpers ──────────────────────────────────────────────────────────────── */
static void sec(const char *t)
{
    printf("\n╔══════════════════════════════════════════════════════════╗\n");
    printf("║  %-56s║\n", t);
    printf("╚══════════════════════════════════════════════════════════╝\n");
}
static void sub(const char *t) { printf("\n  ┌─ %s\n", t); }

/* ╔══════════════════════════════════════════════════════════════════════════╗
 * ║                           STRINGS                                       ║
 * ╚══════════════════════════════════════════════════════════════════════════╝ */

/* =============================================================================
 * ST1. char* vs char[] — MEMORY, MODIFIABILITY, sizeof
 *
 * char arr[] = "hello"  → COPY on stack  — modifiable — sizeof = len+1
 * char *ptr  = "hello"  → points to READ-ONLY literal in .rodata
 *                          sizeof = pointer size (8), NOT string length
 *
 * THE KEY DIFFERENCES:
 *   1. Modifiability : arr modifiable, literal via ptr is READ-ONLY
 *   2. sizeof        : arr gives string size+1, ptr gives pointer size
 *   3. Memory        : arr on stack, literal in text/rodata segment
 *   4. Reassignment  : ptr can be redirected, arr base cannot
 * =========================================================================== */
void st1_char_star_vs_char_array(void)
{
    sec("ST1 [HIGH] char* vs char[] — MEMORY, MODIFIABILITY, sizeof");

    /* char[] — stack copy — modifiable */
    char arr[] = "Embedded";
    /* internally: {'E','m','b','e','d','d','e','d','\0'} on stack */

    /* const char* — read-only literal in .rodata */
    const char *ptr = "Embedded";
    /* ptr points to literal — cannot modify characters */

    sub("sizeof difference — the classic interview trap");
    printf("  char arr[] = \"Embedded\";\n");
    printf("    sizeof(arr) = %zu  ← total bytes incl \\0 (string size)\n", sizeof(arr));
    printf("    strlen(arr) = %zu  ← characters excl \\0\n", strlen(arr));

    printf("\n  const char *ptr = \"Embedded\";\n");
    printf("    sizeof(ptr) = %zu  ← POINTER size, NOT string length!\n", sizeof(ptr));
    printf("    strlen(ptr) = %zu  ← still correct (walks until \\0)\n", strlen(ptr));

    sub("Modifiability");
    arr[0] = 'X';               /* OK — modifying stack copy */
    printf("  arr[0]='X'  → arr = \"%s\"  (modification OK)\n", arr);

    /* ptr[0] = 'X';  ← UNDEFINED BEHAVIOR — read-only .rodata */
    printf("  ptr[0]='X'  → UNDEFINED BEHAVIOR (commented out)\n");
    printf("                  .rodata is mapped read-only — SIGSEGV on write\n");

    sub("Pointer reassignment vs array base");
    ptr = "NewString";          /* OK — ptr itself can point elsewhere */
    printf("  ptr = \"NewString\"  → OK, ptr now points to different literal\n");
    printf("  arr = \"NewString\"  → COMPILE ERROR (array base is not reassignable)\n");

    sub("Memory locations");
    printf("  arr lives on STACK  — address: %p\n", (void*)arr);
    printf("  ptr points to .rodata — address: %p\n", (void*)ptr);

    /*
     * INTERVIEW NOTES:
     *   - sizeof(arr) = strlen + 1  (includes null terminator) — at declaration only
     *   - sizeof(ptr) = 8 always (pointer size) — never gives string length
     *   - char arr[] is modifiable; char *ptr to literal is NOT
     *   - Use const char* when pointing to literals — communicates intent
     *   - After decay (passing to function): sizeof gives 8 for both
     */
}

/* =============================================================================
 * ST2. STRING INITIALIZATION PATTERNS
 * =========================================================================== */
void st2_string_init_patterns(void)
{
    sec("ST2 [HIGH] STRING INITIALIZATION PATTERNS");

    sub("1. Array from string literal — compiler appends \\0");
    char s1[] = "Hello";
    /* {'H','e','l','l','o','\0'} — size 6 */
    printf("  char s1[] = \"Hello\"  sizeof=%zu  s1='%s'\n", sizeof(s1), s1);

    sub("2. Array with explicit size — must fit string + \\0");
    char s2[10] = "Hello";
    /* remaining bytes s2[6..9] = 0 automatically */
    printf("  char s2[10]=\"Hello\"  s2[5]=0x%02X s2[6]=0x%02X (zeroed)\n",
           (unsigned char)s2[5], (unsigned char)s2[6]);

    sub("3. Character-by-character — must add \\0 manually!");
    char s3[6];
    s3[0]='W'; s3[1]='o'; s3[2]='r'; s3[3]='l'; s3[4]='d'; s3[5]='\0';
    printf("  Manual char init: '%s'\n", s3);

    sub("4. Zero-initialize then fill — safest pattern");
    char s4[32] = {0};          /* all bytes = 0 first */
    strncpy(s4, "Safe", sizeof(s4)-1);
    printf("  {0} then strncpy: '%s'\n", s4);

    sub("5. Pointer to literal — read-only");
    const char *s5 = "ReadOnly";
    printf("  const char *s5 = \"ReadOnly\": '%s'  sizeof(s5)=%zu\n",
           s5, sizeof(s5));

    sub("6. strcpy / strncpy into buffer");
    char s6[32];
    strncpy(s6, "Copied", sizeof(s6)-1);
    s6[sizeof(s6)-1] = '\0';    /* always force null terminate */
    printf("  strncpy into s6: '%s'\n", s6);

    sub("7. snprintf — safest string building");
    char s7[32];
    snprintf(s7, sizeof(s7), "Val=%d Fl=%.2f", 42, 3.14f);
    printf("  snprintf: '%s'\n", s7);

    /*
     * INTERVIEW NOTES:
     *   - char s[6] = "Hello" — size MUST be at least strlen+1 for \0
     *   - char s[5] = "Hello" — NO room for \0 — not a C string!
     *   - {0} zero-init: all bytes 0 — safest before strncpy/snprintf
     *   - snprintf: always use sizeof(buf) as limit — prevents overflow
     *   - strncpy does NOT guarantee \0 if src >= n — always add manually
     */
}

/* =============================================================================
 * ST3. strlen vs sizeof — THE CLASSIC TRAP
 * =========================================================================== */
void show_string_sizes(char *s)      /* array decays to pointer here */
{
    printf("  Inside function:\n");
    printf("    sizeof(s) = %zu  ← pointer size — WRONG for string length\n", sizeof(s));
    printf("    strlen(s) = %zu  ← correct string length\n", strlen(s));
}

void st3_strlen_vs_sizeof(void)
{
    sec("ST3 [HIGH] strlen vs sizeof — THE CLASSIC TRAP");

    char arr[]        = "Embedded Systems";
    const char *ptr   = "Embedded Systems";
    char buf[64]      = "Embedded Systems";

    sub("At declaration site");
    printf("  char arr[] = \"Embedded Systems\";\n");
    printf("    sizeof(arr) = %zu  (16 chars + 1 null = 17)\n", sizeof(arr));
    printf("    strlen(arr) = %zu  (16 chars, no null)\n", strlen(arr));

    printf("\n  const char *ptr = \"Embedded Systems\";\n");
    printf("    sizeof(ptr) = %zu  (pointer size — NOT string length!)\n", sizeof(ptr));
    printf("    strlen(ptr) = %zu  (walks to \\0 — correct)\n", strlen(ptr));

    printf("\n  char buf[64] = \"Embedded Systems\";\n");
    printf("    sizeof(buf) = %zu  (total buffer = 64)\n", sizeof(buf));
    printf("    strlen(buf) = %zu  (content length = 16)\n", strlen(buf));

    sub("After passing to function — array decays");
    show_string_sizes(arr);     /* sizeof gives 8 — lost array info */

    sub("Common bugs with sizeof instead of strlen");
    printf("  Bug: for(i=0; i<sizeof(arr)-1; i++) — OK here (arr is real array)\n");
    printf("  Bug: for(i=0; i<sizeof(ptr)-1; i++) — WRONG (sizeof=8, not length)\n");
    printf("  Fix: always use strlen() for string length\n");
    printf("       always use sizeof() for buffer/array capacity\n");

    sub("Summary table");
    printf("  %-25s | sizeof | strlen\n", "Variable");
    printf("  %-25s | ------ | ------\n", "-------------------------");
    printf("  %-25s | %-6zu | %-6zu (real array — size = len+1)\n",
           "char arr[]=\"Hi\"", sizeof(arr), strlen(arr));
    printf("  %-25s | %-6zu | %-6zu (pointer — sizeof=8 always)\n",
           "const char *ptr=\"Hi\"", sizeof(ptr), strlen(ptr));
    printf("  %-25s | %-6zu | %-6zu (buffer capacity vs content)\n",
           "char buf[64]=\"Hi\"", sizeof(buf), strlen(buf));

    /*
     * INTERVIEW NOTES:
     *   - sizeof(arr) at declaration = strlen(arr) + 1 (the \0)
     *   - sizeof(ptr) = 8 ALWAYS — tells you nothing about string length
     *   - sizeof(buf) = buffer CAPACITY (64) — not content length
     *   - strlen() always correct for content length (walks to \0)
     *   - sizeof() always correct for buffer/array size at declaration
     *   - Use sizeof(buf)-1 as limit for strncpy/snprintf
     */
}

/* =============================================================================
 * ST4. strcpy / strncpy — OVERFLOW & NULL TERMINATION TRAP
 * =========================================================================== */
void st4_strcpy_strncpy(void)
{
    sec("ST4 [HIGH] strcpy / strncpy — OVERFLOW & NULL TERMINATION TRAP");

    sub("strcpy — no bounds check (DANGEROUS for unknown input)");
    char dst1[10];
    strcpy(dst1, "Hello");     /* safe here — "Hello" is 5+1=6 bytes */
    printf("  strcpy(dst,'Hello'):  '%s'\n", dst1);
    /* strcpy(dst1, "TooLongString"); ← BUFFER OVERFLOW — UB, heap/stack corruption */
    printf("  strcpy with oversized src → BUFFER OVERFLOW (not shown)\n");

    sub("strncpy — bounded copy — NULL TERMINATION TRAP");
    char dst2[8];
    strncpy(dst2, "Embedded", sizeof(dst2)-1);  /* copy max 7 chars */
    dst2[sizeof(dst2)-1] = '\0';                 /* ALWAYS add \0 manually */
    printf("  strncpy(dst,\"Embedded\",7) + manual \\0: '%s'\n", dst2);

    /* The null termination trap */
    char dst3[5];
    char src3[] = "Embedded";    /* 9 chars > dst3 capacity */
    strncpy(dst3, src3, sizeof(dst3));
    /* dst3 is NOT null-terminated — strncpy didn't fit \0! */
    /* dst3[4] = 'd' (last char copied) — no \0 anywhere in dst3 */
    printf("  strncpy(dst3[5],\"Embedded\",5) — NO \\0 added!\n");
    printf("    dst3[4] = '%c'  (no null terminator — unsafe to printf!)\n",
           dst3[4]);
    dst3[sizeof(dst3)-1] = '\0';   /* fix it */
    printf("  After manually adding \\0: '%s'\n", dst3);

    sub("strncpy also zero-pads if src shorter than n");
    char dst4[10] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                     0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    strncpy(dst4, "Hi", 10);   /* copies 'H','i','\0' then pads 7 more \0s */
    printf("  strncpy(dst4[10],\"Hi\",10): content='%s'\n", dst4);
    printf("    dst4[2]=0x%02X dst4[5]=0x%02X (zero-padded)\n",
           (unsigned char)dst4[2], (unsigned char)dst4[5]);

    sub("snprintf — safest alternative (always null-terminates)");
    char dst5[8];
    int written = snprintf(dst5, sizeof(dst5), "%s", "Embedded");
    printf("  snprintf(dst5[8],\"Embedded\"): '%s'  written=%d\n",
           dst5, written);
    /* snprintf ALWAYS null-terminates — even if truncated */
    /* return value = chars that WOULD be written — check for truncation */
    if (written >= (int)sizeof(dst5))
        printf("  Output was TRUNCATED (would need %d bytes)\n", written);

    sub("Safe copy pattern");
    char safe_dst[16];
    const char *safe_src = "SafeCopy";
    snprintf(safe_dst, sizeof(safe_dst), "%s", safe_src);  /* best */
    printf("  snprintf pattern: '%s'\n", safe_dst);

    /*
     * INTERVIEW NOTES:
     *   - strcpy: no limit — NEVER use on untrusted input
     *   - strncpy: bounded BUT does NOT guarantee \0 if src >= n
     *   - strncpy DOES zero-pad remaining bytes if src < n (security feature)
     *   - ALWAYS: dst[n-1] = '\0' after strncpy
     *   - snprintf: always null-terminates, returns would-be length — BEST CHOICE
     *   - strlcpy (BSD): safe + always null-terminates — not in C standard
     */
}

/* =============================================================================
 * ST5. strcat / strncat — DESTINATION SPACE REQUIREMENT
 * =========================================================================== */
void st5_strcat_strncat(void)
{
    sec("ST5 [HIGH] strcat / strncat — DESTINATION SPACE REQUIREMENT");

    sub("strcat — appends to end of dst (no bounds check)");
    char dst1[32] = "Hello, ";
    strcat(dst1, "World");     /* dst1 must have room for both + \0 */
    printf("  strcat: '%s'\n", dst1);
    /* dst1 must have: strlen("Hello, ") + strlen("World") + 1 = 13 bytes minimum */

    sub("strcat buffer overflow (DANGEROUS)");
    printf("  char dst[8]=\"Hello, \"; strcat(dst,\"World\"); → OVERFLOW (not shown)\n");
    printf("  dst only has 8 bytes — needs 13 — buffer overflow!\n");

    sub("strncat — appends at most n chars (ALWAYS adds \\0)");
    char dst2[16] = "Hello, ";
    strncat(dst2, "World!!!", 5);   /* append at most 5 chars from src */
    printf("  strncat(dst,\"World!!!\",5): '%s'\n", dst2);
    /* strncat ALWAYS adds \0 after the n chars — unlike strncpy */

    sub("Correct space calculation for strncat");
    char dst3[32] = "Prefix_";
    size_t remaining = sizeof(dst3) - strlen(dst3) - 1;  /* space left - 1 for \0 */
    strncat(dst3, "SomeVeryLongSuffix", remaining);
    printf("  Safe strncat (remaining=%zu): '%s'\n", remaining, dst3);

    sub("Safe pattern — using snprintf for concatenation");
    char base[32] = "Hello";
    char result[64];
    snprintf(result, sizeof(result), "%s, %s!", base, "World");
    printf("  snprintf concat: '%s'\n", result);

    sub("Building string incrementally — pointer offset trick");
    char buf[64];
    int  offset = 0;
    offset += snprintf(buf + offset, sizeof(buf) - offset, "Part1");
    offset += snprintf(buf + offset, sizeof(buf) - offset, "-Part2");
    offset += snprintf(buf + offset, sizeof(buf) - offset, "-Part3");
    printf("  Incremental build: '%s'\n", buf);

    /*
     * INTERVIEW NOTES:
     *   - strcat: dst must have room for strlen(dst)+strlen(src)+1
     *   - strncat: UNLIKE strncpy, strncat ALWAYS null-terminates
     *   - strncat n = max chars to APPEND (not total buffer size)
     *   - Safe strncat: strncat(dst, src, sizeof(dst)-strlen(dst)-1)
     *   - Best: use snprintf for any string building — bounds-safe
     *   - strcat of unknown-length strings = classic security bug
     */
}

/* =============================================================================
 * ST6. strcmp / strncmp — RETURN VALUE MEANING
 * =========================================================================== */
void st6_strcmp_strncmp(void)
{
    sec("ST6 [HIGH] strcmp / strncmp — RETURN VALUE MEANING");

    sub("strcmp — full string comparison (lexicographic)");
    /* Return: <0 if s1<s2,  0 if equal,  >0 if s1>s2 */
    /* NOT guaranteed to be -1, 0, +1 — could be any negative/positive */

    const char *s1 = "Apple";
    const char *s2 = "Apple";
    const char *s3 = "Applet";   /* s1 is prefix of s3 */
    const char *s4 = "Banana";
    const char *s5 = "apple";    /* lowercase — different ASCII value */

    printf("  strcmp(\"%s\",\"%s\") = %d  (equal)\n",
           s1, s2, strcmp(s1, s2));
    printf("  strcmp(\"%s\",\"%s\") = %d  (s1 < s3, shorter)\n",
           s1, s3, strcmp(s1, s3));
    printf("  strcmp(\"%s\",\"%s\") = %d  (s1 < s4, 'A' < 'B')\n",
           s1, s4, strcmp(s1, s4));
    printf("  strcmp(\"%s\",\"%s\") = %d  (s1 < s5, 'A' < 'a' in ASCII)\n",
           s1, s5, strcmp(s1, s5));

    sub("How to use strcmp correctly");
    if (strcmp(s1, s2) == 0)
        printf("  s1 == s2: EQUAL\n");

    /* Sort-order comparison */
    int r = strcmp(s1, s4);
    if      (r < 0) printf("  \"%s\" comes before \"%s\"\n", s1, s4);
    else if (r > 0) printf("  \"%s\" comes after  \"%s\"\n", s1, s4);
    else            printf("  \"%s\" equals       \"%s\"\n", s1, s4);

    sub("strncmp — compare only first n characters");
    const char *a = "Embedded";
    const char *b = "EmbeddedSystems";
    printf("  strncmp(\"%s\",\"%s\",8) = %d  (first 8 chars equal)\n",
           a, b, strncmp(a, b, 8));
    printf("  strcmp (\"%s\",\"%s\")   = %d  (different lengths)\n",
           a, b, strcmp(a, b));

    sub("Case-insensitive comparison (non-standard but common)");
    /* strcasecmp (POSIX) or _stricmp (MSVC) — not in C standard */
    printf("  strcasecmp: POSIX only — not in C standard\n");
    printf("  Portable: convert both to lower/upper then strcmp\n");

    sub("Common interview trap — what does return value mean?");
    printf("  strcmp return < 0 : first string is LESS (not necessarily -1)\n");
    printf("  strcmp return = 0 : strings are EQUAL\n");
    printf("  strcmp return > 0 : first string is GREATER (not necessarily +1)\n");
    printf("  WRONG: if(strcmp(a,b)) → TRUE means NOT equal (0=equal, nonzero=diff)\n");
    printf("  RIGHT: if(strcmp(a,b)==0) → TRUE means EQUAL\n");

    /*
     * INTERVIEW NOTES:
     *   - Return value is difference of first differing chars (implementation-specific)
     *   - NEVER assume return is exactly -1 or +1 — just check sign
     *   - strcmp("a","b") < 0 because 'a'(97) < 'b'(98)
     *   - Case-sensitive: 'A'(65) < 'a'(97) — uppercase comes before lowercase
     *   - For sorted arrays: use strcmp as comparator for bsearch/qsort
     *   - if(strcmp(s1,s2)) — means NOT equal (0 is false in C)
     */
}

/* =============================================================================
 * ST7. STRING INPUT — scanf vs fgets vs scanset
 * =========================================================================== */
void st7_string_input(void)
{
    sec("ST7 [HIGH] STRING INPUT — scanf vs fgets vs scanset");

    /* We demonstrate the BEHAVIOR without actual stdin reads */

    sub("scanf with %s — STOPS at whitespace");
    printf("  scanf(\"%%s\", buf):\n");
    printf("    Input: \"Embedded Systems\"  → reads only \"Embedded\"\n");
    printf("    Stops at space — cannot read multi-word strings\n");
    printf("    No width limit → BUFFER OVERFLOW on long input!\n");
    printf("    Safer: scanf(\"%%31s\", buf) — limits to 31 chars (buf size 32)\n");

    sub("fgets — reads full line including spaces — RECOMMENDED");
    printf("  fgets(buf, sizeof(buf), stdin):\n");
    printf("    Input: \"Embedded Systems\"  → reads \"Embedded Systems\\n\"\n");
    printf("    Reads up to sizeof(buf)-1 chars — safe, no overflow\n");
    printf("    STORES the newline \\n — must strip it!\n");

    /* Demonstrate newline stripping */
    char demo_buf[] = "Hello World\n";   /* simulate fgets output */
    printf("  Before strip: '%s'  (has \\n)\n", demo_buf);
    demo_buf[strcspn(demo_buf, "\n")] = '\0';  /* find \n, replace with \0 */
    printf("  After  strip: '%s'  (\\n removed)\n", demo_buf);
    printf("  Method: buf[strcspn(buf,\"\\n\")] = '\\0'  ← safest way\n");

    sub("scanf scanset — reads until specific delimiter");
    printf("  scanf(\"%%31[^\\n]\", buf):\n");
    printf("    [^\\n] means: read everything EXCEPT newline\n");
    printf("    Input: \"Embedded Systems\"  → reads full line\n");
    printf("    31 = max chars (buffer size 32 - 1 for \\0)\n");
    printf("    Does NOT read newline into buffer (unlike fgets)\n");
    printf("    But \\n stays in input buffer — next read sees it\n");

    sub("Comparison table");
    printf("  %-25s | Multi-word | Overflow-safe | Stores \\n\n", "Method");
    printf("  %-25s | ---------- | ------------- | ---------\n",
           "-------------------------");
    printf("  %-25s | NO         | NO            | N/A\n", "scanf(\"%s\")");
    printf("  %-25s | YES        | YES           | YES (strip it)\n",
           "fgets(buf,n,stdin)");
    printf("  %-25s | YES        | YES (w/ width)| NO\n",
           "scanf(\"%31[^\\n]\")");

    sub("Safe input pattern — fgets + strip");
    printf("  char buf[64];\n");
    printf("  fgets(buf, sizeof(buf), stdin);\n");
    printf("  buf[strcspn(buf, \"\\n\")] = '\\0';  // strip newline\n");

    /*
     * INTERVIEW NOTES:
     *   - scanf %s: stops at whitespace, NO buffer overflow protection
     *   - fgets: safe, reads spaces, includes \n — always strip
     *   - strcspn(buf,"\n"): returns index of first \n or strlen if not found
     *   - buf[strcspn(buf,"\n")]='\0': safe strip (if no \n, replaces \0 with \0)
     *   - gets(): REMOVED from C11 — NEVER use — no bounds checking at all
     *   - For embedded UART: typically read char-by-char until \n or \0
     */
}

/* =============================================================================
 * ST8. STRING TRAVERSAL — INDEX vs POINTER WALK
 * =========================================================================== */
void st8_string_traversal(void)
{
    sec("ST8 [HIGH] STRING TRAVERSAL — INDEX vs POINTER WALK");

    const char *str = "Hello, World!";

    sub("Method 1: index-based (clearest)");
    printf("  Index walk: ");
    for (int i = 0; str[i] != '\0'; i++) {
        putchar(str[i]);
    }
    printf("\n");

    sub("Method 2: pointer walk (efficient — common in embedded)");
    printf("  Ptr walk  : ");
    const char *p = str;
    while (*p != '\0') {        /* *p == '\0' means end of string */
        putchar(*p);
        p++;                    /* advance pointer by 1 byte (char) */
    }
    printf("\n");

    sub("Method 3: combined — ptr in for loop");
    printf("  Ptr for   : ");
    for (const char *q = str; *q; q++) {  /* *q is truthy if not '\0' */
        putchar(*q);
    }
    printf("\n");

    sub("Count specific character — pointer walk");
    int count = 0;
    for (const char *c = str; *c; c++) {
        if (*c == 'l') count++;
    }
    printf("  Count of 'l' in \"%s\" = %d\n", str, count);

    sub("Print in reverse — calculate length first");
    int len = (int)strlen(str);
    printf("  Reverse   : ");
    for (int i = len - 1; i >= 0; i--) {
        putchar(str[i]);
    }
    printf("\n");

    sub("Pointer arithmetic — distance between chars");
    const char *first_l = strchr(str, 'l');
    const char *last_l  = strrchr(str, 'l');
    if (first_l && last_l) {
        printf("  First 'l' at index %ld, last 'l' at index %ld\n",
               first_l - str, last_l - str);
        printf("  Distance = %ld chars\n", last_l - first_l);
    }

    /*
     * INTERVIEW NOTES:
     *   - str[i] and *(str+i) are identical — compiler generates same code
     *   - *p condition: '\0' is 0 which is false — so *p alone works as condition
     *   - Pointer walk is idiomatic C — preferred in system/embedded code
     *   - ptr - base_ptr: gives index/offset of current position
     *   - Never go past \0 — UB to access beyond string end
     */
}

/* =============================================================================
 * ST9. NULL TERMINATOR — WHAT HAPPENS WITHOUT '\0'
 * =========================================================================== */
void st9_null_terminator(void)
{
    sec("ST9 [HIGH] NULL TERMINATOR — WHAT HAPPENS WITHOUT '\\0'");

    sub("Correct — with null terminator");
    char good[6] = {'H','e','l','l','o','\0'};
    printf("  With \\0   : strlen=%zu  content='%s'\n", strlen(good), good);

    sub("Missing null terminator — strlen/printf read beyond array (UB)");
    char bad[5]  = {'H','e','l','l','o'};  /* NO \0 — not a valid C string */
    /* strlen(bad) → walks past end until it finds a 0 byte in memory */
    /* printf(bad)  → prints random bytes until it hits a 0 */
    /* This is undefined behavior — may print garbage, crash, or seem fine */
    printf("  Without \\0: bad[5]={'H','e','l','l','o'} — NOT a valid string\n");
    printf("              strlen(bad) would walk off the end → UB\n");
    printf("              printf(\"%%s\",bad) would print garbage → UB\n");
    (void)bad;  /* suppress unused warning */

    sub("char buf[5]=\"Hello\" — NO room for \\0");
    /* char buf[5] = "Hello";  ← WARNING: initializer too long */
    printf("  char buf[5]=\"Hello\" — compiler warning: too long\n");
    printf("  Result: buf has 5 chars, no null — NOT a C string\n");

    sub("strncpy not filling \\0 — another way to get unterminated string");
    char s[5];
    strncpy(s, "Embedded", sizeof(s));  /* copies 5 chars, no room for \0 */
    s[sizeof(s)-1] = '\0';              /* FIX: always force-terminate */
    printf("  strncpy without manual \\0 → unterminated (fixed here)\n");
    printf("  After fix: '%s'\n", s);

    sub("What \\0 means in memory");
    char str[] = "AB";
    printf("  'A'=0x%02X  'B'=0x%02X  '\\0'=0x%02X\n",
           (unsigned char)str[0],
           (unsigned char)str[1],
           (unsigned char)str[2]);  /* \0 is binary 0x00 */
    printf("  \\0 is not printable space — it is binary zero\n");
    printf("  All string functions stop AT \\0, not after it\n");

    /*
     * INTERVIEW NOTES:
     *   - C strings: char array + \0 at end — BOTH required
     *   - Without \0: strlen, printf, strcpy all have UB (read beyond array)
     *   - char s[5]="Hello" — exactly 5 chars, no \0 — warning from GCC
     *   - strncpy: doesn't guarantee \0 if src >= n — always add manually
     *   - memset(buf,0,sizeof(buf)) before filling → guaranteed termination
     *   - \0 = 0x00 = binary zero — distinct from '0' = 0x30 = ASCII 48
     */
}

/* =============================================================================
 * ST10. STRINGS AND POINTERS — POINTER ARITHMETIC ON STRINGS
 * =========================================================================== */
void st10_string_pointer_arithmetic(void)
{
    sec("ST10 [HIGH] STRINGS AND POINTERS — POINTER ARITHMETIC");

    char str[] = "Hello, World!";

    sub("Pointer to string = pointer to first char");
    char *p = str;              /* p = &str[0] */
    printf("  str = %p  p = %p  &str[0] = %p  (all same)\n",
           (void*)str, (void*)p, (void*)&str[0]);

    sub("Advance pointer — skip characters");
    printf("  *p       = '%c'\n", *p);       /* H */
    p += 7;                                   /* skip "Hello, " */
    printf("  p+=7: *p = '%c'\n", *p);       /* W */
    printf("  p as string: \"%s\"\n", p);    /* "World!" */

    sub("Compute offset from base");
    char *found = strchr(str, ',');
    if (found) {
        printf("  ',' found at offset %ld from start\n", found - str);
        printf("  String after ',': \"%s\"\n", found + 1);  /* skip the comma */
    }

    sub("Pointer comparison — test position");
    char *start = str;
    char *end   = str + strlen(str) - 1;  /* pointer to last char (not \0) */
    printf("  start='%c' @ %p\n", *start, (void*)start);
    printf("  end  ='%c' @ %p\n", *end, (void*)end);
    printf("  Length via subtraction: end-start+1 = %ld\n", end - start + 1);

    sub("Passing offset string to function — no copy needed");
    const char *msg  = "ERROR: file not found";
    const char *text = msg + 7;  /* skip "ERROR: " */
    printf("  Full: \"%s\"\n", msg);
    printf("  Skip prefix: \"%s\"  (ptr arithmetic, no copy)\n", text);

    sub("String copy via pointer arithmetic");
    char src[] = "Zephyr RTOS";
    char dst[32];
    char *d = dst;
    const char *s = src;
    while ((*d++ = *s++));     /* copies including \0 — classic C idiom */
    printf("  Pointer-copy idiom: '%s'\n", dst);

    /*
     * INTERVIEW NOTES:
     *   - p++ on char* advances 1 byte (size of char = 1)
     *   - str + n: pointer to nth character — no copy, O(1)
     *   - ptr1 - ptr2: number of characters between (ptrdiff_t)
     *   - while(*d++=*s++) : copies src to dst including \0, then stops
     *   - Passing str+n to function: function sees substring — efficient
     *   - strchr returns pointer — ptr - str gives index of found char
     */
}

/* =============================================================================
 * ST11. strchr / strrchr — FIND CHARACTER (MEDIUM)
 * =========================================================================== */
void st11_strchr_strrchr(void)
{
    sec("ST11 [MEDIUM] strchr / strrchr — FIND CHARACTER IN STRING");

    const char *str = "embedded.systems.c";

    sub("strchr — first occurrence");
    char *first_dot = strchr(str, '.');
    if (first_dot) {
        printf("  First '.': index=%ld  rest=\"%s\"\n",
               first_dot - str, first_dot);
        printf("  Before '.': %.*s\n", (int)(first_dot - str), str);
    }

    sub("strrchr — last occurrence");
    char *last_dot = strrchr(str, '.');
    if (last_dot) {
        printf("  Last  '.': index=%ld  rest=\"%s\"\n",
               last_dot - str, last_dot);
        printf("  Extension: \"%s\"\n", last_dot + 1);  /* file extension */
    }

    sub("Get filename without extension using strrchr");
    char path[] = "src/drivers/uart.c";
    char *ext = strrchr(path, '.');
    char *sep = strrchr(path, '/');
    const char *basename = sep ? sep + 1 : path;
    if (ext) {
        printf("  basename (no ext): %.*s\n", (int)(ext - basename), basename);
    }

    sub("NULL return — character not found");
    char *not_found = strchr(str, 'z');
    printf("  strchr(str,'z') = %s\n", not_found ? "found" : "NULL (not found)");
    /* Always check for NULL before dereferencing! */

    sub("Find all occurrences — loop pattern");
    const char *haystack = "banana";
    printf("  Positions of 'a' in \"%s\": ", haystack);
    const char *pos = haystack;
    while ((pos = strchr(pos, 'a')) != NULL) {
        printf("%ld ", pos - haystack);
        pos++;  /* advance past found char to search rest */
    }
    printf("\n");

    /*
     * INTERVIEW NOTES:
     *   - strchr(s,c): returns ptr to FIRST occurrence or NULL
     *   - strrchr(s,c): returns ptr to LAST occurrence or NULL
     *   - return - base_ptr: gives index of found character
     *   - return + 1: pointer to character AFTER found one
     *   - Loop: pos=strchr(pos,c); ... pos++ to find all occurrences
     *   - strchr(s,'\0') returns pointer to the null terminator
     */
}

/* =============================================================================
 * ST12. strstr — FIND SUBSTRING (MEDIUM)
 * =========================================================================== */
void st12_strstr(void)
{
    sec("ST12 [MEDIUM] strstr — FIND SUBSTRING");

    const char *haystack = "Zephyr RTOS is a real-time OS for embedded";
    const char *needle1  = "RTOS";
    const char *needle2  = "Python";

    sub("Basic substring search");
    char *found = strstr(haystack, needle1);
    if (found) {
        printf("  Found \"%s\" at index %ld\n", needle1, found - haystack);
        printf("  From match: \"%s\"\n", found);
    }

    char *notfound = strstr(haystack, needle2);
    printf("  strstr(\"%s\") = %s\n",
           needle2, notfound ? "found" : "NULL (not found)");

    sub("Count substring occurrences");
    const char *text = "the cat sat on the mat near the bat";
    const char *sub_str = "the";
    int count = 0;
    const char *p = text;
    while ((p = strstr(p, sub_str)) != NULL) {
        count++;
        p += strlen(sub_str);  /* advance past matched substring */
    }
    printf("  \"%s\" appears %d times in text\n", sub_str, count);

    sub("Extract content between delimiters");
    char url[] = "http://example.com/path?key=value";
    char *host_start = strstr(url, "://");
    if (host_start) {
        host_start += 3;  /* skip "://" */
        char *path_start = strchr(host_start, '/');
        if (path_start) {
            printf("  Host: %.*s\n", (int)(path_start - host_start), host_start);
            printf("  Path: %s\n", path_start);
        }
    }

    /*
     * INTERVIEW NOTES:
     *   - strstr(haystack, needle): O(n*m) — naive search
     *   - Returns ptr to start of first match, or NULL if not found
     *   - After match: p += strlen(needle) to search remaining string
     *   - Empty needle ("") always matches — returns haystack pointer
     *   - Case-sensitive — no standard case-insensitive version in C
     *   - For multiple searches: consider KMP or Boyer-Moore for performance
     */
}

/* =============================================================================
 * ST13. strtok / strtok_r — TOKENIZING (MEDIUM)
 * =========================================================================== */
void st13_strtok(void)
{
    sec("ST13 [MEDIUM] strtok / strtok_r — TOKENIZING");

    sub("strtok — modifies original string! NOT reentrant!");
    char csv[] = "Alice,30,Engineer,Coimbatore";  /* WILL BE MODIFIED */
    printf("  Input : \"%s\"\n", csv);

    char *token = strtok(csv, ",");  /* first call: pass the string */
    printf("  Tokens: ");
    while (token != NULL) {
        printf("[%s] ", token);
        token = strtok(NULL, ",");   /* subsequent calls: pass NULL */
    }
    printf("\n");

    /* csv is now "Alice\030\0Engineer\0Coimbatore" — delimiters replaced with \0 */
    printf("  After strtok: csv[5]='%c'(0x%02X) — original MODIFIED!\n",
           csv[5] ? csv[5] : '?', (unsigned char)csv[5]);

    sub("strtok with multiple delimiters");
    char path[] = "/usr/local/bin/gcc";
    printf("  Splitting \"%s\" on '/':\n  ", path);
    char *part = strtok(path, "/");
    while (part) {
        printf("[%s] ", part);
        part = strtok(NULL, "/");
    }
    printf("\n");

    sub("strtok_r — reentrant version (thread-safe)");
    char data[] = "one:two:three";
    char *saveptr;              /* strtok_r uses this for state — not global */
    char *tok = strtok_r(data, ":", &saveptr);
    printf("  strtok_r tokens: ");
    while (tok) {
        printf("[%s] ", tok);
        tok = strtok_r(NULL, ":", &saveptr);
    }
    printf("\n");

    sub("Nested tokenization — ONLY possible with strtok_r");
    char outer[] = "a,b:c,d";
    char *sp_outer, *sp_inner;
    char *outer_tok = strtok_r(outer, ":", &sp_outer);
    while (outer_tok) {
        printf("  Outer '%s' → inner: ", outer_tok);
        char inner_copy[16];
        strncpy(inner_copy, outer_tok, sizeof(inner_copy)-1);
        inner_copy[sizeof(inner_copy)-1]='\0';
        char *inner_tok = strtok_r(inner_copy, ",", &sp_inner);
        while (inner_tok) {
            printf("[%s] ", inner_tok);
            inner_tok = strtok_r(NULL, ",", &sp_inner);
        }
        printf("\n");
        outer_tok = strtok_r(NULL, ":", &sp_outer);
    }

    /*
     * INTERVIEW NOTES:
     *   - strtok MODIFIES original string (replaces delimiters with \0)
     *   - strtok is NOT thread-safe (uses internal static state pointer)
     *   - Subsequent calls: pass NULL as first argument
     *   - strtok_r (POSIX): reentrant — pass &saveptr for state
     *   - Cannot nest strtok calls — use strtok_r for nested parsing
     *   - Make a copy before tokenizing if original must be preserved
     */
}

/* =============================================================================
 * ST14. sprintf / snprintf — FORMAT INTO BUFFER (MEDIUM)
 * =========================================================================== */
void st14_sprintf_snprintf(void)
{
    sec("ST14 [MEDIUM] sprintf / snprintf — FORMAT INTO BUFFER");

    sub("sprintf — no bounds check (DANGEROUS)");
    char buf1[32];
    sprintf(buf1, "Value=%d Float=%.2f", 42, 3.14f);
    printf("  sprintf: '%s'\n", buf1);
    /* sprintf(small_buf, "TooLong...") → OVERFLOW — no protection */
    printf("  sprintf with large input → BUFFER OVERFLOW (no bounds check)\n");

    sub("snprintf — bounded, always null-terminates");
    char buf2[16];
    int written = snprintf(buf2, sizeof(buf2), "Value=%d Float=%.2f", 42, 3.14159f);
    printf("  snprintf(buf[16]): '%s'\n", buf2);
    printf("  Would-be length: %d  Buffer: %zu  Truncated: %s\n",
           written, sizeof(buf2), written >= (int)sizeof(buf2) ? "YES":"NO");

    sub("Check truncation — return value of snprintf");
    char small[10];
    int len = snprintf(small, sizeof(small), "Hello, World!");
    printf("  snprintf(buf[10],\"Hello, World!\"):\n");
    printf("    Written to buf: '%s'\n", small);
    printf("    Return value  : %d  (chars that WOULD be written)\n", len);
    printf("    Truncated     : %s\n", len >= (int)sizeof(small) ? "YES":"NO");
    if (len >= (int)sizeof(small))
        printf("    Need buffer of at least %d bytes\n", len+1);

    sub("snprintf for number-to-string conversion");
    int   num  = 12345;
    float fnum = 3.14159f;
    char  num_str[32];
    char  flt_str[32];
    snprintf(num_str, sizeof(num_str), "%d", num);
    snprintf(flt_str, sizeof(flt_str), "%.4f", fnum);
    printf("  int→str: '%s'  float→str: '%s'\n", num_str, flt_str);

    sub("Building formatted messages — embedded log pattern");
    char log_msg[64];
    int  error_code = 0x1A;
    snprintf(log_msg, sizeof(log_msg), "[ERR 0x%02X] UART timeout at %s:%d",
             error_code, "uart.c", 142);
    printf("  Log: %s\n", log_msg);

    /*
     * INTERVIEW NOTES:
     *   - NEVER use sprintf for untrusted or variable-length input
     *   - snprintf: always use sizeof(buf) as size argument
     *   - snprintf return value: chars that WOULD be written (excl \0)
     *   - if (written >= sizeof(buf)): output was TRUNCATED
     *   - snprintf ALWAYS null-terminates (unlike strncpy)
     *   - In embedded: snprintf uses significant flash — check your libc
     */
}

/* =============================================================================
 * ST15. STRING TO NUMBER — atoi, strtol, strtoul (MEDIUM)
 * =========================================================================== */
void st15_string_to_number(void)
{
    sec("ST15 [MEDIUM] STRING TO NUMBER — atoi vs strtol (error handling)");

    sub("atoi — simple but NO error detection");
    printf("  atoi(\"42\")     = %d\n", atoi("42"));
    printf("  atoi(\"-100\")   = %d\n", atoi("-100"));
    printf("  atoi(\"abc\")    = %d  ← returns 0, no error indication!\n", atoi("abc"));
    printf("  atoi(\"42abc\")  = %d  ← stops at 'a', partial parse\n", atoi("42abc"));
    printf("  atoi overflow   = undefined behavior!\n");

    sub("strtol — full error handling (PREFERRED)");
    const char *inputs[] = {"42", "-100", "0xFF", "abc", "42abc",
                             "9999999999", "  77  "};
    for (int i = 0; i < 7; i++) {
        char *endptr;
        errno = 0;  /* reset before call */
        long val = strtol(inputs[i], &endptr, 0);  /* base 0 = auto-detect */
        if (errno == ERANGE) {
            printf("  strtol(\"%s\") → OVERFLOW\n", inputs[i]);
        } else if (endptr == inputs[i]) {
            printf("  strtol(\"%s\") → NO VALID DIGITS (endptr unchanged)\n",
                   inputs[i]);
        } else {
            printf("  strtol(\"%s\") = %ld  endptr='%s'%s\n",
                   inputs[i], val, endptr,
                   *endptr != '\0' ? " (partial)" : "");
        }
    }

    sub("strtoul — unsigned, with base selection");
    printf("  strtoul(\"0xFF\",NULL,16) = %lu\n", strtoul("0xFF",  NULL, 16));
    printf("  strtoul(\"0b1010\",e,2)  = %lu\n",  strtoul("1010",  NULL, 2));
    printf("  strtoul(\"077\",NULL,8)  = %lu\n",  strtoul("077",   NULL, 8));

    sub("strtof / strtod — float/double from string");
    char *end;
    double d = strtod("3.14159 extra", &end);
    printf("  strtod(\"3.14159 extra\") = %.5f  endptr='%s'\n", d, end);

    /*
     * INTERVIEW NOTES:
     *   - atoi: simple, NO error detection — avoid in production
     *   - strtol(str, &endptr, base): full control
     *     endptr → first char NOT converted (check for error)
     *     base 0 → auto (0x=hex, 0=octal, else decimal)
     *   - Error checks: errno==ERANGE (overflow), endptr==str (no digits)
     *   - strtoul for unsigned, strtoll for long long
     *   - In embedded: careful with errno — may not be supported on all libc
     */
}

/* =============================================================================
 * ST16. NUMBER TO STRING — sprintf pattern (MEDIUM)
 * =========================================================================== */
void st16_number_to_string(void)
{
    sec("ST16 [MEDIUM] NUMBER TO STRING — sprintf/snprintf patterns");

    char buf[32];

    sub("Integer conversions");
    snprintf(buf, sizeof(buf), "%d",   42);        printf("  decimal: '%s'\n", buf);
    snprintf(buf, sizeof(buf), "%05d", 42);        printf("  padded : '%s'\n", buf);
    snprintf(buf, sizeof(buf), "%x",   255);       printf("  hex    : '%s'\n", buf);
    snprintf(buf, sizeof(buf), "%X",   255);       printf("  HEX    : '%s'\n", buf);
    snprintf(buf, sizeof(buf), "0x%08X", 0xDEAD);  printf("  0x fmt : '%s'\n", buf);
    snprintf(buf, sizeof(buf), "%o",   8);         printf("  octal  : '%s'\n", buf);

    sub("Float conversions");
    snprintf(buf, sizeof(buf), "%f",     3.14159f); printf("  %%f     : '%s'\n", buf);
    snprintf(buf, sizeof(buf), "%.2f",   3.14159f); printf("  %%.2f   : '%s'\n", buf);
    snprintf(buf, sizeof(buf), "%e",     12345.6f); printf("  %%e     : '%s'\n", buf);
    snprintf(buf, sizeof(buf), "%g",     0.0001f);  printf("  %%g     : '%s'\n", buf);

    sub("itoa — non-standard (not in C standard library)");
    printf("  itoa() is NOT in C standard — use snprintf instead\n");
    printf("  Portable: snprintf(buf, sizeof(buf), \"%%d\", num)\n");

    /*
     * INTERVIEW NOTES:
     *   - snprintf is the universal number→string tool
     *   - itoa() exists in MSVC and some embedded libc — NOT portable
     *   - Check return value: if >= sizeof(buf), result was truncated
     *   - For hex registers in embedded: "0x%08X" → "0x0000DEAD" style
     *   - %g: chooses shorter of %f or %e automatically
     */
}

/* =============================================================================
 * ST17. STRING REVERSAL — IN-PLACE TWO-POINTER (MEDIUM)
 * =========================================================================== */
void st17_string_reversal(void)
{
    sec("ST17 [MEDIUM] STRING REVERSAL — IN-PLACE TWO-POINTER");

    sub("In-place reversal — two-pointer approach");
    char str1[] = "Embedded";
    int l = 0, r = (int)strlen(str1) - 1;
    printf("  Before: '%s'\n", str1);
    while (l < r) {
        char tmp = str1[l];
        str1[l]  = str1[r];
        str1[r]  = tmp;
        l++; r--;
    }
    printf("  After : '%s'\n", str1);

    sub("Reversal using pointer arithmetic");
    char str2[] = "ZephyrRTOS";
    char *left  = str2;
    char *right = str2 + strlen(str2) - 1;
    printf("  Before: '%s'\n", str2);
    while (left < right) {
        char tmp = *left;
        *left++  = *right;
        *right-- = tmp;
    }
    printf("  After : '%s'\n", str2);

    sub("Cannot reverse string literal — must use array");
    /* const char *lit = "Hello"; reverse(lit) → UB (read-only memory) */
    printf("  const char *lit = \"Hello\"; → CANNOT reverse (read-only)\n");
    printf("  Must: char arr[] = \"Hello\"; then reverse arr\n");

    sub("Reverse words in sentence");
    char sentence[] = "Hello World Embedded";
    int len = (int)strlen(sentence);
    printf("  Before word-rev: '%s'\n", sentence);
    /* Step 1: reverse entire string */
    l=0; r=len-1;
    while(l<r){char t=sentence[l];sentence[l++]=sentence[r];sentence[r--]=t;}
    /* Step 2: reverse each word */
    int start=0;
    for(int i=0;i<=len;i++){
        if(sentence[i]==' '||sentence[i]=='\0'){
            int e=i-1;
            while(start<e){
                char t=sentence[start];
                sentence[start++]=sentence[e];
                sentence[e--]=t;
            }
            start=i+1;
        }
    }
    printf("  After  word-rev: '%s'\n", sentence);

    /*
     * INTERVIEW NOTES:
     *   - Two-pointer O(n) reversal: l from left, r from right, swap until l>=r
     *   - Use array (char arr[]) not pointer to literal — must be modifiable
     *   - XOR swap: a^=b; b^=a; a^=b; works but harder to read — avoid
     *   - Reverse words: reverse all, then reverse each word individually
     *   - strlen called once — don't call inside loop (O(n) each time)
     */
}

/* =============================================================================
 * ST18. STRING AS FUNCTION PARAMETER — const char* vs char* (MEDIUM)
 * =========================================================================== */

/* Read-only — takes const: can accept both const and non-const strings */
size_t my_strlen(const char *s)
{
    const char *p = s;
    while (*p) p++;
    return p - s;
}

/* Modifies in-place — takes char* (non-const) */
void to_upper(char *s)
{
    while (*s) {
        if (*s >= 'a' && *s <= 'z') *s -= 32;  /* ASCII: 'a'-'A'=32 */
        s++;
    }
}

/* Returns newly allocated string — caller must free */
char *my_strdup(const char *src)
{
    char *copy = malloc(strlen(src) + 1);  /* +1 for \0 */
    if (copy) strcpy(copy, src);
    return copy;
}

void st18_string_as_parameter(void)
{
    sec("ST18 [MEDIUM] STRING AS FUNCTION PARAMETER — const char* vs char*");

    sub("const char* — read-only, more flexible (accepts both const & non-const)");
    char arr[]        = "Hello";
    const char *lit   = "World";
    printf("  my_strlen(arr) = %zu\n", my_strlen(arr));  /* non-const → const OK */
    printf("  my_strlen(lit) = %zu\n", my_strlen(lit));  /* const → const OK */

    sub("char* — modifiable, less flexible (cannot accept const)");
    char mutable[] = "hello world";
    to_upper(mutable);
    printf("  to_upper: '%s'\n", mutable);
    /* to_upper(lit) → WARNING/ERROR: passing const char* to char* drops const */

    sub("Returning heap string — caller must free");
    char *dup = my_strdup("Embedded");
    if (dup) {
        printf("  my_strdup: '%s'  @ %p\n", dup, (void*)dup);
        free(dup);
        dup = NULL;
    }

    sub("Parameter const-correctness rules");
    printf("  const char* param: accepts both const and non-const strings\n");
    printf("  char*       param: accepts ONLY non-const strings\n");
    printf("  Passing const char* to char* param: compiler warning/error\n");
    printf("  Rule: if function reads only → use const char*\n");
    printf("        if function modifies  → use char*\n");

    /*
     * INTERVIEW NOTES:
     *   - const char* parameter: function promises not to modify string
     *   - Prefer const char* for read-only operations — more flexible
     *   - Passing non-const to const: always safe (widening)
     *   - Passing const to non-const: WARNING (narrowing — drops const-ness)
     *   - Functions returning heap string: document that caller must free()
     *   - strdup() is POSIX — not in C standard; implement your own if needed
     */
}

/* =============================================================================
 * ST19. memset / memcpy / memmove ON STRINGS (LOW)
 * =========================================================================== */
void st19_mem_functions(void)
{
    sec("ST19 [LOW] memset / memcpy / memmove ON STRINGS");

    sub("memset — fill buffer with a byte value");
    char buf[16];
    memset(buf, 0, sizeof(buf));        /* zero entire buffer */
    printf("  memset(buf,0): buf[0]=0x%02X  (safe null-terminated empty string)\n",
           (unsigned char)buf[0]);

    memset(buf, '*', sizeof(buf)-1);    /* fill with '*' */
    buf[sizeof(buf)-1] = '\0';
    printf("  memset(buf,'*'): '%s'\n", buf);

    memset(buf, 'A', 5); buf[5]='\0';
    printf("  memset first 5 as 'A': '%s'\n", buf);

    sub("memcpy — copy bytes (no overlap allowed)");
    char src[] = "Embedded Systems";
    char dst[32];
    memcpy(dst, src, strlen(src)+1);   /* +1 to include \0 */
    printf("  memcpy src→dst: '%s'\n", dst);

    sub("memmove — copy bytes (overlap-safe)");
    char overlap[] = "Hello World";
    printf("  Before memmove overlap: '%s'\n", overlap);
    memmove(overlap+6, overlap, 5);    /* "Hello" overlaps — memmove handles it */
    overlap[5] = ' ';
    printf("  After  memmove overlap: '%s'\n", overlap);

    sub("memcpy vs memmove — when to use which");
    printf("  memcpy:  src and dst must NOT overlap — faster\n");
    printf("  memmove: handles overlapping regions — safer\n");
    printf("  Rule: when in doubt, use memmove (small/no perf difference)\n");
    printf("  memcpy with overlap = UNDEFINED BEHAVIOR\n");

    /*
     * INTERVIEW NOTES:
     *   - memset fills each BYTE with value (not each int element)
     *   - memset(arr, 1, n): each byte=1 → int reads as 0x01010101, not 1
     *   - memset(arr, 0, n): correct for zero-init (0 = all zero bits)
     *   - memcpy: undefined if regions overlap — use memmove
     *   - memmove: correct even if src and dst overlap — uses temp buffer
     *   - Always +1 in memcpy for strings to include the \0 terminator
     */
}

/* =============================================================================
 * ST20. STRING COMPARISON PITFALL — == vs strcmp (LOW)
 * =========================================================================== */
void st20_comparison_pitfall(void)
{
    sec("ST20 [LOW] STRING COMPARISON PITFALL — == compares ADDRESSES, not content");

    const char *s1 = "Hello";
    const char *s2 = "Hello";
    char        arr1[] = "Hello";
    char        arr2[] = "Hello";

    sub("== compares POINTER VALUES (addresses), not string content");
    /* s1 == s2 may be true OR false — compiler may or may not merge literals */
    printf("  s1 == s2 (literal ptrs): %s  ← UNRELIABLE (compiler-dependent)\n",
           (s1 == s2) ? "true" : "false");
    printf("  arr1==arr2 (array addrs): %s  ← ALWAYS false (different arrays)\n",
           (void*)arr1==(void*)arr2 ? "true" : "false");

    sub("strcmp — compares CONTENT (correct way)");
    printf("  strcmp(s1,s2)      = %d  (0 = equal content)\n",  strcmp(s1,s2));
    printf("  strcmp(arr1,arr2)  = %d  (0 = equal content)\n",  strcmp(arr1,arr2));
    printf("  strcmp(s1,\"World\") = %d  (non-zero = different)\n",strcmp(s1,"World"));

    sub("Classic bug — using == for string comparison");
    char input[] = "quit";
    /* if (input == "quit") — ALWAYS FALSE — compares address to literal address */
    if (strcmp(input, "quit") == 0)   /* CORRECT */
        printf("  strcmp: 'quit' matched correctly\n");
    printf("  input==\"quit\" would always be FALSE (address comparison)\n");

    /*
     * INTERVIEW NOTES:
     *   - == on char* compares POINTER addresses — almost never what you want
     *   - Two identical string literals MAY have same address (compiler merging)
     *   - Two char arrays ALWAYS have different addresses
     *   - ALWAYS use strcmp() to compare string content
     *   - Common bug: if(str == "expected") — silently always false
     */
}

/* =============================================================================
 * ST21. MULTI-LINE STRING LITERALS — IMPLICIT CONCATENATION (LOW)
 * =========================================================================== */
void st21_multiline_strings(void)
{
    sec("ST21 [LOW] MULTI-LINE STRING LITERALS — IMPLICIT CONCATENATION");

    sub("Adjacent string literals — compiler concatenates at compile time");
    const char *msg = "Hello, "
                      "Embedded "
                      "World!";     /* single string at runtime */
    printf("  Concatenated: '%s'\n", msg);

    sub("Long strings split across lines for readability");
    const char *sql = "SELECT name, age "
                      "FROM employees "
                      "WHERE age > 30 "
                      "ORDER BY name;";
    printf("  SQL: '%s'\n", sql);

    sub("Backslash continuation — alternative (fragile, whitespace-sensitive)");
    const char *bs = "Line1 \
Line2 \
Line3";     /* backslash-newline: continuation — no newline in string */
    printf("  Backslash: '%s'\n", bs);

    sub("String with embedded newlines");
    const char *multiline = "Line 1\n"
                            "Line 2\n"
                            "Line 3\n";
    printf("  With \\n:\n%s", multiline);

    /*
     * INTERVIEW NOTES:
     *   - "A" "B" "C" → "ABC" — compiler concatenates adjacent literals
     *   - No operator between them — just whitespace/newline between quotes
     *   - Useful for long strings, SQL, JSON templates, error messages
     *   - Backslash continuation: fragile — trailing space after \ = syntax error
     *   - Prefer adjacent literal approach over backslash continuation
     */
}

/* =============================================================================
 * ST22. WIDE STRINGS — wchar_t (LOW)
 * =========================================================================== */
void st22_wide_strings(void)
{
    sec("ST22 [LOW] WIDE STRINGS — wchar_t, L\"hello\"");

    sub("wchar_t — wide character type for Unicode/multibyte");
    wchar_t wc = L'A';
    wchar_t ws[] = L"Hello Wide";

    printf("  sizeof(char)   = %zu  (1 byte)\n", sizeof(char));
    printf("  sizeof(wchar_t)= %zu  (2 or 4 bytes — platform dependent)\n",
           sizeof(wchar_t));
    printf("  sizeof(ws)     = %zu  (11 wchars × sizeof(wchar_t))\n", sizeof(ws));
    printf("  wcslen(ws)     = %zu  (wide char count excl null)\n", wcslen(ws));

    /* Print wide string */
    wprintf(L"  Wide string: %ls\n", ws);
    printf("  wchar_t wc='A' value = %d\n", (int)wc);

    sub("Wide string functions — parallel to char string functions");
    printf("  strlen  → wcslen\n");
    printf("  strcpy  → wcscpy\n");
    printf("  strcmp  → wcscmp\n");
    printf("  strcat  → wcscat\n");
    printf("  sprintf → swprintf\n");

    sub("When to use wide strings");
    printf("  Use wchar_t: Unicode text, internationalization (i18n)\n");
    printf("  Use char*  : ASCII, embedded systems, protocol strings\n");
    printf("  Most embedded: char* with UTF-8 encoding (no wide strings)\n");
    printf("  Windows API: wchar_t everywhere (UTF-16)\n");
    printf("  Linux/POSIX: char* with UTF-8 (4 bytes per char in UTF-8)\n");

    /*
     * INTERVIEW NOTES:
     *   - wchar_t: 2 bytes on Windows (UTF-16), 4 bytes on Linux (UTF-32)
     *   - L"hello": wide string literal — each char is wchar_t
     *   - wcslen: wide char count (not bytes) — excludes null terminator
     *   - In embedded: rarely used — UTF-8 in char* is more common
     *   - Include <wchar.h> for wcslen, wcscpy, etc.
     *   - C11 added char16_t and char32_t for portable Unicode
     */
}

/* ╔══════════════════════════════════════════════════════════════════════════╗
 * ║                      PREPROCESSOR DIRECTIVES                            ║
 * ╚══════════════════════════════════════════════════════════════════════════╝ */

/* =============================================================================
 * PP1. #define — OBJECT-LIKE MACRO (CONSTANTS) — HIGH
 * =========================================================================== */

/* Object-like macros — text substitution, no memory, no type */
#define MAX_BUFFER_SIZE    256
#define PI                 3.14159f
#define VERSION_MAJOR      2
#define VERSION_MINOR      5
#define BAUD_RATE          115200UL     /* UL suffix — unsigned long */
#define DEBUG_ENABLED      1
#define APP_NAME           "EmbeddedApp"
#define NEWLINE            '\n'

/* Computed constant — expression, computed each use */
#define VERSION_STRING     (VERSION_MAJOR * 100 + VERSION_MINOR)
/* Always parenthesize computed macros! */

void pp1_object_like_macro(void)
{
    sec("PP1 [HIGH] #define — OBJECT-LIKE MACRO (CONSTANTS)");

    printf("  MAX_BUFFER_SIZE = %d\n",    MAX_BUFFER_SIZE);
    printf("  PI              = %.5f\n",  PI);
    printf("  BAUD_RATE       = %lu\n",   BAUD_RATE);
    printf("  APP_NAME        = %s\n",    APP_NAME);
    printf("  VERSION_STRING  = %d\n",    VERSION_STRING);

    char buf[MAX_BUFFER_SIZE];          /* MAX_BUFFER_SIZE replaced by 256 */
    snprintf(buf, sizeof(buf), "%s v%d.%d", APP_NAME, VERSION_MAJOR, VERSION_MINOR);
    printf("  buf: '%s'\n", buf);

    printf("\n  Object-like macro properties:\n");
    printf("    No memory allocated — pure text substitution\n");
    printf("    No type — any 'value' substituted as text\n");
    printf("    No scope — visible from definition to end of file (or #undef)\n");
    printf("    Evaluated at PREPROCESSOR stage, before compilation\n");
    printf("    const int preferred for typed constants in C++\n");

    /*
     * INTERVIEW NOTES:
     *   - #define has NO type — compiler sees raw value after substitution
     *   - const int: has type, scope, and debugger visibility — preferred in C++
     *   - UL suffix (BAUD_RATE): ensures unsigned long — prevents overflow
     *   - Computed macro: (expr) always parenthesized to prevent precedence bugs
     *   - No semicolon at end of #define — adding one includes semicolon in expansion
     */
}

/* =============================================================================
 * PP2. #define — FUNCTION-LIKE MACRO + PARENTHESIZATION RULE — HIGH
 * =========================================================================== */

/* WRONG — missing parentheses around args and result */
#define BAD_MUL(a, b)    a * b        /* fails: BAD_MUL(2+3,4) = 2+3*4=14 */
#define BAD_SQ(x)        x * x        /* fails: BAD_SQ(i++) = i++*i++ (double incr) */

/* CORRECT — every arg and whole expression parenthesized */
#define MUL(a, b)        ((a) * (b))
#define SQ(x)            ((x) * (x))  /* still has double-eval problem! */
#define ABS(x)           ((x) < 0 ? -(x) : (x))
#define MIN(a, b)        ((a) < (b) ? (a) : (b))
#define MAX(a, b)        ((a) > (b) ? (a) : (b))
#define CLAMP(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))
#define IS_UPPER(c)      ((c) >= 'A' && (c) <= 'Z')
#define TO_LOWER(c)      ((c) >= 'A' && (c) <= 'Z' ? (c) + 32 : (c))
#define ARRAY_SIZE(a)    (sizeof(a) / sizeof((a)[0]))

void pp2_function_like_macro(void)
{
    sec("PP2 [HIGH] FUNCTION-LIKE MACRO + PARENTHESIZATION RULE");

    printf("  MUL(2+3, 4)  = %d  (correct: ((2+3)*(4)) = 20)\n",  MUL(2+3, 4));
    printf("  BAD_MUL(2+3,4)=%d  (wrong  : 2+3*4 = 14)\n",        BAD_MUL(2+3, 4));

    printf("  ABS(-7)      = %d\n", ABS(-7));
    printf("  MIN(3,7)     = %d\n", MIN(3, 7));
    printf("  MAX(3,7)     = %d\n", MAX(3, 7));
    printf("  CLAMP(15,0,10)=%d\n", CLAMP(15, 0, 10));
    printf("  IS_UPPER('A')= %d\n", IS_UPPER('A'));
    printf("  TO_LOWER('B')= %c\n", TO_LOWER('B'));

    int arr[] = {1, 2, 3, 4, 5};
    printf("  ARRAY_SIZE   = %zu\n", ARRAY_SIZE(arr));

    printf("\n  Parenthesization RULES:\n");
    printf("    Rule 1: wrap EACH argument:  #define F(x) (x)*(x)\n");
    printf("    Rule 2: wrap WHOLE result:   #define F(x) ((x)*(x))\n");
    printf("    Both needed:  MUL(2+3,4) → BAD_MUL gives 14, MUL gives 20\n");

    printf("\n  Side effect problem — x evaluated TWICE:\n");
    int i = 3;
    /* SQ(i++) → ((i++) * (i++)) — i incremented TWICE — UB! */
    printf("  SQ(i++) where i=3 → UB (i incremented twice)\n");
    printf("  Fix: use static inline function instead of macro for this\n");
    int j = 3;
    printf("  Before: j=%d\n", j);
    int sq = SQ(j);   /* SQ(j) — no side effect here, j not modified */
    printf("  SQ(j)=%d j=%d (safe — no ++ in argument)\n", sq, j);

    /*
     * INTERVIEW NOTES:
     *   - Rule: parenthesize ALL args AND entire expression
     *   - Side effects: args evaluated MULTIPLE TIMES — never pass i++, func()
     *   - Function-like macro looks like function but has NO type check
     *   - Use static inline instead when args have side effects or type matters
     *   - ARRAY_SIZE macro: only safe at declaration site (not after decay)
     */
}

/* =============================================================================
 * PP3. #include — <> vs "" AND SEARCH ORDER — HIGH
 * =========================================================================== */
void pp3_include(void)
{
    sec("PP3 [HIGH] #include — <> vs \"\" AND INCLUDE SEARCH ORDER");

    printf("  Syntax:\n");
    printf("    #include <stdio.h>   ← system header — angle brackets\n");
    printf("    #include \"myfile.h\" ← user header — double quotes\n\n");

    printf("  Search order for #include <file>:\n");
    printf("    1. Standard system include directories (/usr/include, etc.)\n");
    printf("    2. Additional dirs from -I compiler flag\n");
    printf("    3. Does NOT search current directory\n\n");

    printf("  Search order for #include \"file\":\n");
    printf("    1. Directory of the file containing #include\n");
    printf("    2. Additional dirs from -I compiler flag\n");
    printf("    3. Standard system include directories\n\n");

    printf("  Best practices:\n");
    printf("    Always use <> for system/library headers\n");
    printf("    Always use \"\" for your own project headers\n");
    printf("    -I./include: add project include dir to search path\n");
    printf("    #include <mylib/sensor.h>: path in angle brackets\n\n");

    printf("  What preprocessor does:\n");
    printf("    Literally copies content of header into .c file\n");
    printf("    That's why include guards matter — prevent double inclusion\n");
    printf("    Header guards prevent re-expanding on multiple #includes\n");

    /*
     * INTERVIEW NOTES:
     *   - <> and "" differ in SEARCH ORDER, not in what can be included
     *   - "" searches current dir first — useful for project headers
     *   - -I flag adds paths searched for both <> and "" includes
     *   - #include works by textual insertion — no magic, no modules (pre-C20)
     *   - Circular includes: A includes B, B includes A → infinite loop
     *     prevented by include guards
     */
}

/* =============================================================================
 * PP4. #ifdef / #ifndef / #endif — HIGH
 * =========================================================================== */

/* These would normally be in command line: gcc -DDEBUG_ENABLED -DPLATFORM_ARM */
#define DEBUG_MODE
#define PLATFORM_ARM
/* #define FEATURE_BLE  -- intentionally not defined */

void pp4_ifdef_ifndef(void)
{
    sec("PP4 [HIGH] #ifdef / #ifndef / #endif");

    printf("  #ifdef — compile block if macro IS defined:\n");
#ifdef DEBUG_MODE
    printf("    DEBUG_MODE is defined → debug code compiled\n");
#endif

#ifdef FEATURE_BLE
    printf("    FEATURE_BLE is defined\n");
#else
    printf("    FEATURE_BLE is NOT defined → feature excluded\n");
#endif

    printf("\n  #ifndef — compile block if macro IS NOT defined:\n");
#ifndef RELEASE_BUILD
    printf("    RELEASE_BUILD not defined → extra checks enabled\n");
#endif

    printf("\n  Platform selection pattern:\n");
#ifdef PLATFORM_ARM
    printf("    Platform: ARM — using ARM-specific code path\n");
#elif defined(PLATFORM_X86)
    printf("    Platform: x86 — using x86-specific code path\n");
#else
    printf("    Platform: UNKNOWN — using generic fallback\n");
#endif

    printf("\n  Common uses:\n");
    printf("    Debug logging     : #ifdef DEBUG → printf(...) #endif\n");
    printf("    Feature toggles   : #ifdef FEATURE_X → include code #endif\n");
    printf("    Platform/compiler : #ifdef __linux__ / #ifdef _WIN32\n");
    printf("    Include guards    : #ifndef HEADER_H / #define HEADER_H / #endif\n");

    /*
     * INTERVIEW NOTES:
     *   - #ifdef X: true if X is defined, even as empty: #define X
     *   - #if defined(X): equivalent to #ifdef X but can be combined: #if defined(A) && defined(B)
     *   - Defined via command line: gcc -DDEBUG_MODE (equivalent to #define DEBUG_MODE)
     *   - #ifdef 0 is NOT the same as #if 0 (0 is not a macro name)
     *   - Use #if 0 to comment out code blocks (handles nested comments)
     */
}

/* =============================================================================
 * PP5. #if / #elif / #else — EXPRESSION-BASED CONDITIONAL — HIGH
 * =========================================================================== */

#define VERSION  3
#define USE_DMA  1
#define LOG_LEVEL 2   /* 0=none, 1=error, 2=warn, 3=info, 4=debug */

void pp5_if_elif_else(void)
{
    sec("PP5 [HIGH] #if / #elif / #else — EXPRESSION-BASED CONDITIONAL");

    printf("  #if checks CONSTANT EXPRESSIONS (not runtime values):\n\n");

    printf("  Version check:\n");
#if VERSION >= 3
    printf("    VERSION=%d: using new API (>= 3)\n", VERSION);
#elif VERSION == 2
    printf("    VERSION=%d: using v2 API\n", VERSION);
#else
    printf("    VERSION=%d: legacy mode\n", VERSION);
#endif

    printf("\n  Feature flag:\n");
#if USE_DMA == 1
    printf("    DMA enabled — using DMA transfer\n");
#else
    printf("    DMA disabled — using CPU copy\n");
#endif

    printf("\n  Log level — conditional logging macros:\n");
#if LOG_LEVEL >= 1
    printf("    ERROR logging: ENABLED\n");
#endif
#if LOG_LEVEL >= 2
    printf("    WARN  logging: ENABLED\n");
#endif
#if LOG_LEVEL >= 3
    printf("    INFO  logging: ENABLED\n");
#endif
#if LOG_LEVEL >= 4
    printf("    DEBUG logging: ENABLED\n");
#endif

    printf("\n  Difference #if vs #ifdef:\n");
    printf("    #ifdef X      : true if X is defined (any value, even 0)\n");
    printf("    #if X         : true if X expands to non-zero\n");
    printf("    #if defined(X): same as #ifdef, but composable\n");
    printf("    #define X 0; #ifdef X → TRUE; #if X → FALSE\n");

    printf("\n  #if 0 — disable code block (better than block comment):\n");
#if 0
    printf("  This block is EXCLUDED from compilation\n");
    int x = 10;  /* can have C comments inside: /* nested ok */ */
#endif
    printf("    #if 0 block above was excluded from compilation\n");

    /*
     * INTERVIEW NOTES:
     *   - #if evaluates constant integer expressions
     *   - #define X 0 then #if X: false (X=0). #ifdef X: true (X defined)
     *   - #if 0: reliable way to disable large code blocks
     *   - #if defined(A) && defined(B): combine multiple checks
     *   - All expressions must be evaluable at PREPROCESSING time (no variables)
     */
}

/* =============================================================================
 * PP6. #pragma once vs INCLUDE GUARD — HIGH
 * =========================================================================== */
void pp6_pragma_once_vs_include_guard(void)
{
    sec("PP6 [HIGH] #pragma once vs INCLUDE GUARD (#ifndef pattern)");

    printf("  Problem: header included multiple times → duplicate definitions\n\n");

    printf("  Solution 1: Traditional include guard (#ifndef):\n");
    printf("    // myheader.h\n");
    printf("    #ifndef MYHEADER_H          ← if not defined yet\n");
    printf("    #define MYHEADER_H          ← define it (mark as included)\n");
    printf("    // ... header content ...  \n");
    printf("    #endif                      ← end of guard\n\n");

    printf("  Solution 2: #pragma once (modern, simpler):\n");
    printf("    // myheader.h\n");
    printf("    #pragma once               ← compiler ensures included only once\n");
    printf("    // ... header content ... \n\n");

    printf("  Comparison:\n");
    printf("    %-30s | %-20s | %-20s\n",
           "Aspect", "#ifndef guard", "#pragma once");
    printf("    %-30s | %-20s | %-20s\n",
           "------------------------------",
           "--------------------", "--------------------");
    printf("    %-30s | %-20s | %-20s\n",
           "C standard compliance",   "YES (C89+)",      "NO (compiler ext)");
    printf("    %-30s | %-20s | %-20s\n",
           "Portability",            "Universal",        "GCC/Clang/MSVC OK");
    printf("    %-30s | %-20s | %-20s\n",
           "Verbosity",              "3 lines",          "1 line");
    printf("    %-30s | %-20s | %-20s\n",
           "Name collision risk",    "YES (bad name)",   "No (file-based)");
    printf("    %-30s | %-20s | %-20s\n",
           "Copy-paste header risk", "YES",              "No");

    printf("\n  Best practice:\n");
    printf("    Embedded/portable: use #ifndef guard — universal\n");
    printf("    Modern projects (GCC/Clang): #pragma once is fine\n");
    printf("    Guard name: FILENAME_H_ or PROJECT_FILENAME_H\n");
    printf("    Avoid: common names like HEADER_H — collision risk\n");

    /*
     * INTERVIEW NOTES:
     *   - Include guard: standard C, portable, slightly more verbose
     *   - #pragma once: compiler extension, simpler, all major compilers support
     *   - MISRA C: recommends include guards (not #pragma once)
     *   - Guard macro name must be unique — use path-based names
     *   - Both prevent multiple inclusion but not multiple inclusion from copy
     */
}

/* =============================================================================
 * PP7. MACRO PITFALL — SIDE EFFECTS (i++ evaluated twice) — HIGH
 * =========================================================================== */

/* These macros have the side-effect problem */
#define BAD_MAX(a, b)  ((a) > (b) ? (a) : (b))  /* evaluates winner TWICE */
#define BAD_ABS(x)     ((x) < 0 ? -(x) : (x))   /* evaluates x TWICE */

/* Fix: use static inline — evaluated exactly once, type-checked */
static inline int safe_max(int a, int b) { return a > b ? a : b; }
static inline int safe_abs(int x) { return x < 0 ? -x : x; }

void pp7_macro_side_effects(void)
{
    sec("PP7 [HIGH] MACRO PITFALL — SIDE EFFECTS (arg evaluated twice)");

    sub("BAD_MAX(i++, j++) — i or j incremented TWICE");
    int i = 3, j = 5;
    printf("  Before: i=%d j=%d\n", i, j);
    int m = BAD_MAX(i++, j++);
    /* Expands to: ((i++) > (j++) ? (i++) : (j++)) */
    /* j is greater so: j++ evaluated TWICE → j incremented twice */
    printf("  BAD_MAX(i++,j++): result=%d  i=%d j=%d  (j incremented TWICE!)\n",
           m, i, j);

    sub("Fix: static inline function — args evaluated EXACTLY ONCE");
    i = 3; j = 5;
    printf("  Before: i=%d j=%d\n", i, j);
    m = safe_max(i++, j++);  /* i++ and j++ each evaluated EXACTLY once */
    printf("  safe_max(i++,j++): result=%d  i=%d j=%d  (correct)\n", m, i, j);

    sub("BAD_ABS(x++) — x incremented twice");
    int x = -3;
    printf("  x=%d\n", x);
    int a = BAD_ABS(x++);
    /* Expands to: ((x++) < 0 ? -(x++) : (x++)) — x incremented twice! */
    printf("  BAD_ABS(x++): result=%d x=%d (x incremented twice)\n", a, x);

    sub("Other dangerous macro arguments");
    printf("  BAD_MAX(func(),func()): func() called TWICE — side effects!\n");
    printf("  BAD_MAX(arr[i++],arr[j++]): index incremented twice\n");
    printf("  RULE: NEVER pass expressions with side effects to macros\n");

    /*
     * INTERVIEW NOTES:
     *   - Macro args: textually substituted — if arg appears N times, evaluated N times
     *   - MAX(a,b): the winner (larger) is evaluated TWICE in most implementations
     *   - Fix 1: static inline function (best — type-safe, no side-effect)
     *   - Fix 2: GCC typeof extension: #define MAX(a,b) ({typeof(a) _a=(a), _b=(b); ...})
     *   - Rule: if arg appears twice in expansion, NEVER pass expr with side effects
     *   - Classic interview: "What's wrong with #define SQ(x) x*x?" → two issues
     */
}

/* =============================================================================
 * PP8. MACRO PITFALL — MISSING PARENTHESES (PRECEDENCE) — HIGH
 * =========================================================================== */

/* All the wrong ways */
#define WRONG1(x)    x * 2           /* missing parens around x */
#define WRONG2(a, b) a + b           /* missing parens around result */
#define WRONG3(a, b) (a) + (b)       /* missing outer parens */

/* Correct */
#define RIGHT(a, b)  ((a) + (b))     /* both args AND result parenthesized */

void pp8_missing_parentheses(void)
{
    sec("PP8 [HIGH] MACRO PITFALL — MISSING PARENTHESES (PRECEDENCE)");

    sub("Missing parens around argument");
    printf("  #define WRONG1(x) x * 2\n");
    printf("  WRONG1(3+4) → 3+4*2 = 3+8 = %d  (expected 14)\n", WRONG1(3+4));
    printf("  #define FIXED(x) ((x)*2)\n");
    printf("  FIXED(3+4)  → ((3+4)*2) = %d  (correct)\n",  ((3+4)*2));

    sub("Missing parens around result");
    printf("  #define WRONG2(a,b) a + b\n");
    printf("  WRONG2(2,3)*4 → 2+3*4 = 2+12 = %d  (expected 20)\n", WRONG2(2,3)*4);
    printf("  #define RIGHT(a,b) ((a)+(b))\n");
    printf("  RIGHT(2,3)*4  → ((2)+(3))*4 = %d  (correct)\n",  RIGHT(2,3)*4);

    sub("Unary minus surprise");
    int val = 5;
    /* #define NEG(x) -x   →   -NEG(val+1) becomes --val+1 (predecrement!) */
    printf("  #define NEG(x) -x\n");
    printf("  -NEG(val+1) → --val+1 = %d (predecrement bug!)\n", --val+1);
    val = 5; /* restore */
    printf("  #define NEG(x) (-(x))  →  -(-(val+1)) = %d (correct)\n", (-(val+1)));

    sub("Correct pattern — memorize this");
    printf("  #define F(a, b)   ((a) op (b))\n");
    printf("                     ^^^  ^^^     ← wrap each arg\n");
    printf("                    ^         ^  ← wrap entire result\n");

    /*
     * INTERVIEW NOTES:
     *   - Two rules: wrap each arg in (), wrap entire expansion in ()
     *   - Without arg parens: WRONG1(a+b) = a+b*2 not (a+b)*2
     *   - Without result parens: WRONG2(a,b)*c = a+b*c not (a+b)*c
     *   - Unary operators: -MACRO(x) can become --x if expansion starts with -
     *   - Interview: spot the bug in #define SQ(x) x*x — two bugs!
     *     1. No outer parens 2. Double evaluation of x
     */
}

/* =============================================================================
 * PP9. PREDEFINED MACROS — HIGH
 * =========================================================================== */
void pp9_predefined_macros(void)
{
    sec("PP9 [HIGH] PREDEFINED MACROS — __FILE__ __LINE__ __func__ etc.");

    printf("  __FILE__   = \"%s\"\n", __FILE__);
    printf("  __LINE__   = %d\n",     __LINE__);
    printf("  __func__   = \"%s\"\n", __func__);
    printf("  __DATE__   = \"%s\"\n", __DATE__);
    printf("  __TIME__   = \"%s\"\n", __TIME__);
    printf("  __STDC__   = %d  (1 = conforming C implementation)\n", __STDC__);

#ifdef __STDC_VERSION__
    printf("  __STDC_VERSION__ = %ld  (C standard version)\n", __STDC_VERSION__);
    /* 199901L = C99,  201112L = C11,  201710L = C17 */
#endif

    printf("\n  Practical: debug/logging macros using predefined macros:\n");

/* Debug log macro — includes file, line, function automatically */
#define LOG_DEBUG(fmt, ...) \
    printf("[%s:%d %s] " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define ASSERT_MSG(cond, msg) \
    do { if (!(cond)) { \
        printf("ASSERT FAILED: %s | %s:%d | %s\n", \
               msg, __FILE__, __LINE__, __func__); \
    }} while(0)

    LOG_DEBUG("Sensor value = %d", 42);
    LOG_DEBUG("Initialization complete");

    ASSERT_MSG(1 == 1, "Math is broken");
    ASSERT_MSG(1 == 2, "One should equal two");  /* triggers */

    printf("\n  __func__ vs __FUNCTION__:\n");
    printf("    __func__    : C99 standard — use this\n");
    printf("    __FUNCTION__: GCC extension — not standard\n");
    printf("    __PRETTY_FUNCTION__: GCC extension — includes full signature\n");

    /*
     * INTERVIEW NOTES:
     *   - __FILE__: string literal of current source file
     *   - __LINE__: integer of current line (changes each line)
     *   - __func__: string of current function name (C99)
     *   - __DATE__, __TIME__: build timestamp — useful in version strings
     *   - __STDC_VERSION__: 201112L for C11, 199901L for C99
     *   - Combine in macros: LOG(), ASSERT() — essential embedded patterns
     */
}

/* =============================================================================
 * PP10. #undef — REMOVING AND REDEFINING MACROS — HIGH
 * =========================================================================== */
void pp10_undef(void)
{
    sec("PP10 [HIGH] #undef — REMOVING AND REDEFINING MACROS");

#define TEMP_VALUE  100
    printf("  TEMP_VALUE = %d\n", TEMP_VALUE);

#undef TEMP_VALUE                /* remove the definition */
/* printf("%d", TEMP_VALUE);    // COMPILE ERROR after undef */
    printf("  After #undef TEMP_VALUE — macro no longer exists\n");

#define TEMP_VALUE  200          /* redefine with new value */
    printf("  After redefine: TEMP_VALUE = %d\n", TEMP_VALUE);
#undef TEMP_VALUE

    printf("\n  Common uses of #undef:\n");
    printf("  1. Redefine macro with different value for a section:\n");
    printf("     #undef  LOG_LEVEL\n");
    printf("     #define LOG_LEVEL 4   // temporarily verbose\n");
    printf("     ...code...\n");
    printf("     #undef  LOG_LEVEL\n");
    printf("     #define LOG_LEVEL 1   // restore\n\n");

    printf("  2. Remove a macro before redefining (avoid warning):\n");
    printf("     #undef  MAX_RETRIES\n");
    printf("     #define MAX_RETRIES 5\n\n");

    printf("  3. Temporarily override system macro:\n");
    printf("     #undef  NULL\n");
    printf("     #define NULL ((void*)0)\n\n");

    printf("  4. Scope simulation — define, use, undefine in .c file\n");

    /*
     * INTERVIEW NOTES:
     *   - #undef removes a macro — subsequent uses are undefined identifier
     *   - Redefining without #undef: compiler WARNING (redefinition)
     *   - #undef of undefined macro: silently ignored (no error)
     *   - Use for: temporary overrides, scoped macros, version-controlled defs
     *   - #undef doesn't affect const variables — only macros
     */
}

/* =============================================================================
 * PP11. do { } while(0) IDIOM — SAFE MULTI-STATEMENT MACRO (MEDIUM)
 * =========================================================================== */

/* WRONG — semicolon breaks if-else */
#define BAD_SWAP(a, b)  \
    int _t = (a); (a) = (b); (b) = _t;
/* if (cond) BAD_SWAP(x,y); else ... → dangling else problem */

/* CORRECT — do-while(0) wraps everything into single statement */
#define SWAP(a, b)  \
    do {            \
        int _t = (a); (a) = (b); (b) = _t; \
    } while (0)     /* no semicolon here — caller adds it */

#define SET_AND_CHECK(ptr, val, label)  \
    do {                                \
        if (!(ptr)) goto label;         \
        *(ptr) = (val);                 \
    } while (0)

void pp11_do_while_zero(void)
{
    sec("PP11 [MEDIUM] do { } while(0) — SAFE MULTI-STATEMENT MACRO");

    sub("Why do-while(0)?");
    printf("  Multi-statement macro without wrapper:\n");
    printf("    #define SWAP(a,b)  int t=a; a=b; b=t;\n");
    printf("    if(cond) SWAP(x,y); else ...;\n");
    printf("    Expands to: if(cond) int t=x; x=y; y=t; else ...;\n");
    printf("    PROBLEM: only 'int t=x' is guarded by if!\n\n");

    printf("  With do-while(0):\n");
    printf("    if(cond) SWAP(x,y); else ...;\n");
    printf("    Expands to: if(cond) do{...}while(0); else ...;\n");
    printf("    CORRECT: entire swap is one statement\n\n");

    int a = 3, b = 7;
    printf("  Before SWAP: a=%d b=%d\n", a, b);
    SWAP(a, b);
    printf("  After  SWAP: a=%d b=%d\n", a, b);

    if (a > b)
        SWAP(a, b);    /* safe in if without braces */
    else
        printf("  Else branch works correctly: a=%d b=%d\n", a, b);

    sub("do-while(0) with goto (error handling pattern)");
    int value = 99;
    int *ptr  = &value;
    SET_AND_CHECK(ptr, 42, cleanup);
    printf("  SET_AND_CHECK: *ptr = %d\n", *ptr);
    goto skip_cleanup;
cleanup:
    printf("  Cleanup reached\n");
skip_cleanup:;

    printf("\n  Why not if(1){...} instead?\n");
    printf("    if(1){...} after if causes 'dangling else' ambiguity\n");
    printf("    do-while(0) is the ONLY correct pattern\n");

    /*
     * INTERVIEW NOTES:
     *   - do{}while(0): makes multi-line macro act as single statement
     *   - Caller adds semicolon: SWAP(a,b); → do{...}while(0);
     *   - Without it: if(c) MACRO; else → only first line in if
     *   - Compiler optimizes away while(0) check — zero overhead
     *   - Standard pattern in Linux kernel, Zephyr RTOS macros
     *   - if(1){} alternative has dangling else problem — wrong
     */
}

/* =============================================================================
 * PP12. STRINGIFY OPERATOR # — TOKEN TO STRING (MEDIUM)
 * =========================================================================== */

#define STRINGIFY(x)       #x
#define TOSTRING(x)        STRINGIFY(x)   /* extra level to expand macros first */
#define PRINT_VAR(var)     printf("  " #var " = %d\n", (var))
#define PRINT_EXPR(expr)   printf("  " #expr " = %d\n", (expr))

void pp12_stringify(void)
{
    sec("PP12 [MEDIUM] STRINGIFY OPERATOR # — TOKEN TO STRING");

    sub("Basic stringify — converts token to string literal");
    printf("  STRINGIFY(hello)      = \"%s\"\n", STRINGIFY(hello));
    printf("  STRINGIFY(42)         = \"%s\"\n", STRINGIFY(42));
    printf("  STRINGIFY(x + y)      = \"%s\"\n", STRINGIFY(x + y));

    sub("Stringifying macros — need double expansion");
    /* STRINGIFY(MAX_BUFFER_SIZE) → "MAX_BUFFER_SIZE" (not "256") */
    printf("  STRINGIFY(MAX_BUFFER_SIZE) = \"%s\"  (NOT expanded!)\n",
           STRINGIFY(MAX_BUFFER_SIZE));
    /* TOSTRING expands the macro first, then stringifies */
    printf("  TOSTRING(MAX_BUFFER_SIZE)  = \"%s\"  (value expanded)\n",
           TOSTRING(MAX_BUFFER_SIZE));

    sub("Auto-naming variables in debug output");
    int sensor_value = 42;
    int error_code   = 0x1A;
    PRINT_VAR(sensor_value);    /* prints: sensor_value = 42 */
    PRINT_VAR(error_code);      /* prints: error_code = 26 */
    PRINT_EXPR(sensor_value * 2 + 1);  /* prints: sensor_value * 2 + 1 = 85 */

    sub("Enum name to string — stringify pattern");
#define ENUM_TO_STR(e)  case e: return #e
    /* Used in functions like: const char* state_name(State s) { switch(s) { ENUM_TO_STR(IDLE); ... } } */
    printf("  ENUM_TO_STR pattern: case IDLE: return \"IDLE\";\n");
    printf("  Very useful for debug printing of enum values\n");

    /*
     * INTERVIEW NOTES:
     *   - # operator: stringifies the token — wraps in double quotes
     *   - Stringifying a MACRO: use double expansion (TOSTRING trick)
     *     STRINGIFY(X) where X is a macro → gives macro NAME not value
     *     TOSTRING(X) expands X first, then stringifies value
     *   - # only works in function-like macros (not object-like)
     *   - Common use: debug printing, test frameworks, error messages
     */
}

/* =============================================================================
 * PP13. TOKEN PASTE OPERATOR ## — JOINING TOKENS (MEDIUM)
 * =========================================================================== */

#define JOIN(a, b)       a##b
#define MAKE_VAR(type, name)  type name##_value
#define REGISTER(n)      volatile uint32_t *REG_##n = (volatile uint32_t*)(0x40000000 + n)
#define HANDLER(name)    void name##_handler(void)
#define ARRAY_TYPED(T, N) T T##_array[N]

void pp13_token_paste(void)
{
    sec("PP13 [MEDIUM] TOKEN PASTE ## — JOINING TOKENS");

    sub("Basic token pasting");
    int xy = 42;   /* variable named 'xy' */
    printf("  JOIN(x,y) creates identifier 'xy': %d\n", JOIN(x,y));

    int var_value = 100;
    printf("  MAKE_VAR creates 'var_value': %d\n", var_value);

    sub("Generate function names");
    /* HANDLER(uart) → void uart_handler(void) */
    printf("  HANDLER(uart) → void uart_handler(void)\n");
    printf("  HANDLER(gpio) → void gpio_handler(void)\n");
    printf("  Used in embedded: generates ISR names from peripheral names\n");

    sub("Generate register names");
    /* REGISTER(0x04) → volatile uint32_t *REG_0x04 = ... */
    printf("  REGISTER(n) → volatile uint32_t *REG_n = base + n\n");

    sub("Generating multiple typed arrays");
    ARRAY_TYPED(int, 8);      /* creates: int int_array[8] */
    ARRAY_TYPED(float, 4);    /* creates: float float_array[4] */
    for (int i = 0; i < 8; i++) int_array[i]   = i;
    for (int i = 0; i < 4; i++) float_array[i] = (float)i * 1.5f;
    printf("  int_array  : %d %d %d ...\n", int_array[0], int_array[1], int_array[2]);
    printf("  float_array: %.1f %.1f %.1f %.1f\n",
           float_array[0], float_array[1], float_array[2], float_array[3]);

    sub("Double expansion needed with ##");
    /* Like #, ## does not expand macros on its own */
    /* #define CONCAT(a,b) a##b  — if a or b are macros, expand first */
    printf("  JOIN(PI,_VALUE): if PI is a macro, use double expansion\n");

    /*
     * INTERVIEW NOTES:
     *   - ## joins two tokens into one identifier
     *   - Result must be a valid token (identifier, number, etc.)
     *   - Common use: generate unique names, register names, state tables
     *   - Double expansion: if operands are macros, need extra level like ##
     *   - ## can create function names, struct names, variable names
     *   - Embedded: #define GPIO_PIN(port,pin) GPIO##port##_PIN##pin
     */
}

/* =============================================================================
 * PP14. VARIADIC MACROS — __VA_ARGS__ (MEDIUM)
 * =========================================================================== */

/* Variadic log macro — variable number of arguments */
#define LOG(level, fmt, ...)  \
    printf("[" level "] " fmt "\n", ##__VA_ARGS__)

/* Debug macro — only active when DEBUG_MODE defined */
#ifdef DEBUG_MODE
#define DBG(fmt, ...)  printf("[DBG %s:%d] " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)
#else
#define DBG(fmt, ...)  /* nothing — stripped out in release */
#endif

/* Custom assert with message */
#define MY_ASSERT(cond, fmt, ...) \
    do { \
        if (!(cond)) { \
            printf("ASSERT[%s:%d]: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
        } \
    } while (0)

void pp14_variadic_macros(void)
{
    sec("PP14 [MEDIUM] VARIADIC MACROS — __VA_ARGS__");

    sub("Basic variadic macro");
    LOG("INFO",  "System started");
    LOG("WARN",  "Retry count = %d", 3);
    LOG("ERROR", "Code=0x%02X at %s", 0x1A, "uart.c");

    sub("Debug macro — present in debug, stripped in release");
    DBG("Sensor reading: %d mV", 3300);
    DBG("Entering %s mode", "low-power");

    sub("Custom assert with format string");
    int val = 42;
    MY_ASSERT(val > 0,  "val=%d must be positive", val);
    MY_ASSERT(val < 10, "val=%d exceeds limit 10",  val);  /* triggers */

    sub("## before __VA_ARGS__ — handles zero args");
    printf("  ##__VA_ARGS__: removes leading comma when no variadic args\n");
    printf("  Without ##: LOG(\"INFO\") → printf(\"[INFO] \\n\",) — extra comma\n");
    printf("  With    ##: LOG(\"INFO\") → printf(\"[INFO] \\n\")  — comma removed\n");

    sub("__VA_ARGS__ rules");
    printf("  ... in macro params: accepts zero or more arguments\n");
    printf("  __VA_ARGS__: expands to all variadic arguments\n");
    printf("  ##__VA_ARGS__: removes preceding comma if no args given\n");
    printf("  C99+: __VA_ARGS__ supported. C23: __VA_OPT__ (cleaner)\n");

    /*
     * INTERVIEW NOTES:
     *   - ... in macro parameter list: accepts variadic args
     *   - __VA_ARGS__: expands to all extra arguments
     *   - ##__VA_ARGS__: GCC extension — removes comma when zero extra args
     *   - C23: __VA_OPT__(,) — standard way to handle optional comma
     *   - Use for: logging, assertions, debug wrappers
     *   - Common in embedded: LOG_INFO, LOG_ERROR, LOG_DEBUG macros
     */
}

/* =============================================================================
 * PP15. #error / #warning — COMPILE-TIME DIAGNOSTICS (MEDIUM)
 * =========================================================================== */
void pp15_error_warning(void)
{
    sec("PP15 [MEDIUM] #error / #warning — COMPILE-TIME DIAGNOSTICS");

    printf("  These are compile-time directives — demonstrated conceptually:\n\n");

    printf("  #error — stops compilation with message:\n");
    printf("    #ifndef REQUIRED_DEFINE\n");
    printf("    #error \"REQUIRED_DEFINE must be set before including this\"\n");
    printf("    #endif\n\n");

    printf("  #warning — emits warning but continues compilation:\n");
    printf("    #warning \"This API is deprecated, use new_func() instead\"\n\n");

    printf("  Common patterns:\n");
    printf("    Version check:\n");
    printf("      #if __STDC_VERSION__ < 199901L\n");
    printf("      #error \"C99 or later required\"\n");
    printf("      #endif\n\n");

    printf("    Platform requirement:\n");
    printf("      #ifndef __arm__\n");
    printf("      #error \"This code only runs on ARM\"\n");
    printf("      #endif\n\n");

    printf("    Size check:\n");
    printf("      #if BUFFER_SIZE < 64\n");
    printf("      #error \"BUFFER_SIZE too small — minimum 64\"\n");
    printf("      #endif\n\n");

    printf("    Deprecation warning:\n");
    printf("      #define OLD_FUNC(x)  \\\n");
    printf("          (_Pragma(\"GCC warning \\\"OLD_FUNC deprecated\\\"\") new_func(x))\n");

    /*
     * INTERVIEW NOTES:
     *   - #error: hard stop — compilation fails with message
     *   - #warning: soft warning — compilation continues
     *   - Use #error to enforce: C version, platform, required defines
     *   - Use #warning for: API deprecation, migration hints
     *   - _Pragma("GCC warning ...") — #pragma inside macro (C99)
     *   - _Static_assert: runtime-type check at compile time (C11)
     */
}

/* =============================================================================
 * PP16. #line — OVERRIDE LINE/FILE REPORTING (MEDIUM)
 * =========================================================================== */
void pp16_line_directive(void)
{
    sec("PP16 [MEDIUM] #line — OVERRIDE LINE/FILE REPORTING");

    printf("  Current: %s:%d\n", __FILE__, __LINE__);

#line 1000 "virtual_file.c"
    /* Compiler now thinks we're at line 1000 of virtual_file.c */
    printf("  After #line 1000 \"virtual_file.c\": %s:%d\n", __FILE__, __LINE__);
    printf("  Next line: %s:%d\n", __FILE__, __LINE__);

#line 1 "string_preprocessor_concepts.c"  /* restore */
    printf("  Restored: %s:%d\n", __FILE__, __LINE__);

    printf("\n  Use cases for #line:\n");
    printf("    Code generators: generated .c files report errors in original source\n");
    printf("    Yacc/Bison: parser generator inserts #line for error reporting\n");
    printf("    Template engines: errors point to template, not generated file\n");
    printf("    Obfuscators: hide true source locations\n");
    printf("    Test frameworks: report test file/line not framework internals\n");

    /*
     * INTERVIEW NOTES:
     *   - #line N: sets __LINE__ to N for next source line
     *   - #line N "file": also changes __FILE__
     *   - Used by code generators (yacc, bison, m4) for error reporting
     *   - Compiler error messages will show the #line-set values
     *   - Rarely needed in handwritten code — mainly for tools
     */
}

/* =============================================================================
 * PP17. #pragma pack — STRUCT ALIGNMENT (MEDIUM)
 * =========================================================================== */
void pp17_pragma_pack(void)
{
    sec("PP17 [MEDIUM] #pragma pack — STRUCT ALIGNMENT IN PREPROCESSOR");

    /* Normal struct with compiler padding */
    struct Normal { char a; int b; char c; };
    printf("  struct Normal {char,int,char}  sizeof=%zu  (padded)\n",
           sizeof(struct Normal));

/* Pack everything to 1-byte alignment */
#pragma pack(push, 1)
    struct Packed1 { char a; int b; char c; };
#pragma pack(pop)
    printf("  struct Packed1 (pack=1)        sizeof=%zu  (no padding)\n",
           sizeof(struct Packed1));

#pragma pack(push, 2)
    struct Packed2 { char a; int b; char c; };
#pragma pack(pop)
    printf("  struct Packed2 (pack=2)        sizeof=%zu  (2-byte align)\n",
           sizeof(struct Packed2));

    printf("\n  Pragma pack pattern:\n");
    printf("    #pragma pack(push, N)  ← save current, set to N\n");
    printf("    struct { ... };\n");
    printf("    #pragma pack(pop)      ← restore previous\n\n");

    printf("  N values and effect:\n");
    printf("    pack(1): members packed tightly — no padding\n");
    printf("    pack(2): each member aligned to 2-byte boundary\n");
    printf("    pack(4): each member aligned to 4-byte boundary (default often)\n\n");

    printf("  Warnings:\n");
    printf("    pack(1) on ARM Cortex-M0: unaligned access → bus fault!\n");
    printf("    pack(1) on x86: unaligned access → slower but works\n");
    printf("    Forgetting pop: affects all subsequent structs — always use push/pop\n");

    /*
     * INTERVIEW NOTES:
     *   - #pragma pack changes struct member alignment, not element alignment
     *   - push/pop: saves and restores previous pack value — ALWAYS use
     *   - Forgetting pop: subsequent structs unexpectedly packed
     *   - Use case: network protocol headers, file format structures
     *   - Alternative: __attribute__((packed)) per-struct (GCC/Clang)
     */
}

/* =============================================================================
 * PP18. MACRO vs static inline — WHEN TO USE WHICH (MEDIUM)
 * =========================================================================== */

/* Macro version — no type, no scope */
#define MACRO_SQ(x)     ((x) * (x))
#define MACRO_ABS(x)    ((x) < 0 ? -(x) : (x))

/* static inline versions — typed, scoped, debuggable */
static inline int    inline_sq_int(int x)       { return x * x; }
static inline float  inline_sq_flt(float x)     { return x * x; }
static inline int    inline_abs(int x)           { return x < 0 ? -x : x; }

void pp18_macro_vs_inline(void)
{
    sec("PP18 [MEDIUM] MACRO vs static inline — WHEN TO USE WHICH");

    printf("  Macro:\n");
    printf("    MACRO_SQ(3)   = %d\n",   MACRO_SQ(3));
    printf("    MACRO_SQ(3.5) = %.1f\n", MACRO_SQ(3.5));  /* works with float too */
    printf("    MACRO_ABS(-7) = %d\n",   MACRO_ABS(-7));

    printf("\n  static inline:\n");
    printf("    inline_sq_int(3)   = %d\n",   inline_sq_int(3));
    printf("    inline_sq_flt(3.5) = %.1f\n", inline_sq_flt(3.5));
    printf("    inline_abs(-7)     = %d\n",   inline_abs(-7));

    printf("\n  Comparison:\n");
    printf("  %-25s | %-20s | %-20s\n", "Property", "Macro", "static inline");
    printf("  %-25s | %-20s | %-20s\n", "-------------------------",
           "--------------------", "--------------------");
    printf("  %-25s | %-20s | %-20s\n", "Type checking",  "NONE", "Full");
    printf("  %-25s | %-20s | %-20s\n", "Scope",          "None (global)", "File scope");
    printf("  %-25s | %-20s | %-20s\n", "Side effects",   "Possible (2x eval)", "Safe (1x eval)");
    printf("  %-25s | %-20s | %-20s\n", "Debuggable",     "No (expanded)", "Yes (address)");
    printf("  %-25s | %-20s | %-20s\n", "Works any type", "YES (generic)", "No (one type)");
    printf("  %-25s | %-20s | %-20s\n", "Conditional comp","YES (#ifdef)",  "No");
    printf("  %-25s | %-20s | %-20s\n", "Performance",    "=inline(good)",  "=macro(good)");

    printf("\n  Rule of thumb:\n");
    printf("    Use MACRO for: constants, conditional compile, bit ops, generics\n");
    printf("    Use inline for: math, logic, type-sensitive — avoids side effects\n");

    /*
     * INTERVIEW NOTES:
     *   - Both get inlined by compiler — zero function call overhead
     *   - Macro advantage: works with any type (generic), conditional compile
     *   - inline advantage: type safety, single evaluation, debuggable
     *   - For math: inline if single type; macro if needs to work with float AND int
     *   - C11 _Generic: create type-safe generic macros (advanced)
     */
}

/* =============================================================================
 * PP19. #pragma GCC poison — BAN IDENTIFIER (LOW)
 * =========================================================================== */
void pp19_pragma_poison(void)
{
    sec("PP19 [LOW] #pragma GCC poison — BAN IDENTIFIER");

    printf("  Syntax: #pragma GCC poison identifier\n\n");
    printf("  Effect: any use of 'identifier' after this point = compile error\n\n");

    printf("  Common use cases:\n");
    printf("    Ban unsafe functions:\n");
    printf("      #pragma GCC poison gets strcpy sprintf atoi\n");
    printf("      strcpy(dst, src);  // error: attempt to use poisoned 'strcpy'\n\n");

    printf("    Enforce safe alternatives:\n");
    printf("      #pragma GCC poison malloc\n");
    printf("      // Use your_safe_malloc() wrapper instead\n\n");

    printf("    MISRA compliance — ban C99 VLAs:\n");
    printf("      (Cannot poison keywords, but can poison custom allocators)\n\n");

    printf("  Note: only works for IDENTIFIERS, not keywords\n");
    printf("  Note: if poisoned macro used inside another macro — also error\n");
    printf("  Note: GCC/Clang — not portable to MSVC\n");

    /*
     * INTERVIEW NOTES:
     *   - GCC extension — not in C standard
     *   - Useful for enforcing safe coding standards (no gets(), no strcpy())
     *   - Can poison: function names, variable names, macro names
     *   - Cannot poison: C keywords (if, while, int, etc.)
     *   - Error message: "error: attempt to use poisoned 'identifier'"
     */
}

/* =============================================================================
 * PP20. X-MACRO PATTERN — CODE GENERATION (LOW)
 * =========================================================================== */

/* X-macro table — single source of truth */
/* Each entry: X(enum_value, string_name, handler_function) */
#define ERROR_TABLE \
    X(ERR_NONE,    "No error",        NULL)          \
    X(ERR_TIMEOUT, "Timeout",         NULL)          \
    X(ERR_OVERFLOW,"Buffer overflow", NULL)          \
    X(ERR_INVALID, "Invalid input",   NULL)

/* Generate enum from table */
typedef enum {
#define X(val, str, fn)  val,
    ERROR_TABLE
#undef X
    ERR_COUNT
} ErrorCode;

/* Generate string array from table */
static const char *error_strings[] = {
#define X(val, str, fn)  str,
    ERROR_TABLE
#undef X
};

void pp20_x_macro(void)
{
    sec("PP20 [LOW] X-MACRO PATTERN — CODE GENERATION");

    printf("  X-macro: define data ONCE, generate multiple representations\n\n");

    printf("  ERROR_TABLE defines %d errors\n", ERR_COUNT);
    printf("  Enum values generated:\n");
    printf("    ERR_NONE=%d  ERR_TIMEOUT=%d  ERR_OVERFLOW=%d  ERR_INVALID=%d\n",
           ERR_NONE, ERR_TIMEOUT, ERR_OVERFLOW, ERR_INVALID);

    printf("\n  String array generated:\n");
    for (int i = 0; i < ERR_COUNT; i++) {
        printf("    [%d] = \"%s\"\n", i, error_strings[i]);
    }

    printf("\n  How it works:\n");
    printf("    Step 1: #define X(a,b,c) a,     → generates enum values\n");
    printf("    Step 2: #undef X               → remove definition\n");
    printf("    Step 3: #define X(a,b,c) b,     → generates string array\n");
    printf("    Step 4: #undef X               → remove definition\n");
    printf("    Single table → multiple consistent representations\n\n");

    printf("  Benefits:\n");
    printf("    Add new error: ONE change in table, all arrays/enums updated\n");
    printf("    No sync errors between enum and string array\n");
    printf("    Can also generate: switch cases, function tables, counts\n");

    /*
     * INTERVIEW NOTES:
     *   - X-macro: define list once, expand differently each time
     *   - Prevents enum/string mismatch — common source of bugs
     *   - Used in: error codes, state machines, command tables, register maps
     *   - Always #undef X after use to allow reuse of X name
     *   - Linux kernel uses this pattern extensively
     */
}

/* =============================================================================
 * PP21. __attribute__ — GCC FUNCTION/VARIABLE ATTRIBUTES (LOW)
 * =========================================================================== */

/* Function attributes */
__attribute__((noreturn))
void fatal_error(const char *msg)
{
    printf("  FATAL: %s — aborting\n", msg);
    exit(1);
    /* compiler knows this never returns — no missing-return warnings */
}

__attribute__((always_inline))
static inline int force_inlined(int x) { return x * x; }

__attribute__((noinline))
static int never_inlined(int x) { return x + 1; }

__attribute__((deprecated("use new_api() instead")))
void old_api(void) { printf("  old_api called\n"); }

/* Variable attributes */
__attribute__((aligned(16)))
static int aligned_array[4] = {1, 2, 3, 4};

/* Section placement — common in embedded */
/* __attribute__((section(".ccmram"))) int fast_var; */

void pp21_attributes(void)
{
    sec("PP21 [LOW] __attribute__ — GCC FUNCTION/VARIABLE ATTRIBUTES");

    printf("  Commonly used GCC attributes:\n\n");

    printf("  Function attributes:\n");
    printf("    __attribute__((noreturn))    : function never returns (exit, abort)\n");
    printf("    __attribute__((always_inline)): force inline even without -O\n");
    printf("    __attribute__((noinline))    : prevent inlining\n");
    printf("    __attribute__((pure))        : no side effects, can be CSE'd\n");
    printf("    __attribute__((const))       : pure + ignores globals\n");
    printf("    __attribute__((deprecated)) : warn when called\n");
    printf("    __attribute__((visibility)) : control symbol visibility\n");
    printf("    __attribute__((format(...))): check printf/scanf format strings\n\n");

    printf("  Variable/type attributes:\n");
    printf("    __attribute__((aligned(N))) : align to N bytes\n");
    printf("    __attribute__((packed))     : remove padding (see S3)\n");
    printf("    __attribute__((section(\"...\"))): place in named section\n");
    printf("    __attribute__((unused))     : suppress unused warning\n\n");

    printf("  Demonstrations:\n");
    int r1 = force_inlined(5);
    int r2 = never_inlined(5);
    printf("    force_inlined(5) = %d  (always inlined)\n", r1);
    printf("    never_inlined(5) = %d  (never inlined)\n", r2);

    printf("    aligned_array at %p (aligned to 16 bytes: %s)\n",
           (void*)aligned_array,
           ((uintptr_t)aligned_array % 16 == 0) ? "YES" : "NO");

    printf("    old_api(): ");
    old_api();   /* compiler emits deprecation warning */

    printf("\n  Embedded-specific attributes:\n");
    printf("    __attribute__((section(\".ccmram\"))): place in Core Coupled Memory\n");
    printf("    __attribute__((section(\".flash\"))) : place function in flash\n");
    printf("    __attribute__((constructor))       : run before main()\n");
    printf("    __attribute__((destructor))        : run after main()\n");
    printf("    __attribute__((interrupt))         : ISR function attribute\n");

    /*
     * INTERVIEW NOTES:
     *   - GCC/Clang attributes — not in C standard (use #ifdef __GNUC__ guard)
     *   - noreturn: allows compiler to skip code after call, warn on missing return
     *   - always_inline: forces inline even at -O0 (useful for ISR helpers)
     *   - section: critical in embedded for placing code/data in specific memory
     *   - CMSIS uses __attribute__((weak)) for overrideable default handlers
     *   - Portable: use macros like #define INLINE __attribute__((always_inline))
     */
}

/* =============================================================================
 * PP22. _Static_assert — COMPILE-TIME ASSERTION (LOW)
 * =========================================================================== */
void pp22_static_assert(void)
{
    sec("PP22 [LOW] _Static_assert — COMPILE-TIME ASSERTION (C11)");

    /* _Static_assert(constant_expression, "error message") */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes on this platform");
    _Static_assert(sizeof(char) == 1, "char must be 1 byte");
    _Static_assert(sizeof(void*) >= 4, "pointer must be at least 4 bytes");

    printf("  _Static_assert(sizeof(int)==4): PASSED\n");
    printf("  _Static_assert(sizeof(char)==1): PASSED\n");

    /* Practical uses */
    _Static_assert(MAX_BUFFER_SIZE >= 64,
                   "MAX_BUFFER_SIZE too small — minimum 64 required");
    printf("  _Static_assert(MAX_BUFFER_SIZE>=64): PASSED (%d >= 64)\n",
           MAX_BUFFER_SIZE);

    /* Struct size assertion — ensure no unexpected padding */
#pragma pack(push, 1)
    struct Protocol { uint8_t cmd; uint16_t len; uint32_t data; };
#pragma pack(pop)
    _Static_assert(sizeof(struct Protocol) == 7,
                   "Protocol struct must be exactly 7 bytes");
    printf("  _Static_assert(sizeof(Protocol)==7): PASSED\n");

    printf("\n  _Static_assert vs assert():\n");
    printf("    assert()        : RUNTIME check — program aborts if fails\n");
    printf("    _Static_assert  : COMPILE-TIME — stops compilation if fails\n");
    printf("    _Static_assert  : zero runtime overhead — no code generated\n");
    printf("    _Static_assert  : only for constant expressions\n\n");

    printf("  Preprocessor alternative (pre-C11):\n");
    printf("    typedef char _assert[(condition)?1:-1]; // negative size = error\n");
    printf("    #define STATIC_ASSERT(c) typedef char _sa[(c)?1:-1]\n\n");

    printf("  C11: _Static_assert(expr, msg)\n");
    printf("  C23: static_assert(expr, msg)  ← no underscore\n");

    /*
     * INTERVIEW NOTES:
     *   - C11 feature: _Static_assert(condition, "message")
     *   - Condition must be constant expression (no variables)
     *   - Fails at COMPILE TIME — no runtime cost
     *   - Use to enforce: sizeof requirements, config value ranges, ABI contracts
     *   - C23 added static_assert as keyword (no underscore prefix)
     *   - assert() in <assert.h>: runtime, disabled by -DNDEBUG
     */
}

/* =============================================================================
 * PP23. #pragma GCC optimize — PER-FUNCTION OPTIMIZATION (LOW)
 * =========================================================================== */

/* Override optimization level for this function only */
#pragma GCC optimize("O3")
void speed_critical(int *arr, int n)
{
    /* This function compiled with -O3 regardless of global -O flag */
    for (int i = 0; i < n; i++) arr[i] *= 2;
}
#pragma GCC optimize("O0")   /* restore to O0 for rest of file */

/* More specific optimizations */
#pragma GCC optimize("unroll-loops")
void loop_unrolled(int *arr, int n)
{
    for (int i = 0; i < n; i++) arr[i] += 1;
}

void pp23_pragma_optimize(void)
{
    sec("PP23 [LOW] #pragma GCC optimize — PER-FUNCTION OPTIMIZATION");

    int arr[] = {1, 2, 3, 4, 5};
    speed_critical(arr, 5);
    printf("  speed_critical (O3) result: ");
    for (int i = 0; i < 5; i++) printf("%d ", arr[i]);
    printf("\n");

    loop_unrolled(arr, 5);
    printf("  loop_unrolled result: ");
    for (int i = 0; i < 5; i++) printf("%d ", arr[i]);
    printf("\n");

    printf("\n  Per-function optimization pragmas:\n");
    printf("    #pragma GCC optimize(\"O0\") : disable optimization (debug)\n");
    printf("    #pragma GCC optimize(\"O2\") : standard optimization\n");
    printf("    #pragma GCC optimize(\"O3\") : aggressive optimization\n");
    printf("    #pragma GCC optimize(\"Os\") : optimize for size\n");
    printf("    #pragma GCC optimize(\"unroll-loops\") : unroll all loops\n\n");

    printf("  Use cases:\n");
    printf("    ISR: #pragma GCC optimize(\"O3\") for speed-critical interrupt handler\n");
    printf("    Debug function: #pragma GCC optimize(\"O0\") to keep debug-friendly\n");
    printf("    Memory: #pragma GCC optimize(\"Os\") for rarely-called large functions\n\n");

    printf("  Alternative: __attribute__((optimize(\"O3\"))) per function\n");

    /*
     * INTERVIEW NOTES:
     *   - #pragma GCC optimize: changes optimization for subsequent functions
     *   - Use push_options/pop_options for clean scoping:
     *     #pragma GCC push_options
     *     #pragma GCC optimize("O3")
     *     ... fast functions ...
     *     #pragma GCC pop_options
     *   - Alternative: __attribute__((optimize("O3"))) void func() {}
     *   - Embedded: ISRs often need O3; debug helpers need O0
     *   - GCC-specific — use #ifdef __GNUC__ guard for portability
     */
}

/* ============================================================================
 * MAIN
 * ========================================================================== */
int main(void)
{
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║   STRINGS & PREPROCESSOR — COMPLETE CONCEPT REFERENCE   ║\n");
    printf("║   Aravinth K — Interview Preparation                    ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");

    /* ── STRINGS ─────────────────────────────────────────────────── */
    printf("\n\n▶▶▶  STRINGS  ◀◀◀\n");

    /* HIGH */
    st1_char_star_vs_char_array();
    st2_string_init_patterns();
    st3_strlen_vs_sizeof();
    st4_strcpy_strncpy();
    st5_strcat_strncat();
    st6_strcmp_strncmp();
    st7_string_input();
    st8_string_traversal();
    st9_null_terminator();
    st10_string_pointer_arithmetic();

    /* MEDIUM */
    st11_strchr_strrchr();
    st12_strstr();
    st13_strtok();
    st14_sprintf_snprintf();
    st15_string_to_number();
    st16_number_to_string();
    st17_string_reversal();
    st18_string_as_parameter();

    /* LOW */
    st19_mem_functions();
    st20_comparison_pitfall();
    st21_multiline_strings();
    st22_wide_strings();

    /* ── PREPROCESSOR ────────────────────────────────────────────── */
    printf("\n\n▶▶▶  PREPROCESSOR DIRECTIVES  ◀◀◀\n");

    /* HIGH */
    pp1_object_like_macro();
    pp2_function_like_macro();
    pp3_include();
    pp4_ifdef_ifndef();
    pp5_if_elif_else();
    pp6_pragma_once_vs_include_guard();
    pp7_macro_side_effects();
    pp8_missing_parentheses();
    pp9_predefined_macros();
    pp10_undef();

    /* MEDIUM */
    pp11_do_while_zero();
    pp12_stringify();
    pp13_token_paste();
    pp14_variadic_macros();
    pp15_error_warning();
    pp16_line_directive();
    pp17_pragma_pack();
    pp18_macro_vs_inline();

    /* LOW */
    pp19_pragma_poison();
    pp20_x_macro();
    pp21_attributes();
    pp22_static_assert();
    pp23_pragma_optimize();

    printf("\n╔══════════════════════════════════════════════════════════╗\n");
    printf("║  ALL DEMOS COMPLETE — 45 topics covered                  ║\n");
    printf("║  Strings: 22 topics (10 high, 8 medium, 4 low)           ║\n");
    printf("║  Preprocessor: 23 topics (10 high, 8 medium, 5 low)      ║\n");
    printf("║  Compile: gcc -O0 -Wall -Wextra -std=c11 -o sp \\         ║\n");
    printf("║           string_preprocessor_concepts.c                  ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    return 0;
}
