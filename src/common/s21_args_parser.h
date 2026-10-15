#ifndef S21_ARGS_PARSER_H
#define S21_ARGS_PARSER_H

struct simple_key_arg {
  char name;
  int position;
  int has_val;  // 0 - without values; 1 - necessary value; 2 - not necessary
                // value; 3 - necessary argument
};

struct key_arg {
  char *name;
  int position;
  int has_val;
};

struct key_vals {
  int position;
  char *value;
};

int s21_find_help(int argc, char **argv);

int s21_find_args_files(int argc, char **argv,
                        struct simple_key_arg *short_key_array,
                        int short_key_len, struct key_arg *key_array,
                        int key_len, struct key_vals **values_array,
                        int *values_num, int *flags_array, int flags_num,
                        char ***files_list, int *files_num,
                        const char *prog_name);

int s21_arg_parsing(char *arg, struct key_arg *key_array, int key_len,
                    int *flags_array, struct key_vals *key_value,
                    int *nec_arg_array, const char *prog_name);
// 0 - error (not found); 1 - necessary value; 2 - not necessary value; 3 -
// value is found

int s21_simple_arg_parsing(const char *arg,
                           struct simple_key_arg *short_key_array,
                           int short_key_len, int *flags_array,
                           struct key_vals *key_value, int *nec_arg_array,
                           const char *prog_name);
// 0 - error (not found); 1 - necessary value; 2 - not necessary value; 3 -
// value is found

int s21_short_key_check_val(const char *arg, int arg_pos,
                            struct simple_key_arg key,
                            struct key_vals *key_value);
int s21_key_check_val(const char *arg, struct key_arg key,
                      struct key_vals *key_value);
int s21_copy_arg_value(struct key_vals **values_array, int *values_num,
                       int value_pos, const char *value);
int s21_key_parse(int argc, char **argv, int *i,
                  struct simple_key_arg *short_key_array, int short_key_len,
                  struct key_arg *key_array, int key_len,
                  struct key_vals **values_array, int *values_num,
                  int *flags_array, int *nec_arg_array, const char *prog_name);
void s21_find_nec_args(int argc, char **argv,
                       struct simple_key_arg *short_key_array,
                       int short_key_len, struct key_arg *key_array,
                       int key_len, int *nec_arg_array);
void s21_find_nec_shrt_keys(const char *str,
                            struct simple_key_arg *short_key_array,
                            int short_key_len, int *nec_arg_array);
void s21_find_nec_keys(char *str, struct key_arg *key_array, int key_len,
                       int *nec_arg_array);
int s21_prepare_nec_array(int **nec_array, int *flags_array, int flags_num,
                          struct simple_key_arg *short_key_array,
                          int short_key_len, struct key_arg *key_array,
                          int key_len);
int s21_check_nec_flags(const int *nec_arg_array, int nec_arr_len);
int s21_add_file_to_list(char ***files_list, int *files_num, const char *argv);

#endif