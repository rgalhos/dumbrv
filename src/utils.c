#include "utils.h"
#include "log.h"

size_t parse_memsize(char *s) {
  char rsize[5] = {0};
  size_t rsize_count = 0;
  size_t n = 0;

  char c;
  while ((c = *s++)) {
    if (c >= '0' && c <= '9') {
      if (rsize_count >= 4) {
        return 0;
      }

      rsize[rsize_count++] = c;
      break;
    }
  }

  char unit = *(++s);
  bool ibi = *(++s) == 'i';
  n = atoi(rsize);

  if (ibi) {
    switch (unit) {
    case 'B':
      return n;
    case 'K':
      return n << 10;
    case 'M':
      return n << 20;
    case 'G':
      return n << 30;
    }
  } else {
    switch (unit) {
    case 'B':
      return n;
    case 'K':
      return n * 1000;
    case 'M':
      return n * 1e6;
    case 'G':
      return n * 1e9;
    }
  }

  return 0;
}

int hexdump(const char *buffer, size_t offset, size_t length) {
  int pipe_in[2], pipe_out[2];
  if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1) {
    LOG_ERROR("pipe failed");
    exit(EXIT_FAILURE);
  }

  pid_t pid = fork();
  if (pid == -1)
    return -1;

  if (pid == 0) {
    close(pipe_in[1]);
    close(pipe_out[0]);

    dup2(pipe_in[0], STDIN_FILENO);
    dup2(pipe_out[1], STDOUT_FILENO);

    return execlp("xxd", "xxd", "-R", "always", NULL);
  } else {
    close(pipe_in[0]);
    close(pipe_out[1]);

    write(pipe_in[1], buffer + offset, length);
    close(pipe_in[1]); // EOF

    char output[128];
    ssize_t count;
    while ((count = read(pipe_out[0], output, sizeof(output) - 1)) > 0) {
      output[count] = '\0';
      printf("%s", output);
    }

    close(pipe_out[0]);

    wait(NULL);

    return 0;
  }
}
