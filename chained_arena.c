#include "chained_arena.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

static struct ArenaList *arena_list_init(size_t size) {
  struct ArenaList *arena_list = malloc(sizeof(struct ArenaList));
  arena_list->previous = NULL;
  arena_list->current = arena_init(size);
  return arena_list;
}

ChainedArena *chained_arena_init(size_t default_size) {
  ChainedArena *chain_arena = malloc(sizeof(ChainedArena));
  chain_arena->default_size = default_size;
  chain_arena->chain = arena_list_init(default_size);
  return chain_arena;
}

static void arena_list_release(struct ArenaList *arena_list) {
  if (arena_list == NULL) {
    return;
  }
  arena_release(arena_list->current);
  arena_list_release(arena_list->previous);
  free(arena_list);
}

void chained_arena_release(ChainedArena *chain_arena) {
  if (chain_arena == NULL) {
    return;
  }
  arena_list_release(chain_arena->chain);
  free(chain_arena);
}

void *chained_arena_push(ChainedArena *chain_arena, size_t size) {
  void *push_result = arena_push(chain_arena->chain->current, size);
  if (push_result != NULL) {
    return push_result;
  }

  size_t new_arena_size = chain_arena->default_size;
  while (new_arena_size < size) {
    assert(new_arena_size < new_arena_size + chain_arena->default_size);
    new_arena_size += chain_arena->default_size;
  }

  struct ArenaList *new_chain = arena_list_init(new_arena_size);
  new_chain->previous = chain_arena->chain;
  chain_arena->chain = new_chain;

  return arena_push(chain_arena->chain->current, size);
}
