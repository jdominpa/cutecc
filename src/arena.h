#ifndef ARENA_H_
#define ARENA_H_

#include <stdint.h>
#include <stdlib.h>

typedef struct Chunk Chunk;
struct Chunk {
    Chunk *prev;
    unsigned char *start;
    size_t size;
    size_t offset;
};

typedef struct {
    Chunk *current;
    size_t next_chunk_size;
} Arena;

Arena arena_init(void);
void *arena_alloc_aligned(Arena *a, size_t size, size_t align);
void arena_reset(Arena *a);
void arena_free(Arena *a);

#define arena_alloc(a, T) \
    ((T *) arena_alloc_aligned((a), sizeof(T), _Alignof(T)))
#define arena_alloc_many(a, T, count)                                       \
    ((count) <= SIZE_MAX / sizeof(T)                                        \
         ? (T *) arena_alloc_aligned((a), sizeof(T) * (count), _Alignof(T)) \
         : (abort(), (T *) NULL))

typedef struct {
    size_t chunk_count;
    size_t total_size;   /* sum of all chunk sizes */
    size_t total_offset; /* sum of all chunk offsets */
} ArenaStats;

ArenaStats arena_stats(const Arena *a);

#endif
