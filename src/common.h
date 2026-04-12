#ifndef COMMON_H_
#define COMMON_H_

#include <stdlib.h>

#define UNUSED(x) (void) (x)
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

#define MAX(a, b) (a) > (b) ? (a) : (b)
#define MIN(a, b) (a) > (b) ? (b) : (a)

#endif  // COMMON_H_
