#include "arena.h"
#include <stddef.h>

typedef struct {
  struct ArenaList {
    struct ArenaList *previous;
    Arena *current;
  } *chain;
  size_t default_size;
} ChainArena;

ChainArena *chain_arena_init(size_t default_size);
void chain_arena_release(ChainArena *chain_arena);
void *chain_arena_push(ChainArena *chain_arena, size_t size);
