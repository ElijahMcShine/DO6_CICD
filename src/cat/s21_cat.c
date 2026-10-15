#include <stdio.h>
#include <stdlib.h>

#include "../common/s21_args_parser.h"
#include "../common/s21_shared_func.h"
#include "../common/s21_string.h"

#define PROG_NAME "s21_cat"
#define CON_FLAGS_COUNT 7
#define CON_SIMPLE_KEYS_COUNT 14
#define CON_KEYS_COUNT 9
#define CON_MIN_VIS_CHAR 32
#define CON_SHIFT 64
#define CON_MAX_VIS_CHAR 126
#define CON_LAST_ASCII 127
#define CON_DIVIDER 128
#define CON_FLAG_b flags[0]
#define CON_FLAG_e flags[1]
#define CON_FLAG_n flags[2]
#define CON_FLAG_s flags[3]
#define CON_FLAG_t flags[4]
#define CON_FLAG_v flags[5]

void print_help();
void if_flag_bn(int ch, int new_str, int *str_num, const int *flags);
void if_flag_e(int ch);
void if_flag_s(int ch, int *modificated, int new_str, int *repeat_empty);
void if_flag_t(int ch, int *modificated);
void if_flag_v(int ch, int *modificated);
int cat_file(char *file_name, const int *flags, int *str_num);

int main(int argc, char **argv) {
  char **files_list = NULL;
  int files_num = 0, res = 1, str_num = 1, values_num = 0;
  struct key_vals *key_values = NULL;
  int flags[CON_FLAGS_COUNT] = {0, 0, 0, 0, 0, 0, 0};
  // flag_b, flag_e, flag_n, flag_s, flag_t, flag_v, flag_u;

  // help check
  if (s21_find_help(argc, argv)) {
    print_help();
    res = 0;
  }

  if (res) {
    struct simple_key_arg simple_keys[CON_SIMPLE_KEYS_COUNT] = {
        {'A', 1, 0}, {'A', 4, 0}, {'A', 5, 0}, {'b', 0, 0}, {'e', 1, 0},
        {'e', 5, 0}, {'E', 1, 0}, {'n', 2, 0}, {'s', 3, 0}, {'t', 4, 0},
        {'t', 5, 0}, {'T', 4, 0}, {'u', 6, 0}, {'v', 5, 0}};

    struct key_arg keys[CON_KEYS_COUNT] = {
        {"--show-all", 1, 0},        {"--show-all", 4, 0},
        {"--show-all", 5, 0},        {"--number-nonblank", 0, 0},
        {"--show-ends", 1, 0},       {"--number", 2, 0},
        {"--squeeze-blank", 3, 0},   {"--show-tabs", 4, 0},
        {"--show-nonprinting", 5, 0}};

    res = s21_find_args_files(argc, argv, simple_keys, CON_SIMPLE_KEYS_COUNT,
                              keys, CON_KEYS_COUNT, &key_values, &files_num,
                              flags, CON_FLAGS_COUNT, &files_list, &files_num,
                              PROG_NAME);
  }

  if (res && CON_FLAG_b) CON_FLAG_n = 0;

  if (res && files_num == 0) res = cat_file("stdin", flags, &str_num);

  for (int i = 0; res && i < files_num; i++) {
    if (s21_strcmp(files_list[i], "-") != 0)
      res = cat_file(files_list[i], flags, &str_num);
    else
      res = cat_file("stdin", flags, &str_num);
  }

  s21_free_before_exit(files_list, files_num, key_values, values_num);
  return res;
}

void if_flag_bn(int ch, int new_str, int *str_num, const int *flags) {
  if (new_str && (CON_FLAG_n || (CON_FLAG_b && ch != '\n'))) {
    printf("%6d", *str_num);
    putchar('\t');
    *str_num = *str_num + 1;
  }
}

void if_flag_e(int ch) {
  if (ch == '\n') putchar('$');
}

void if_flag_t(int ch, int *modificated) {
  if (ch == '\t') {
    putchar('^');
    putchar(ch + CON_SHIFT);
    *modificated = 1;
  }
}

void if_flag_v(int ch, int *modificated) {
  if ((ch < CON_MIN_VIS_CHAR || ch > CON_MAX_VIS_CHAR) &&
      (ch != '\t' && ch != '\n')) {
    if (ch > CON_LAST_ASCII) {
      putchar('M');
      putchar('-');
      *modificated = 1;
      ch = ch % CON_DIVIDER;
    }
    if (ch == CON_LAST_ASCII) {
      putchar('^');
      *modificated = 1;
      ch = ch - CON_SHIFT;
    }
    if (ch < CON_MIN_VIS_CHAR) {
      putchar('^');
      *modificated = 1;
      ch = ch + CON_SHIFT;
    }
    putchar(ch);
  }
}

void if_flag_s(int ch, int *modificated, int new_str, int *repeat_empty) {
  if (ch != '\n') {
    *repeat_empty = 0;
  }
  if (new_str && (*repeat_empty == 1) && (ch == '\n')) {
    *modificated = 1;
  }
  if (new_str && (*repeat_empty == 0) && (ch == '\n')) {
    *repeat_empty = 1;
  }
}

int cat_file(char *file_name, const int *flags, int *str_num) {
  FILE *file = NULL;
  int modificated = 0, repeat_empty = 0, res = 1;

  if (s21_strcmp(file_name, "stdin") != 0) {
    res = s21_check_file(file_name, PROG_NAME, 0);
    if (res) {
      file = fopen(file_name, "r");
    }
  } else {
    rewind(stdin);
    file = stdin;
  }
  if (file && res) {
    int new_str = 1, ch;
    while ((ch = fgetc(file)) != EOF) {
      if (CON_FLAG_s) if_flag_s(ch, &modificated, new_str, &repeat_empty);
      if (((CON_FLAG_b) || (CON_FLAG_n)) && !modificated)
        if_flag_bn(ch, new_str, str_num, flags);
      if ((CON_FLAG_e) && !modificated) if_flag_e(ch);
      if ((CON_FLAG_t)) if_flag_t(ch, &modificated);
      if ((CON_FLAG_v)) if_flag_v(ch, &modificated);

      if (!modificated) putchar(ch);

      modificated = 0;
      if (ch == '\n')
        new_str = 1;
      else
        new_str = 0;
    }
    fflush(stdout);
  }
  if ((file != NULL) && (s21_strcmp(file_name, "stdin") != 0)) fclose(file);
  return res;
}

void print_help() {
  printf(
      "Использование: cat [ПАРАМЕТР]… [ФАЙЛ]… (порядок не важен, это не "
      "mac)\n");
  printf("Печатает слияние ФАЙЛ(ов) на стандартный вывод.\n\n");
  printf("Если ФАЙЛ не задан или задан как -, читает стандартный ввод.\n\n");
  printf("  -A, --show-all           синоним -vET\n");
  printf(
      "  -b, --number-nonblank    нумеровать непустые строки при выводе. "
      "подавляет -n\n");
  printf("  -e                       синоним -vE\n");
  printf("  -E, --show-ends          показывать $ в конце каждой строки\n");
  printf("  -n, --number             нумеровать все строки при выводе\n");
  printf(
      "  -s, --squeeze-blank      выводить не более одной пустой строки при "
      "повторе\n");
  printf("  -t                       синоним -vT\n");
  printf("  -T, --show-tabs          показывать символы табуляции как ^I\n");
  printf("  -u                       (игнорируется)\n");
  printf(
      "  -v, --show-nonprinting   использовать запись с ^ и M-, за "
      "исключением\n");
  printf("                           символов перевода строки и табуляции\n");
  printf("      --help               показать эту справку и выйти\n\n");
  printf("Примеры:\n");
  printf(
      "  s21_cat f - g  Выводит содержимое f, потом стандартный ввод, потом "
      "g.\n");
  printf(
      "  s21_cat        Копирует стандартный ввод на стандартный вывод.\n\n");
}