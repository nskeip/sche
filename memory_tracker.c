#include "memory_tracker.h"
#include <stddef.h>
#include <stdlib.h>

MemoryTracker *memory_tracker_init(size_t capacity) {
  void **memory_for_pointers = malloc(capacity * sizeof(void **));
  if (memory_for_pointers == NULL) {
    return NULL;
  }

  MemoryTracker *mt = malloc(sizeof(MemoryTracker));
  if (mt == NULL) {
    free(memory_for_pointers);
    return NULL;
  }

  mt->capacity = capacity;
  mt->len = 0;
  mt->pointers = memory_for_pointers;

  return mt;
}

void memory_tracker_release(MemoryTracker *mt) {
  for (size_t i = 0; i < mt->len; ++i) {
    free(mt->pointers[i]);
    mt->pointers[i] = NULL;
  }
  free(mt->pointers);
  free(mt);
  mt = NULL;
}

void *memory_tracker_push(MemoryTracker *mt, size_t size) {
  if (mt->capacity == mt->len) {
    const size_t new_capacity = mt->capacity * 2;
    if (new_capacity < mt->capacity) {
      return NULL;
    }
    void *new_pointers_container =
        realloc(mt->pointers, new_capacity * sizeof(void **));
    if (new_pointers_container == NULL) {
      return NULL;
    }
    mt->pointers = new_pointers_container;
    mt->capacity = new_capacity;
  }
  void *new_pointer = malloc(size);
  if (new_pointer == NULL) {
    return NULL;
  }
  mt->pointers[mt->len++] = new_pointer;
  return new_pointer;
}
