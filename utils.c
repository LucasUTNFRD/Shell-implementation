#include "utils.h"
#include "shell.h"

char *split_line(char *line, char split_cond) {
  size_t i = 0;
  while (line[i] != split_cond && line[i] != END_STRING) {
    i++;
  }
  line[i++] = END_STRING;

  while (line[i] == SPACE) {
    i++;
  }

  return &line[i];
}
