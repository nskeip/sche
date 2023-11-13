#include "arena.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

Arena ArenaAlloc(size_t bytes_total) {
  char *memory = calloc(bytes_total, sizeof(char));
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
  assert(arena->ptr >= arena->memory_start);
  if ((size_t)(arena->ptr - arena->memory_start) >= size) {
    arena->ptr -= size;
  }
}

void ArenaClean(Arena *arena) {
  arena->ptr = arena->memory_start;
  memset(arena->memory_start, 0, arena->bytes_total);
}
