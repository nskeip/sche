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

Arena ArenaAlloc(size_t size);
void ArenaRelease(Arena *arena);
void *ArenaPush(Arena *arena, size_t size);
void ArenaPop(Arena *arena, size_t size);
void ArenaClean(Arena *arena);

#define PushArray(arena, type, count)                                          \
  (type *)ArenaPush((arena), sizeof(type) * (count))

#define PushStruct(arena, type) PushArray((arena), (type), 1)

#define PopArray(arena, type, count) ArenaPop((arena), sizeof(type) * (count))

#define PopStruct(arena, type) PopArray((arena), (type), 1)
