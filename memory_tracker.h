#ifndef _MEMORY_TRACKER_H
#define _MEMORY_TRACKER_H
#include <stddef.h>
#include <stdlib.h>

typedef struct {
  size_t capacity;
  size_t len;
  void **pointers;
} MemoryTracker;

MemoryTracker *memory_tracker_init(size_t pointers_n);
void memory_tracker_release(MemoryTracker *mt);
void *memory_tracker_push(MemoryTracker *mt, size_t size);
#endif // !_MEMORY_TRACKER_H
