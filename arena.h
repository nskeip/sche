#include <stddef.h>

typedef struct {
  char *memory_start;
  size_t current_byte_n;
  size_t bytes_total;
} Arena;

// based on
// Untangling Lifetimes: The Arena Allocator
// by Ryan Fleury
// https://www.rfleury.com/p/untangling-lifetimes-the-arena-allocator

Arena arena_alloc(size_t size);
void arena_release(Arena *arena);
void *arena_peek(Arena *arena);
void *arena_push_dyn(Arena *arena, size_t size);
void arena_pop(Arena *arena, size_t size);
void arena_clean(Arena *arena);
