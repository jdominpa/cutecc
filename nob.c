#include <string.h>

#define NOB_IMPLEMENTATION
#define NOB_WARN_DEPRECATED
#include "nob.h"

#define CFLAGS "-Wall", "-Wextra", "-ggdb"

#define BUILD_DIR "./build/"
#define SRC_DIR "./src/"
#define TEST_DIR "./test/"

static bool build_static_libsimpcc(Cmd *cmd)
{
    const char *lib_files[] = {
        "arena",
        "ast_print",
        "diag",
        "lexer",
        "parser",
    };
    const size_t files_count = NOB_ARRAY_LEN(lib_files);

    // Build object files
    for (size_t i = 0; i < files_count; ++i) {
        const char *input_lib_file = nob_temp_sprintf(SRC_DIR"%s.c", lib_files[i]);
        const char *output_obj_file = nob_temp_sprintf(BUILD_DIR"%s.o", lib_files[i]);
        if (nob_needs_rebuild1(output_obj_file, input_lib_file)) {
            nob_cmd_append(cmd, "cc", "-c", CFLAGS);
            nob_cmd_append(cmd, "-o", output_obj_file);
            nob_cmd_append(cmd, input_lib_file);
            if (!nob_cmd_run(cmd)) return false;
        }
    }

    // Build static archive libsimpcc.a
    const char *obj_files[files_count];
    for (size_t i = 0; i < files_count; ++i)
        obj_files[i] = nob_temp_sprintf(BUILD_DIR"%s.o", lib_files[i]);
    if (nob_needs_rebuild(BUILD_DIR"libsimpcc.a", obj_files, files_count)) {
        if (!nob_delete_file(BUILD_DIR"libsimpcc.a")) return false;
        nob_cmd_append(cmd, "ar", "rcs");
        nob_cmd_append(cmd, BUILD_DIR"libsimpcc.a");
        for (size_t i = 0; i < files_count; ++i)
            nob_cmd_append(cmd, nob_temp_sprintf(BUILD_DIR"%s.o", lib_files[i]));
        if (!nob_cmd_run(cmd)) return false;
    }

    return true;
}

static const char *test_files[] = {
    "lexer",
};

static bool build_tests(Cmd *cmd)
{
    for (size_t i = 0; i < NOB_ARRAY_LEN(test_files); ++i) {
        const char *test_bin = nob_temp_sprintf(BUILD_DIR"test_%s", test_files[i]);
        const char *input_test_file = nob_temp_sprintf(TEST_DIR"%s.c", test_files[i]);
        if (nob_needs_rebuild(test_bin,
                              (const char *[]) {
                                  input_test_file,
                                  BUILD_DIR "libsimpcc.a",
                              },
                              2)) {
            nob_cmd_append(cmd, "cc", CFLAGS, "-o", test_bin);
            nob_cmd_append(cmd, input_test_file);
            nob_cmd_append(cmd, BUILD_DIR"libsimpcc.a");
            if (!nob_cmd_run(cmd)) return false;
        }
    }
    return true;
}

static bool build_simpcc(Cmd *cmd)
{
    if (nob_needs_rebuild(BUILD_DIR "simpcc",
                          (const char *[]) {
                              SRC_DIR "simpcc.c",
                              BUILD_DIR "libsimpcc.a",
                          },
                          2)) {
        nob_cmd_append(cmd, "cc", CFLAGS, "-o", BUILD_DIR"simpcc");
        nob_cmd_append(cmd, SRC_DIR"simpcc.c");
        nob_cmd_append(cmd, BUILD_DIR"libsimpcc.a");
        if (!nob_cmd_run(cmd)) return false;
    }
    return true;
}

static void usage(const char *program)
{
    nob_log(NOB_INFO, "Usage: %s [<subcommand>]", program);
    nob_log(NOB_INFO, "Subcommands:");
    nob_log(NOB_INFO, "    test");
    nob_log(NOB_INFO, "        Build and run compiler tests.");
    nob_log(NOB_INFO, "    build");
    nob_log(NOB_INFO, "        Build the compiler.");
    nob_log(NOB_INFO, "    run [<args>]");
    nob_log(NOB_INFO, "        Build and run the compiler.");
    nob_log(NOB_INFO, "        If <args> is provided the compiler is run with them.");
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);
    Nob_Cmd cmd = { 0 };

    if (!nob_mkdir_if_not_exists(BUILD_DIR)) return 1;
    const char *program = nob_shift_args(&argc, &argv);
    if (argc == 0) {
        nob_log(NOB_ERROR, "No subcommand provided");
        usage(program);
        return 1;
    }
    const char *arg = nob_shift_args(&argc, &argv);
    if (strcmp(arg, "test") == 0) {
        if (!build_static_libsimpcc(&cmd)) return 1;
        if (!build_tests(&cmd)) return 1;
        bool err = false;
        for (size_t i = 0; i < NOB_ARRAY_LEN(test_files); ++i) {
            nob_cmd_append(&cmd, nob_temp_sprintf(BUILD_DIR"test_%s", test_files[i]));
            if (!nob_cmd_run(&cmd)) err = true;
        }
        if (err) return 1;
    } else if (strcmp(arg, "build") == 0) {
        if (!build_static_libsimpcc(&cmd)) return 1;
        if (!build_simpcc(&cmd)) return 1;
    } else if (strcmp(arg, "run") == 0) {
        if (!build_static_libsimpcc(&cmd)) return 1;
        if (!build_simpcc(&cmd)) return 1;
        nob_cmd_append(&cmd, BUILD_DIR"simpcc");
        if (argc > 0) nob_da_append_many(&cmd, argv, argc);
        if (!nob_cmd_run(&cmd)) return 1;
    } else {
        nob_log(NOB_ERROR, "Unknown subcommand `%s`", arg);
        usage(program);
        return 1;
    }

    return 0;
}
