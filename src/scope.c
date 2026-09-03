#include "scope.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "diag.h"

// Enters a new scope inside the current one.
void scope_enter(Scope *sc)
{
    sc->depth++;
}

// Exits the inner most scope.
void scope_exit(Scope *sc)
{
    assert(sc->depth > 0 && "Attempted to exit top level scope");
    while (sc->count > 0 &&
           sc->items[sc->count - 1]->depth == sc->depth)
        sc->count--;
    sc->depth--;
}

// Releases the symbol table of `sc` and leaves it in the same state as a
// zero-initialised scope. The symbols themselves are owned by the arena they
// were allocated from and are not freed here.
void scope_free(Scope *sc)
{
    free(sc->items);
    sc->items = NULL;
    sc->count = 0;
    sc->capacity = 0;
    sc->depth = 0;
}

// Adds the symbol `sym` to the scope `sc`. The depth of `sym` gets set to the
// current depth of `sc`.
void scope_add_sym(Scope *sc, Symbol *sym)
{
    sym->depth = sc->depth;
    for (size_t i = sc->count; i-- > 0;) {
        if (sc->items[i]->depth < sc->depth) break;
        if (sc->items[i]->ns == sym->ns &&
            strcmp(sc->items[i]->name, sym->name) == 0) {
            // TODO: check redefinition of symbol with different type once we
            // have real type comparison
            if (sc->items[i]->kind != sym->kind)
                diag_fatal_at(sym->loc,
                              "redefinition of symbol `%s` as a different kind of symbol",
                              sym->name);
            else
                diag_fatal_at(sym->loc, "redefinition of symbol `%s`",
                              sym->name);
        }
    }
    da_append(sc, sym, "could not append symbol to current scope");
}

// Looks up a symbol `name` with namespace `ns`. Returns NULL if no such symbol
// exists.
static Symbol *scope_lookup_sym(const Scope *sc, Namespace ns, const char *name, size_t len)
{
    for (size_t i = sc->count; i-- > 0;) {
        Symbol *sym = sc->items[i];
        if (sym->ns == ns &&
            strlen(sym->name) == len &&
            strncmp(sym->name, name, len) == 0)
            return sym;
    }
    return NULL;
}

// Looks up a variable symbol `name` of length `len` or returns NULL if no such
// symbol exists.
Symbol *scope_lookup_var_n(const Scope *sc, const char *name, size_t len)
{
    return scope_lookup_sym(sc, NS_VAR, name, len);
}

// Looks up a tag symbol `name` of length `len` or returns NULL if no such
// symbol exists.
Symbol *scope_lookup_tag_n(const Scope *sc, const char *name, size_t len)
{
    return scope_lookup_sym(sc, NS_TAG, name, len);
}

// Looks up a variable symbol `name` or returns NULL if no such symbol exists.
Symbol *scope_lookup_var(const Scope *sc, const char *name)
{
    return scope_lookup_var_n(sc, name, strlen(name));
}

// Looks up a tag symbol `name` or returns NULL if no such symbol exists.
Symbol *scope_lookup_tag(const Scope *sc, const char *name)
{
    return scope_lookup_tag_n(sc, name, strlen(name));
}
