#include <stdlib.h>

#define UNUSED(value) (void) (value)
#define UNREACHABLE(message)                                            \
    do {                                                                \
        fprintf(stderr, "%s:%d: UNREACHABLE: %s\n", __FILE__, __LINE__, \
                message);                                               \
        abort();                                                        \
    } while (0)
#define TODO(message)                                                      \
    do {                                                                   \
        fprintf(stderr, "%s:%d: TODO: %s\n", __FILE__, __LINE__, message); \
        abort();                                                           \
    } while (0)
