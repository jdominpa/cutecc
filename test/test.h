#ifndef TEST_H_
#define TEST_H_

#include <setjmp.h>
#include <stdio.h>

#include "../src/arena.h"

typedef struct {
    int passed;
    int failed;
    const char *current_test;
    jmp_buf abort_test;
    Arena test_arena;
} TestCtx;

static TestCtx g_test_ctx = {
    .passed = 0,
    .failed = 0,
    .current_test = NULL,
    .test_arena = {
        .current = NULL,
        .next_chunk_size = 0,
    },
};

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
        if (g_test_ctx.test_arena.current != NULL)       \
            arena_reset(&g_test_ctx.test_arena);         \
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

// Runs `...` in a forked child and checks that it terminated by calling exit()
// with `status`. Needed for code paths that end the process instead of
// returning (e.g. diag_fatal_at()) which cannot be exercised in-process. The
// child's stderr is discarded so the expected diagnostic does not clutter the
// test output.
//
// POSIX only. Windows has no fork(); these checks would need CreateProcess and
// a way to re-enter the binary at a single test, so they are simply skipped
// there rather than faked.
#ifndef _WIN32
#include <unistd.h>

#include <sys/wait.h>

#define EXPECT_EXIT(status, ...)                                              \
    do {                                                                      \
        fflush(stdout);                                                       \
        fflush(stderr);                                                       \
        pid_t test_pid = fork();                                              \
        if (test_pid < 0) {                                                   \
            TEST_FAIL("could not fork");                                      \
        } else if (test_pid == 0) {                                           \
            FILE *test_unused = freopen("/dev/null", "w", stderr);            \
            (void) test_unused;                                               \
            __VA_ARGS__;                                                      \
            /* Reached only if the body returned instead of exiting. */       \
            _exit(0);                                                         \
        } else {                                                              \
            int test_wstatus = 0;                                             \
            if (waitpid(test_pid, &test_wstatus, 0) < 0)                      \
                TEST_FAIL("could not wait for child process");                \
            else if (WIFSIGNALED(test_wstatus))                               \
                TEST_FAIL(                                                    \
                    "expected exit(%d) but child process died on signal %d",  \
                    (status), WTERMSIG(test_wstatus));                        \
            else if (!WIFEXITED(test_wstatus))                                \
                TEST_FAIL("expected exit(%d) but child process did not exit", \
                          (status));                                          \
            else if (WEXITSTATUS(test_wstatus) != (status))                   \
                TEST_FAIL(                                                    \
                    "expected exit(%d) but child process exited with %d",     \
                    (status), WEXITSTATUS(test_wstatus));                     \
        }                                                                     \
    } while (0)
#endif  // _WIN32

#define TEST_SUMMARY()                                                 \
    do {                                                               \
        if (g_test_ctx.test_arena.current != NULL)                     \
            arena_free(&g_test_ctx.test_arena);                        \
        fprintf(stderr, "\n%d passed, %d failed\n", g_test_ctx.passed, \
                g_test_ctx.failed);                                    \
        if (g_test_ctx.failed > 0)                                     \
            return 1;                                                  \
    } while (0)

#endif  // TEST_H_
