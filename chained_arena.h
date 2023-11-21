#include "arena.h"
#include <stddef.h>

typedef struct {
  struct ArenaList {
    struct ArenaList *previous;
    Arena *current;
  } *chain;
  size_t default_size;
} ChainedArena;

ChainedArena *chained_arena_init(size_t default_size);
void chained_arena_release(ChainedArena *chain_arena);
void *chained_arena_push(ChainedArena *chain_arena, size_t size);
