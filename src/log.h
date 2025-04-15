#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

int custom_log(const char *, const char *, const int, const char *, ...);

#define FP_BYTE_TO_BIN_PATTERN "%c%c%c%c%c%c%c%c"
#define FP_BYTE_TO_BIN(byte)                                                   \
  ((byte) & 0x80 ? '1' : '0'), ((byte) & 0x40 ? '1' : '0'),                    \
      ((byte) & 0x20 ? '1' : '0'), ((byte) & 0x10 ? '1' : '0'),                \
      ((byte) & 0x08 ? '1' : '0'), ((byte) & 0x04 ? '1' : '0'),                \
      ((byte) & 0x02 ? '1' : '0'), ((byte) & 0x01 ? '1' : '0')

#define FP_WORD_TO_BIN_PATTERN                                                 \
  FP_BYTE_TO_BIN_PATTERN " " FP_BYTE_TO_BIN_PATTERN " " FP_BYTE_TO_BIN_PATTERN \
                         " " FP_BYTE_TO_BIN_PATTERN

#define FP_WORD_TO_BIN(word)                                                   \
  FP_BYTE_TO_BIN(word >> 24), FP_BYTE_TO_BIN((word >> 16) & 0xFF),             \
      FP_BYTE_TO_BIN((word >> 8) & 0xFF), FP_BYTE_TO_BIN(word & 0xFF)

#define LOG_TRACE(format, ...)                                                 \
  custom_log("\033[0;37mTRACE\033[0;0m", __FILE__, __LINE__, format "\n", ##__VA_ARGS__)
#define LOG_DEBUG(format, ...)                                                 \
  custom_log("\033[0;36mDEBUG\033[0;0m", __FILE__, __LINE__, format "\n", ##__VA_ARGS__)
#define LOG_INFO(format, ...)                                                  \
  custom_log("\033[0;32mINFO\033[0;0m", __FILE__, __LINE__, format "\n", ##__VA_ARGS__)
#define LOG_WARN(format, ...)                                                  \
  custom_log("\033[0;33mWARN\033[0;0m", __FILE__, __LINE__, format "\n", ##__VA_ARGS__)
#define LOG_ERROR(format, ...)                                                 \
  custom_log("\033[0;31mERROR\033[0;0m", __FILE__, __LINE__, format "\n", ##__VA_ARGS__)
#undef LOG

#endif // LOG_H
