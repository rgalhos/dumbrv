#ifndef MEMORY_H
#define MEMORY_H

#include "log.h"
#include <stdint.h>
#include <stdlib.h>
#ifdef __linux__
#include <sys/mman.h>
#endif // __linux__

int alloc_hugepage(void **memptr, size_t, size_t);

#endif // MEMORY_H