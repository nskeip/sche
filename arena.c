#include "arena.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

Arena *arena_init(size_t bytes_total) {
  Arena *a = malloc(sizeof(Arena));
  a->memory_start = malloc(bytes_total);
  a->current_byte_n = 0;
  a->bytes_total = bytes_total;
  return a;
}

void arena_release(Arena *arena) {
  if (arena == NULL) {
    return;
  }
  if (arena->memory_start != NULL) {
    free(arena->memory_start);
  }
  free(arena);
}

void *arena_push(Arena *arena, size_t size) {
  assert(arena->bytes_total >= arena->current_byte_n);
  const size_t bytes_left_in = arena->bytes_total - arena->current_byte_n;
  if (bytes_left_in < size) {
    return NULL;
  }
  void *peek = arena->memory_start + arena->current_byte_n;
  arena->current_byte_n += size;
  return peek;
}
