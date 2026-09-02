#include "../src/scope.h"

#include <stdlib.h>
#include <string.h>

#include "../src/ast_print.h"
#include "test.h"

DEFINE_TEST(test_lookup_unknown_name)
{
    Scope sc = { 0 };
    EXPECT(scope_lookup_var(&sc, "a") == NULL,
           "expected variable `a` to be unknown");
    EXPECT(scope_lookup_tag(&sc, "a") == NULL,
           "expected tag `a` to be unknown");
    free(sc.items);
}

DEFINE_TEST(test_lookup_existing_symbol)
{
    Scope sc = { 0 };
    const char *name = "a";

    Symbol *sym = arena_alloc(&g_test_ctx.test_arena, Symbol);
    sym->ns = NS_VAR;
    sym->name = name;
    scope_add_sym(&sc, sym);
    Symbol *found = scope_lookup_var(&sc, name);
    EXPECT(found && strcmp(found->name, "a") == 0,
           "expected to find variable `%s` in scope", name);

    sym = arena_alloc(&g_test_ctx.test_arena, Symbol);
    sym->ns = NS_TAG;
    sym->name = name;
    scope_add_sym(&sc, sym);
    found = scope_lookup_tag(&sc, name);
    EXPECT(found && strcmp(found->name, "a") == 0,
           "expected to find tag `%s` in scope", name);
    free(sc.items);
}

DEFINE_TEST(test_shadowing_symbol)
{
    Scope sc = { 0 };
    const char *name = "a";

    Symbol *sym = arena_alloc(&g_test_ctx.test_arena, Symbol);
    sym->ns = NS_VAR;
    sym->name = name;
    sym->ty = (Type) { .kind = TYPE_INT };
    scope_add_sym(&sc, sym);
    scope_enter(&sc);

    sym = arena_alloc(&g_test_ctx.test_arena, Symbol);
    sym->ns = NS_VAR;
    sym->name = name;
    sym->ty = (Type) { .kind = TYPE_CHAR };
    scope_add_sym(&sc, sym);
    Symbol *found = scope_lookup_var(&sc, name);
    EXPECT(found && strcmp(found->name, name) == 0 && found->ty.kind == TYPE_CHAR,
           "expected to find variable `%s` of type `%s` but got `%s` with type `%s` instead",
           name, type_to_str((Type) { .kind = TYPE_CHAR }),
           found->name, type_to_str(found->ty));

    scope_exit(&sc);
    found = scope_lookup_var(&sc, name);
    EXPECT(found && strcmp(found->name, name) == 0 && found->ty.kind == TYPE_INT,
           "expected to find variable `%s` of type `%s` but got `%s` with type `%s` instead",
           name, type_to_str((Type) { .kind = TYPE_INT }),
           found->name, type_to_str(found->ty));
}

DEFINE_TEST(test_unbind_symbol_on_scope_exit)
{
    Scope sc = { 0 };
    const char *name = "a";
    Symbol *sym = arena_alloc(&g_test_ctx.test_arena, Symbol);
    sym->ns = NS_VAR;
    sym->name = name;
    scope_enter(&sc);
    scope_add_sym(&sc, sym);
    scope_exit(&sc);
    Symbol *found = scope_lookup_var(&sc, name);
    EXPECT(!found,
           "expected symbol `%s` to be unbound after exiting inner scope",
           name);
}

int main(void)
{
    g_test_ctx.test_arena = arena_init();
    RUN_TEST(test_lookup_unknown_name);
    RUN_TEST(test_lookup_existing_symbol);
    RUN_TEST(test_shadowing_symbol);
    RUN_TEST(test_unbind_symbol_on_scope_exit);
    TEST_SUMMARY();
}
