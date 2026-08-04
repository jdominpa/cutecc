#ifndef TEST_H_
#define TEST_H_

#include <stdio.h>

typedef struct {
    int passed;
    int failed;
    const char *current_test;
} TestCtx;

static TestCtx g_test_ctx = { 0 };

#define DEFINE_TEST(name) static void name(void)

#define RUN_TEST(name)                                   \
    do {                                                 \
        g_test_ctx.current_test = #name;                 \
        int prev_failed = g_test_ctx.failed;             \
        name();                                          \
        fprintf(stderr, "%s:", g_test_ctx.current_test); \
        if (g_test_ctx.failed == prev_failed) {          \
            g_test_ctx.passed++;                         \
            fprintf(stderr, " OK\n");                    \
        } else                                           \
            fprintf(stderr, " FAILED\n");                \
    } while (0)

#define ASSERT(cond, ...)                                           \
    do {                                                            \
        if (!(cond)) {                                              \
            fprintf(stderr, "    FAIL %s:%d:", __FILE__, __LINE__); \
            fprintf(stderr, __VA_ARGS__);                           \
            fprintf(stderr, "\n");                                  \
            g_test_ctx.failed++;                                    \
        }                                                           \
    } while (0)

#define TEST_SUMMARY()                                                 \
    do {                                                               \
        fprintf(stderr, "\n%d passed, %d failed\n", g_test_ctx.passed, \
                g_test_ctx.failed);                                    \
        if (g_test_ctx.failed > 0)                                     \
            return 1;                                                  \
    } while (0)

#endif  // TEST_H_
