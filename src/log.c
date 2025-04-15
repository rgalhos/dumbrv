#include "log.h"

int custom_log(const char *level, const char *filename, const int line,
               const char *format, ...) {
  va_list vargs;

  int count = printf("[%s] [%s:%d] ", level,
                     strchr(filename, '/') + 1 /* ./src/ */, line);

  va_start(vargs, format);
  count += vprintf(format, vargs);
  va_end(vargs);

  return count;
}