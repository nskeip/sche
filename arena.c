#include "arena.h"
#include <stdlib.h>

Arena ArenaAlloc(size_t bytes_total) {
  char *memory = malloc(bytes_total);
  Arena a = {.memory_start = memory, .ptr = memory, .bytes_total = bytes_total};
  return a;
}

void ArenaRelease(Arena *arena) { free(arena->memory_start); }

static size_t bytes_left_in(Arena *arena) {
  return arena->bytes_total - (arena->ptr - arena->memory_start);
}

void *ArenaPush(Arena *arena, size_t size) {
  if (bytes_left_in(arena) < size) {
    return NULL;
  }
  void *result = arena->ptr;
  arena->ptr += size;
  return result;
}

void ArenaPop(Arena *arena, size_t size) {
  if (arena->ptr - arena->memory_start > size) {
    arena->ptr -= size;
  }
}
