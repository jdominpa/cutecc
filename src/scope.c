#include "scope.h"

#include <assert.h>
#include <stdio.h>
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
static Symbol *scope_lookup_sym(Scope *sc, Namespace ns, const char *name)
{
    for (size_t i = sc->count; i-- > 0;)
        if (sc->items[i]->ns == ns &&
            strcmp(sc->items[i]->name, name) == 0) return sc->items[i];
    return NULL;
}

// Looks up a variable symbol `name` or returns NULL if no such symbol exists.
Symbol *scope_lookup_var(Scope *sc, const char *name)
{
    return scope_lookup_sym(sc, NS_VAR, name);
}

// Looks up a tag symbol `name` or returns NULL if no such symbol exists.
Symbol *scope_lookup_tag(Scope *sc, const char *name)
{
    return scope_lookup_sym(sc, NS_TAG, name);
}
