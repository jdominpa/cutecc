#ifndef TEST_H_
#define TEST_H_

#include <setjmp.h>
#include <stdio.h>

typedef struct {
    int passed;
    int failed;
    const char *current_test;
    jmp_buf abort_test;
} TestCtx;

static TestCtx g_test_ctx = { 0 };

#define DEFINE_TEST(name) static void name(void)

#define RUN_TEST(name)                                   \
    do {                                                 \
        g_test_ctx.current_test = #name;                 \
        int prev_failed = g_test_ctx.failed;             \
        if (setjmp(g_test_ctx.abort_test) == 0)          \
            name();                                      \
        fprintf(stderr, "%s:", g_test_ctx.current_test); \
        if (g_test_ctx.failed == prev_failed) {          \
            g_test_ctx.passed++;                         \
            fprintf(stderr, " OK\n");                    \
        } else                                           \
            fprintf(stderr, " FAILED\n");                \
    } while (0)

#define TEST_FAIL(...)                                            \
    do {                                                          \
        fprintf(stderr, "    %s:%d: FAIL: ", __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__);                             \
        fprintf(stderr, "\n");                                    \
        g_test_ctx.failed++;                                      \
    } while (0)

#define EXPECT(cond, ...)           \
    do {                            \
        if (!(cond))                \
            TEST_FAIL(__VA_ARGS__); \
    } while (0)

#define ASSERT(cond, ...)                      \
    do {                                       \
        if (!(cond)) {                         \
            TEST_FAIL(__VA_ARGS__);            \
            longjmp(g_test_ctx.abort_test, 1); \
        }                                      \
    } while (0)

#define TEST_SUMMARY()                                                 \
    do {                                                               \
        fprintf(stderr, "\n%d passed, %d failed\n", g_test_ctx.passed, \
                g_test_ctx.failed);                                    \
        if (g_test_ctx.failed > 0)                                     \
            return 1;                                                  \
    } while (0)

#endif  // TEST_H_
