#include "s21_args_parser.h"

#include <stdio.h>
#include <stdlib.h>

#include "s21_string.h"

#define CON_KEY_CHAR '-'

int s21_find_help(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    if (s21_strcmp(argv[i], "--help") == 0) return 1;
  }
  return 0;
}

int s21_find_args_files(int argc, char **argv,
                        struct simple_key_arg *short_key_array,
                        int short_key_len, struct key_arg *key_array,
                        int key_len, struct key_vals **values_array,
                        int *values_num, int *flags_array, int flags_num,
                        char ***files_list, int *files_num,
                        const char *prog_name) {
  // if (short_key_array == NULL) return 0;
  // if (key_array == NULL) return 0;
  if (flags_array == NULL) return 0;
  int res = 1;
  free(*values_array);
  *values_num = 0;
  free(*files_list);
  *files_num = 0;

  int *nec_arg_array;
  res =
      s21_prepare_nec_array(&nec_arg_array, flags_array, flags_num,
                            short_key_array, short_key_len, key_array, key_len);

  if (res)
    s21_find_nec_args(argc, argv, short_key_array, short_key_len, key_array,
                      key_len, nec_arg_array);

  for (int i = 1; res && i < argc; i++) {
    int len = s21_strlen(argv[i]);
    if ((argv[i][0] == CON_KEY_CHAR) && (len != 1)) {
      if (!s21_key_parse(argc, argv, &i, short_key_array, short_key_len,
                         key_array, key_len, values_array, values_num,
                         flags_array, nec_arg_array, prog_name))
        res = 0;
    } else {
      int pos = s21_check_nec_flags(nec_arg_array, flags_num);
      if (pos != -1) {
        nec_arg_array[pos] = 0;
        if (!s21_copy_arg_value(values_array, values_num, pos, argv[i]))
          res = 0;
      } else {
        res = s21_add_file_to_list(files_list, files_num, argv[i]);
      }
    }
  }

  if (s21_check_nec_flags(nec_arg_array, flags_num) != -1) {
    res = 0;
    fprintf(stderr, "%s: one of the options requires a value\n", prog_name);
  }

  free(nec_arg_array);
  return res;
}

int s21_key_parse(int argc, char **argv, int *i,
                  struct simple_key_arg *short_key_array, int short_key_len,
                  struct key_arg *key_array, int key_len,
                  struct key_vals **values_array, int *values_num,
                  int *flags_array, int *nec_arg_array, const char *prog_name) {
  int res = 1;
  struct key_vals temp_val;
  temp_val.value = NULL;
  int var = 0, temp = 0;
  if (argv[*i][1] != CON_KEY_CHAR) {
    temp = s21_simple_arg_parsing(argv[*i], short_key_array, short_key_len,
                                  flags_array, &temp_val, nec_arg_array,
                                  prog_name);
  }
  if (argv[*i][1] == CON_KEY_CHAR) {
    temp = s21_arg_parsing(argv[*i], key_array, key_len, flags_array, &temp_val,
                           nec_arg_array, prog_name);
    var = 1;
  }
  if (temp == 0)
    res = 0;
  else if (temp != 1) {
    if (temp == 2) {
      if (*i + 1 < argc) {
        *i = *i + 1;
        if (!s21_copy_arg_value(values_array, values_num, temp_val.position,
                                argv[*i]))
          res = 0;
      } else {
        res = 0;
        if (var)
          fprintf(stderr, "%s: option '%s' requires an argument\n", prog_name,
                  argv[*i]);
        else
          fprintf(stderr, "%s: option requires an argument -- '%c'\n",
                  prog_name, argv[*i][s21_strlen(argv[*i]) - 1]);
      }
    }
    if ((temp == 3) && (*i + 1 < argc) && (argv[*i + 1][0] != CON_KEY_CHAR)) {
      *i = *i + 1;
      if (!s21_copy_arg_value(values_array, values_num, temp_val.position,
                              argv[*i]))
        res = 0;
    }
    if (temp == 4) {
      if (!s21_copy_arg_value(values_array, values_num, temp_val.position,
                              temp_val.value))
        res = 0;
    }
  }
  free(temp_val.value);
  return res;
}

int s21_copy_arg_value(struct key_vals **values_array, int *values_num,
                       int value_pos, const char *value) {
  int res = 1;
  *values_num = *values_num + 1;
  *values_array =
      realloc(*values_array, sizeof(struct key_vals) * (*values_num));

  if (*values_array) {
    (*values_array)[*values_num - 1].position = value_pos;
    (*values_array)[*values_num - 1].value =
        (char *)malloc(sizeof(char) * (s21_strlen(value) + 1));
    if ((*values_array)[*values_num - 1].value)
      s21_strcpy((*values_array)[*values_num - 1].value, value);
    else {
      res = 0;
      for (int i = 0; i < *values_num; i++) {
        free((*values_array)[i].value);
      }
      *values_num = 0;
    }
  } else
    res = 0;

  return res;
}

int s21_arg_parsing(char *arg, struct key_arg *key_array, int key_len,
                    int *flags_array, struct key_vals *key_value,
                    int *nec_arg_array, const char *prog_name) {
  // 0 - error (not found); 1 - ok; 2 - necessary value; 3 - not necessary
  // value; 4  - value is found
  int res = 1, eq = 0;

  for (int i = 0; i < key_len && (res == 1); i++) {
    const char *substr_found = s21_strstr(arg, key_array[i].name);
    // char *substr_found = strstr(key_array[i].name, arg);
    if (substr_found) {
      (*key_value).position = key_array[i].position;
      flags_array[key_array[i].position] = 1;
      nec_arg_array[key_array[i].position] = 0;

      res = s21_key_check_val(arg, key_array[i], key_value);
      if (res != 0) eq = 1;
    }
  }

  if (!eq) {
    res = 0;
    fprintf(stderr, "%s: invalid option -- '%s'\n", prog_name, arg);
    fprintf(stderr, "Try '%s --help' for more information.\n", prog_name);
  }

  return res;
}

int s21_key_check_val(const char *arg, struct key_arg key,
                      struct key_vals *key_value) {
  int res = 1;
  if (s21_strcmp(arg, key.name) == 0) {
    if ((key.has_val == 1) || (key.has_val == 3)) res = 2;
    if (key.has_val == 2) res = 3;
  } else {
    if (((key.has_val == 1) || (key.has_val == 2) || (key.has_val == 3)) &&
        (arg[s21_strlen(key.name)] == '=')) {
      int val_len = s21_strlen(arg) - (s21_strlen(key.name));
      (*key_value).value = (char *)malloc(sizeof(char) * (val_len));
      if (!(*key_value).value) {
        free((*key_value).value);
        res = 0;
      } else {
        for (int m = 0; m < val_len; m++) {
          (*key_value).value[m] = arg[s21_strlen(key.name) + 1 + m];
        }
        res = 4;
      }
    }
  }
  return res;
}

int s21_simple_arg_parsing(const char *arg,
                           struct simple_key_arg *short_key_array,
                           int short_key_len, int *flags_array,
                           struct key_vals *key_value, int *nec_arg_array,
                           const char *prog_name) {
  // 0 - error (not found); 1 - ok; 2 - necessary value; 3 - not necessary
  // value; 4 - value is found
  int res = 1;
  for (int j = 1; j < s21_strlen(arg) && (res == 1); j++) {
    int eq = 0;
    for (int k = 0; k < short_key_len; k++) {
      if (arg[j] == short_key_array[k].name) {
        flags_array[short_key_array[k].position] = 1;
        nec_arg_array[short_key_array[k].position] = 0;
        if ((short_key_array[k].has_val == 1) ||
            (short_key_array[k].has_val == 2) ||
            (short_key_array[k].has_val == 3)) {
          res = s21_short_key_check_val(arg, j, short_key_array[k], key_value);
        }
        eq = 1;
      }
    }
    if (!eq) {
      res = 0;
      fprintf(stderr, "%s: invalid option -- '%c'\n", prog_name, arg[j]);
      fprintf(stderr, "Try '%s --help' for more information.\n", prog_name);
    }
  }

  return res;
}

int s21_short_key_check_val(const char *arg, int arg_pos,
                            struct simple_key_arg key,
                            struct key_vals *key_value) {
  int res = 1;
  (*key_value).position = key.position;
  if (arg_pos == s21_strlen(arg) - 1) {
    if ((key.has_val == 1) || (key.has_val == 3)) res = 2;
    if (key.has_val == 2) res = 3;
  } else {
    (*key_value).value =
        (char *)malloc(sizeof(char) * (s21_strlen(arg) - arg_pos + 1));
    if (!(*key_value).value) {
      res = 0;
    } else {
      for (int n = 0; n < (s21_strlen(arg) - arg_pos); n++) {
        (*key_value).value[n] = arg[n + arg_pos + 1];
      }
      res = 4;
    }
  }
  return res;
}

void s21_find_nec_shrt_keys(const char *str,
                            struct simple_key_arg *short_key_array,
                            int short_key_len, int *nec_arg_array) {
  for (int j = 1; j < s21_strlen(str); j++) {
    for (int k = 0; k < short_key_len; k++) {
      if (str[j] == short_key_array[k].name) {
        nec_arg_array[short_key_array[k].position] = 0;
      }
    }
  }
}

void s21_find_nec_keys(char *str, struct key_arg *key_array, int key_len,
                       int *nec_arg_array) {
  for (int i = 0; i < key_len; i++) {
    const char *substr_found = s21_strstr(str, key_array[i].name);
    if (substr_found) {
      if ((s21_strcmp(str, key_array[i].name) == 0) ||
          ((s21_strcmp(str, key_array[i].name) != 0) &&
           (str[s21_strlen(key_array[i].name)] == '=')))
        nec_arg_array[key_array[i].position] = 0;
    }
  }
}

void s21_find_nec_args(int argc, char **argv,
                       struct simple_key_arg *short_key_array,
                       int short_key_len, struct key_arg *key_array,
                       int key_len, int *nec_arg_array) {
  for (int i = 1; i < argc; i++) {
    if ((argv[i][0] == CON_KEY_CHAR) && (s21_strlen(argv[i]) > 1)) {
      if (argv[i][1] != CON_KEY_CHAR)
        s21_find_nec_shrt_keys(argv[i], short_key_array, short_key_len,
                               nec_arg_array);
      else
        s21_find_nec_keys(argv[i], key_array, key_len, nec_arg_array);
    }
  }
}

int s21_prepare_nec_array(int **nec_array, int *flags_array, int flags_num,
                          struct simple_key_arg *short_key_array,
                          int short_key_len, struct key_arg *key_array,
                          int key_len) {
  int res = 1;
  *nec_array = (int *)malloc(sizeof(int) * flags_num);
  if (!*nec_array) {
    free(*nec_array);
    res = 0;
  } else {
    for (int i = 0; i < flags_num; i++) {
      (*nec_array)[i] = 0;
    }
    if (short_key_len != 0) {
      for (int i = 0; i < short_key_len; i++) {
        if (short_key_array[i].has_val == 3) {
          (*nec_array)[short_key_array[i].position] = 1;
          flags_array[short_key_array[i].position] = 1;
        }
      }
    }
    if (key_len != 0) {
      for (int i = 0; i < key_len; i++) {
        if (key_array[i].has_val == 3) {
          (*nec_array)[key_array[i].position] = 1;
          flags_array[key_array[i].position] = 1;
        }
      }
    }
  }

  return res;
}

int s21_check_nec_flags(const int *nec_arg_array, int nec_arr_len) {
  int res = -1;
  for (int i = 0; i < nec_arr_len; i++) {
    if (nec_arg_array[i]) {
      res = i;
    }
  }
  return res;
}

int s21_add_file_to_list(char ***files_list, int *files_num, const char *argv) {
  int res = 1;
  *files_num = *files_num + 1;
  *files_list = realloc(*files_list, sizeof(char *) * (*files_num));
  if (!(*files_list))
    res = 0;
  else {
    (*files_list)[*files_num - 1] =
        malloc(sizeof(char) * (s21_strlen(argv) + 1));
    if (!(*files_list)[*files_num - 1]) {
      for (int m = 0; m < *files_num; m++) {
        free((*files_list)[m]);
      }
      free(*files_list);
      res = 0;
    } else {
      s21_strcpy((*files_list)[*files_num - 1], argv);
    }
  }
  return res;
}