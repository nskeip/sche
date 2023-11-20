#include "arena.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

Arena arena_alloc(size_t bytes_total) {
  char *memory = malloc(bytes_total);
  Arena a = {
      .memory_start = memory, .current_byte_n = 0, .bytes_total = bytes_total};
  return a;
}

void arena_release(Arena *arena) { free(arena->memory_start); }

void *arena_push(Arena *arena, size_t size) {
  assert(arena->bytes_total >= arena->current_byte_n);
  const size_t bytes_left_in = arena->bytes_total - arena->current_byte_n;
  while (bytes_left_in < size) {
    return NULL;
  }
  void *peek = arena->memory_start + arena->current_byte_n;
  arena->current_byte_n += size;
  return peek;
}
