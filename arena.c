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

static size_t bytes_left_in(Arena *arena) {
  assert(arena->bytes_total > arena->current_byte_n);
  return arena->bytes_total - arena->current_byte_n;
}

static void x2_arena_memory(Arena *arena) {
  const size_t new_bytes_total = 2 * arena->bytes_total;
  arena->memory_start = realloc(arena->memory_start, new_bytes_total);
  assert(arena->memory_start != NULL);
  arena->bytes_total = new_bytes_total;
}

void *arena_push(Arena *arena, size_t size) {
  while (bytes_left_in(arena) < size) {
    x2_arena_memory(arena);
  }
  assert(arena->bytes_total > arena->current_byte_n);
  void *result = arena->memory_start + arena->current_byte_n;
  arena->current_byte_n += size;
  return result;
}

void arena_pop(Arena *arena, size_t size) {
  assert(arena->current_byte_n >= size);
  arena->current_byte_n -= size;
}

void arena_clean(Arena *arena) { arena->current_byte_n = 0; }
