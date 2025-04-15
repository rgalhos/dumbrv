#include "memory.h"
#include <stdint.h>

int alloc_hugepage(void **memptr, size_t alignment, size_t memsize) {
  LOG_TRACE("alloc_hugepage(%p, %zu, %zu)", memptr, alignment, memsize);

#ifdef __WIN32__
  *memptr = malloc(memsize);
  // lol
  return memptr == NULL;
#else  // __WIN32__

  int result = posix_memalign(memptr, alignment, memsize);
  if (result) {
    LOG_ERROR("Could not allocate memory: %d", result);
    return result;
  }

  result = madvise(*memptr, memsize, MADV_HUGEPAGE);
  if (result) {
    LOG_WARN("madvise returned error: %d", result);
    return result;
  }
#endif // __WIN32__

  return 0;
}
