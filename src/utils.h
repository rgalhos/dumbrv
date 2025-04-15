#ifndef UTILS_H
#define UTILS_H

#include "log.h"
#include <stdbool.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

struct program_params {
  size_t memory;
  const char *image;
};

size_t parse_memsize(char *);
int hexdump(const char *, size_t, size_t);

#endif // UTILS_H