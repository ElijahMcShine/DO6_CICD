#include "s21_shared_func.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int s21_check_file(char *file_name, const char *prog_name, int hide_errors) {
  int res = 1;
  struct stat buffer;
  if (stat(file_name, &buffer) == 0) {
    if (S_ISREG(buffer.st_mode) == 0) {
      if (!hide_errors)
        fprintf(stderr, "%s: %s: Is a directory\n", prog_name, file_name);
      res = 0;
    }
  } else {
    if (!hide_errors)
      fprintf(stderr, "%s: %s: No such file or directory\n", prog_name,
              file_name);
    res = 0;
  }
  return res;
}

void s21_free_before_exit(char **files_list, int files_num,
                          struct key_vals *key_values, int values_num) {
  if (files_list) {
    for (int i = 0; i < files_num; i++) {
      if (files_list[i]) {
        free(files_list[i]);
      }
    }
    free(files_list);
  }
  if (key_values) {
    for (int i = 0; i < values_num; i++) {
      if (key_values[i].value) {
        free(key_values[i].value);
      }
    }
    free(key_values);
  }
}