#ifndef S21_SHARED_FUNC_H
#define S21_SHARED_FUNC_H

#include "s21_args_parser.h"

int s21_check_file(char *file_name, const char *prog_name, int hide_errors);
void s21_free_before_exit(char **files_list, int files_num,
                          struct key_vals *key_values, int values_num);

#endif