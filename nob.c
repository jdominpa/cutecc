#include <string.h>

#define NOB_IMPLEMENTATION
#define NOB_WARN_DEPRECATED
#include "nob.h"

#define CFLAGS "-Wall", "-Wextra", "-ggdb"

#define BUILD_DIR "./build/"
#define SRC_DIR "./src/"
#define TEST_DIR "./test/"
#define TEST_DATA_DIR "./test/expected/"

// Appends every header in SRC_DIR to `headers`. Nothing here tracks which
// translation unit includes which header, so a change to any of them rebuilds
// everything. We should switch to `cc -MMD` dep files if the full rebuild ever
// becomes noticeable.
static bool collect_src_headers(Nob_File_Paths *headers)
{
    bool result = true;
    Nob_File_Paths entries = { 0 };

    if (!nob_read_entire_dir(SRC_DIR, &entries)) nob_return_defer(false);
    for (size_t i = 0; i < entries.count; ++i) {
        const char *entry = entries.items[i];
        size_t len = strlen(entry);
        if (len > 2 && strcmp(entry + len - 2, ".h") == 0)
            nob_da_append(headers, nob_temp_sprintf(SRC_DIR"%s", entry));
    }

defer:
    nob_da_free(entries);
    return result;
}

static bool build_static_libsimpcc(Cmd *cmd)
{
    bool result = true;
    const char *lib_files[] = {
        "arena",
        "ast_print",
        "diag",
        "io",
        "lexer",
        "parser",
        "scope",
    };
    const char *obj_files[NOB_ARRAY_LEN(lib_files)];
    Nob_File_Paths headers = { 0 };
    Nob_File_Paths inputs = { 0 };
    int rebuild;

    if (!collect_src_headers(&headers)) nob_return_defer(false);

    // Build object files
    for (size_t i = 0; i < NOB_ARRAY_LEN(lib_files); ++i) {
        const char *input_lib_file = nob_temp_sprintf(SRC_DIR"%s.c", lib_files[i]);
        obj_files[i] = nob_temp_sprintf(BUILD_DIR"%s.o", lib_files[i]);

        inputs.count = 0;
        nob_da_append(&inputs, input_lib_file);
        nob_da_append_many(&inputs, headers.items, headers.count);

        rebuild = nob_needs_rebuild(obj_files[i], inputs.items, inputs.count);
        if (rebuild < 0) nob_return_defer(false);
        if (rebuild > 0) {
            nob_cmd_append(cmd, "cc", "-c", CFLAGS);
            nob_cmd_append(cmd, "-o", obj_files[i]);
            nob_cmd_append(cmd, input_lib_file);
            if (!nob_cmd_run(cmd)) nob_return_defer(false);
        }
    }

    // Build static archive libsimpcc.a
    rebuild = nob_needs_rebuild(BUILD_DIR"libsimpcc.a", obj_files,
                                NOB_ARRAY_LEN(lib_files));
    if (rebuild < 0) nob_return_defer(false);
    if (rebuild > 0) {
        if (nob_file_exists(BUILD_DIR "libsimpcc.a") &&
            !nob_delete_file(BUILD_DIR "libsimpcc.a"))
            nob_return_defer(false);
        nob_cmd_append(cmd, "ar", "rcs", BUILD_DIR"libsimpcc.a");
        nob_da_append_many(cmd, obj_files, NOB_ARRAY_LEN(lib_files));
        if (!nob_cmd_run(cmd)) nob_return_defer(false);
    }

defer:
    nob_da_free(headers);
    nob_da_free(inputs);
    return result;
}

static const char *test_files[] = {
    "lexer",
    "parser",
    "scope",
};

static bool build_tests(Cmd *cmd)
{
    bool result = true;
    Nob_File_Paths headers = { 0 };
    Nob_File_Paths inputs = { 0 };

    if (!nob_mkdir_if_not_exists(TEST_DATA_DIR)) nob_return_defer(false);
    if (!collect_src_headers(&headers)) nob_return_defer(false);

    for (size_t i = 0; i < NOB_ARRAY_LEN(test_files); ++i) {
        const char *test_bin = nob_temp_sprintf(BUILD_DIR"test_%s", test_files[i]);
        const char *input_test_file = nob_temp_sprintf(TEST_DIR"%s.c", test_files[i]);

        inputs.count = 0;
        nob_da_append(&inputs, input_test_file);
        nob_da_append(&inputs, TEST_DIR"test.h");
        nob_da_append_many(&inputs, headers.items, headers.count);
        nob_da_append(&inputs, BUILD_DIR"libsimpcc.a");

        int rebuild = nob_needs_rebuild(test_bin, inputs.items, inputs.count);
        if (rebuild < 0) nob_return_defer(false);
        if (rebuild > 0) {
            nob_cmd_append(cmd, "cc", CFLAGS, "-o", test_bin);
            nob_cmd_append(cmd, input_test_file);
            nob_cmd_append(cmd, BUILD_DIR"libsimpcc.a");
            nob_cmd_append(cmd, nob_temp_sprintf("-DTEST_DATA_DIR=\"%s\"", TEST_DATA_DIR));
            if (!nob_cmd_run(cmd)) nob_return_defer(false);
        }
    }

defer:
    nob_da_free(headers);
    nob_da_free(inputs);
    return result;
}

static bool build_simpcc(Cmd *cmd)
{
    bool result = true;
    Nob_File_Paths headers = { 0 };
    Nob_File_Paths inputs = { 0 };

    if (!collect_src_headers(&headers)) nob_return_defer(false);
    nob_da_append(&inputs, SRC_DIR"simpcc.c");
    nob_da_append_many(&inputs, headers.items, headers.count);
    nob_da_append(&inputs, BUILD_DIR"libsimpcc.a");

    int rebuild = nob_needs_rebuild(BUILD_DIR"simpcc", inputs.items, inputs.count);
    if (rebuild < 0) nob_return_defer(false);
    if (rebuild > 0) {
        nob_cmd_append(cmd, "cc", CFLAGS, "-o", BUILD_DIR"simpcc");
        nob_cmd_append(cmd, SRC_DIR"simpcc.c");
        nob_cmd_append(cmd, BUILD_DIR"libsimpcc.a");
        if (!nob_cmd_run(cmd)) nob_return_defer(false);
    }

defer:
    nob_da_free(headers);
    nob_da_free(inputs);
    return result;
}

static bool update_test_data_files(void)
{
    bool result = true;
    Nob_File_Paths files = { 0 };
    if (!nob_read_entire_dir(TEST_DATA_DIR, &files)) nob_return_defer(false);
    for (size_t i = 0; i < files.count; ++i) {
        Nob_String_View name = nob_sv_from_cstr(files.items[i]);
        if (!nob_sv_chop_suffix(&name, nob_sv_from_cstr(".actual"))) continue;
        const char *src = nob_temp_sprintf(TEST_DATA_DIR SV_Fmt ".actual", SV_Arg(name));
        const char *dst = nob_temp_sprintf(TEST_DATA_DIR SV_Fmt, SV_Arg(name));
        nob_log(NOB_INFO, "updating %s", dst);
        if (!nob_rename(src, dst)) nob_return_defer(false);
    }

defer:
    nob_da_free(files);
    return result;
}

static void usage(const char *program)
{
    nob_log(NOB_INFO, "Usage: %s [<subcommand>]", program);
    nob_log(NOB_INFO, "Subcommands:");
    nob_log(NOB_INFO, "    test");
    nob_log(NOB_INFO, "        Build and run compiler tests.");
    nob_log(NOB_INFO, "    update_tests");
    nob_log(NOB_INFO, "        Update the expected test output files in '%s'.", TEST_DATA_DIR);
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
    } else if (strcmp(arg, "update_tests") == 0) {
        if (!update_test_data_files()) return 1;
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
