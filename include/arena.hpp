#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stddef.h>
#include <stdint.h>
#include <utility>

#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT (2 * sizeof(void *))
#endif // !DEFAULT_ALIGNMENT

// ----Arena----//
struct Arena {
  unsigned char *buffer;
  size_t buffer_size;
  size_t offset;
  size_t prev_offset;
};

void *arena_alloc(Arena *arena, size_t size, size_t align = DEFAULT_ALIGNMENT);
void *arena_resize(Arena *arena, void *buffer, size_t curr_size,
                   size_t new_size, size_t align = DEFAULT_ALIGNMENT);

void arena_init(Arena *arena, void *buffer, size_t buffer_size);

void arena_reset(Arena *arena);
void arena_free(Arena *arena);

/*
 * utility functions
 * */
bool arena_is_power_of_two(uintptr_t);
uintptr_t arena_align_forward(uintptr_t ptr, size_t align);

template <typename T, typename... Args>
T *arena_create(Arena *arena, Args &&...args) {
  void *mem = arena_alloc(arena, sizeof(T), alignof(T));

  if (!mem)
    return nullptr;

  return new (mem) T(std::forward<Args>(args)...);
}
// ----TempArena ----//

struct ArenaTemp {

  Arena *arena;
  size_t offset;
  size_t prev_offset;
};

ArenaTemp arena_savepoint_create(Arena *);
void arena_savepoint_restore(ArenaTemp);
