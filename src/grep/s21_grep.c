#include <regex.h>
#include <stdio.h>
#include <stdlib.h>

#include "../common/s21_args_parser.h"
#include "../common/s21_shared_func.h"
#include "../common/s21_string.h"

#define PROG_NAME "s21_grep"
#define CON_STDIN_NAME "(standard input)"
#define CON_FLAGS_COUNT 10
#define CON_SIMPLE_KEYS_COUNT 10
#define CON_KEYS_COUNT 10
#define CON_PMATCH_SIZE 1
#define CON_FLAG_e flags[0]
#define CON_FLAG_i flags[1]
#define CON_FLAG_v flags[2]
#define CON_FLAG_c flags[3]
#define CON_FLAG_l flags[4]
#define CON_FLAG_n flags[5]
#define CON_FLAG_h flags[6]
#define CON_FLAG_s flags[7]
#define CON_FLAG_f flags[8]
#define CON_FLAG_o flags[9]

int grep_file(char *file_name, int *flags, struct key_vals *key_values,
              int values_num);
char *get_next_string(FILE *file, unsigned long *str_start,
                      unsigned long *str_end);
int check_find_values(int *flags, struct key_vals **key_values, int *values_num,
                      char **files_list, int files_num);
void check_flag_h(int *flags, char **files_list, int files_num);
int check_files_list(int *flags, char **files_list, int files_num);
int get_val_from_file(int *flags, struct key_vals **key_values, int *values_num,
                      char *file_name);
regex_t *regex_array(int *rx_len, int *flags, struct key_vals *key_values,
                     int values_num);
FILE *open_file(char *file_name, const int *flags);
char *get_next_string_stdin();
void print_help();
int prepare_before_grep(int *flags, char **files_list, int files_num,
                        struct key_vals **key_values, int *values_num);
void print_found(const char *file_name, const int *flags, int str_num,
                 const char *str);
int find_all_in_str(char *str, regex_t *regex_array, int rx_len,
                    const int *flags, const char *file_name, int str_num);

int main(int argc, char **argv) {
  char **files_list = NULL;
  int files_num = 0, res = 1, values_num = 0;
  int flags[CON_FLAGS_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  struct key_vals *key_values = NULL;

  // help check
  if (s21_find_help(argc, argv)) {
    print_help();
    res = 0;
  }
  if (res) {
    struct simple_key_arg simple_keys[CON_SIMPLE_KEYS_COUNT] = {
        {'e', 0, 1}, {'i', 1, 0}, {'v', 2, 0}, {'c', 3, 0}, {'l', 4, 0},
        {'n', 5, 0}, {'h', 6, 0}, {'s', 7, 0}, {'f', 8, 1}, {'o', 9, 0}};

    struct key_arg keys[CON_KEYS_COUNT] = {
        {"--regexp", 0, 1},
        {"--ignore-case", 1, 0},
        {"--invert-match  ", 2, 0},
        {"--count", 3, 0},
        {"--files-with-matches", 4, 0},
        {"--line-number", 5, 0},
        {"--no-filename", 6, 0},
        {"--no-messages", 7, 0},
        {"--file", 8, 1},
        {"--only-matching", 9, 0},
    };
    res = s21_find_args_files(argc, argv, simple_keys, CON_SIMPLE_KEYS_COUNT,
                              keys, CON_KEYS_COUNT, &key_values, &values_num,
                              flags, CON_FLAGS_COUNT, &files_list, &files_num,
                              PROG_NAME);
  }

  if (res)
    res = check_find_values(flags, &key_values, &values_num, files_list,
                            files_num);

  if (res)
    res = prepare_before_grep(flags, files_list, files_num, &key_values,
                              &values_num);

  if (res && !check_files_list(flags, files_list, files_num))
    res = grep_file(CON_STDIN_NAME, flags, key_values, values_num);

  for (int i = 0; res && i < files_num; i++) {
    if (files_list[i]) {
      if (s21_strcmp(files_list[i], "-") != 0)
        res = grep_file(files_list[i], flags, key_values, values_num);
      else
        res = grep_file(CON_STDIN_NAME, flags, key_values, values_num);
    }
  }

  s21_free_before_exit(files_list, files_num, key_values, values_num);
  return res;
}

int grep_file(char *file_name, int *flags, struct key_vals *key_values,
              int values_num) {
  if (file_name == NULL) return 0;

  unsigned long str_start = 0, str_end = 0;
  int res = 1, rx_len = 0;

  regex_t *rx = regex_array(&rx_len, flags, key_values, values_num);
  if (!rx) res = 0;

  FILE *file = open_file(file_name, flags);

  if (file && res) {
    int flag = 1, str_num = 0, match_count = 0;
    while (flag) {
      char *str = NULL;
      if (s21_strcmp(file_name, CON_STDIN_NAME) != 0)
        str = get_next_string(file, &str_start, &str_end);
      else
        str = get_next_string_stdin();
      if (str == NULL) {
        flag = 0;
      } else {
        str_num++;
        if (CON_FLAG_o) {
          find_all_in_str(str, rx, rx_len, flags, file_name, str_num);
        } else {
          int match = 0;
          for (int i = 0; i < rx_len; i++) {
            if (regexec(&rx[i], str, 0, NULL, 0) == 0) match = 1;
          }
          if ((CON_FLAG_v && !match) || (!CON_FLAG_v && match) && !CON_FLAG_o) {
            print_found(file_name, flags, str_num, str);
            match_count++;
          }
        }
      }
      free(str);
    }
    if (CON_FLAG_c) {
      if (!CON_FLAG_h) printf("%s:", file_name);
      printf("%d\n", match_count);
    }
    if (CON_FLAG_l && match_count > 0) printf("%s\n", file_name);
    fflush(stdout);
  }
  if ((file != NULL) && (s21_strcmp(file_name, CON_STDIN_NAME) != 0))
    fclose(file);

  for (int i = 0; i < rx_len; i++) {
    regfree(&rx[i]);
  }
  free(rx);
  return res;
}

char *get_next_string(FILE *file, unsigned long *str_start,
                      unsigned long *str_end) {
  if (file == NULL) return NULL;
  int ch = fgetc(file);
  char *str = NULL;
  if (ch != EOF) {
    *str_start = ftell(file);
    while ((ch != EOF) && (ch != '\n')) {
      ch = fgetc(file);
    }
    *str_end = ftell(file);

    unsigned long str_len = *str_end - (*str_start - 1);
    if (ch == EOF) str_len++;
    str = (char *)malloc(sizeof(char) * str_len + 1);
    if (str) {
      fseek(file, *str_start - 1, SEEK_SET);
      for (unsigned long i = 0; i < str_len; i++) {
        str[i] = fgetc(file);
      }
      if (ch == EOF) str[str_len - 1] = '\n';
      str[str_len] = '\0';
    }
  }
  return str;
}

int check_find_values(int *flags, struct key_vals **key_values, int *values_num,
                      char **files_list, int files_num) {
  int res = 1;
  if (!CON_FLAG_e && !CON_FLAG_f) {
    if (files_num > 0) {
      if (s21_copy_arg_value(key_values, values_num, 0, files_list[0])) {
        free(files_list[0]);
        files_list[0] = NULL;
        CON_FLAG_e = 1;
      } else
        res = 0;
    } else {
      fprintf(stderr, "Usage: ./s21_grep [OPTION]... PATTERNS [FILE]...\n");
      fprintf(stderr, "Try 'grep --help' for more information.\n");
      res = 0;
    }
  }
  return res;
}

int get_val_from_file(int *flags, struct key_vals **key_values, int *values_num,
                      char *file_name) {
  int res = 1;
  if (s21_check_file(file_name, PROG_NAME, CON_FLAG_s)) {
    FILE *file = NULL;
    file = fopen(file_name, "r");
    if (file) {
      rewind(file);
      unsigned long str_start = 0, str_end = 0;
      int flag = 1;
      while (flag && res) {
        char *str = get_next_string(file, &str_start, &str_end);
        if (str == NULL)
          flag = 0;
        else {
          if (str[s21_strlen(str) - 1] == '\n') str[s21_strlen(str) - 1] = '\0';
          res = s21_copy_arg_value(key_values, values_num, 0, str);
          CON_FLAG_e = 1;
        }
        free(str);
      }
      fclose(file);
    } else
      res = 0;
  } else
    res = 0;

  return res;
}

int prepare_before_grep(int *flags, char **files_list, int files_num,
                        struct key_vals **key_values, int *values_num) {
  int res = 1;
  if (CON_FLAG_l) {
    CON_FLAG_c = 0;
    CON_FLAG_h = 0;
    CON_FLAG_o = 0;
  }
  if (CON_FLAG_c) {
    CON_FLAG_o = 0;
  }

  if (CON_FLAG_f) {
    for (int i = 0; i < *values_num && res; i++) {
      if ((*key_values)[i].position == 8) {
        res = get_val_from_file(flags, key_values, values_num,
                                (*key_values)[i].value);
      }
    }
  }

  if (!CON_FLAG_l) check_flag_h(flags, files_list, files_num);
  return res;
}

void check_flag_h(int *flags, char **files_list, int files_num) {
  int files_count = 0;
  for (int i = 0; i < files_num; i++) {
    if (files_list[i] != NULL) files_count++;
  }
  if (files_count < 2) CON_FLAG_h = 1;
}

int check_files_list(int *flags, char **files_list, int files_num) {
  int res = 0;
  for (int i = 0; i < files_num; i++) {
    if (files_list[i] != NULL) res++;
  }
  return res;
}

regex_t *regex_array(int *rx_len, int *flags, struct key_vals *key_values,
                     int values_num) {
  for (int i = 0; i < values_num; i++) {
    if (key_values[i].position == 0) *rx_len = *rx_len + 1;
  }
  regex_t *rx = (regex_t *)malloc(sizeof(regex_t) * (*rx_len));
  if (rx) {
    for (int i = 0, j = 0; i < values_num; i++) {
      if (key_values[i].position == 0) {
        regcomp(&rx[j], key_values[i].value, CON_FLAG_i == 1 ? REG_ICASE : 0);
        j++;
      }
    }
  }
  return rx;
}

FILE *open_file(char *file_name, const int *flags) {
  FILE *file = NULL;
  if (s21_strcmp(file_name, CON_STDIN_NAME) != 0) {
    if (s21_check_file(file_name, PROG_NAME, CON_FLAG_s)) {
      file = fopen(file_name, "r");
      rewind(file);
    }
  } else {
    rewind(stdin);
    file = stdin;
  }
  return file;
}

void print_found(const char *file_name, const int *flags, int str_num,
                 const char *str) {
  if (!CON_FLAG_h && !CON_FLAG_c && !CON_FLAG_l) printf("%s:", file_name);
  if (CON_FLAG_n && !CON_FLAG_c && !CON_FLAG_l) printf("%d:", str_num);
  if (!CON_FLAG_c && !CON_FLAG_l) printf("%s", str);
}

int find_all_in_str(char *str, regex_t *regex_array, int rx_len,
                    const int *flags, const char *file_name, int str_num) {
  char *tempstr = str;
  int fl = 1, match = 0, offset = 0;
  while (fl) {
    regmatch_t pmatch[rx_len][1];
    for (int i = 0; i < rx_len; i++) {
      if (regexec(&regex_array[i], tempstr, 1, pmatch[i], 0) == 0)
        match = 1;
      else
        pmatch[i][0].rm_so = -1;
    }
    if (match) {
      int min_pos = 0, temp = -1;
      for (int i = 0; i < rx_len; i++) {
        if ((pmatch[i][0].rm_so != -1) && (temp == -1)) {
          temp = pmatch[i][0].rm_so;
          min_pos = i;
        }
        if ((pmatch[i][0].rm_so != -1) && (temp > pmatch[i][0].rm_so)) {
          temp = pmatch[i][0].rm_so;
          min_pos = i;
        }
      }
      if (!CON_FLAG_h && !CON_FLAG_v) printf("%s:", file_name);
      if (CON_FLAG_n && !CON_FLAG_v) printf("%d:", str_num);
      if (!CON_FLAG_v) {
        for (int i = pmatch[min_pos][0].rm_so; i < pmatch[min_pos][0].rm_eo;
             i++)
          putchar(tempstr[i]);
        putchar('\n');
      }
      match = 0;

      offset = offset + pmatch[min_pos][0].rm_eo;
      tempstr = str + offset;
    } else
      fl = 0;
  }
}

char *get_next_string_stdin() {
  char *str = NULL;
  int ch = fgetc(stdin);
  if (ch != EOF) {
    str = (char *)malloc(sizeof(char) * 1);
    if (str) {
      int position = 0, flag = 1;
      str[position] = ch;
      while (flag) {
        ch = fgetc(stdin);
        if ((ch == '\n') || (ch == EOF)) {
          flag = 0;
        }
        if (ch != EOF) {
          position++;
          char *temp = realloc(str, sizeof(char) * (position + 1));
          if (temp) {
            if (str) free(str);
            str = temp;
          }
          str[position] = ch;
        }
      }
      position++;
      char *temp = realloc(str, sizeof(char) * (position + 1));
      if (temp) {
        if (str) free(str);
        str = temp;
      }
      str[position] = '\0';
    }
  }
  return str;
}

void print_help() {
  printf("Использование: grep [ПАРАМЕТР]… ШАБЛОНЫ [ФАЙЛ]…\n");
  printf("Поиск ШАБЛОНОВ в каждом ФАЙЛЕ.\n");
  printf("Пример: grep -i 'hello world' menu.h main.c\n");
  printf(
      "ШАБЛОНОВ можно указать несколько, разделяя их символом новой "
      "строки.\n\n");
  printf("Шаблон выбора и его интерпретация:\n");
  printf("  -e, --regexp=ШАБЛОНЫ   использовать ШАБЛОНЫ для поиска\n");
  printf("  -f, --file=ФАЙЛ        брать ШАБЛОНЫ из ФАЙЛа\n");
  printf("  -i, --ignore-case      игнорировать различие регистра\n\n");
  printf("Разное:\n");
  printf("  -s, --no-messages         не показывать сообщения об ошибках\n");
  printf("  -v, --invert-match        выбирать не подходящие строки\n");
  printf(
      "      --help                показать эту справку и закончить "
      "работу\n\n");
  printf("Управление выводом:\n");
  printf(
      "  -n, --line-number         печатать номер строки вместе с выходными "
      "строками\n");
  printf("  -h, --no-filename         не начинать вывод с имени файла\n");
  printf(
      "  -o, --only-matching       показывать только совпавшие непустые части "
      "строк\n");
  printf(
      "  -l, --files-with-matches  печатать только имена ФАЙЛОВ с выбранными "
      "строками\n");
  printf(
      "  -c, --count               печатать только количество выбранных строк "
      "на ФАЙЛ\n\n");
  printf(
      "Если в качестве ФАЙЛА указан символ «-», то читается стандартный "
      "ввод.\n");
  printf("Если ФАЙЛ не указан, то читается стандартный ввод.\n");
  printf("Если указано менее двух ФАЙЛОВ, то предполагается -h.\n");
}