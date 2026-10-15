#include "s21_string.h"

#include <stdio.h>
#include <stdlib.h>

int s21_strlen(const char *string) {
  int len = 0;
  if (string) {
    for (; *(string + len); len++) {
    };
  }
  return len;
}

int s21_strcmp(const char *string1, const char *string2) {
  if (!string1 && !string2) return 0;
  if (string1 && !string2) return 1;
  if (!string1 && string2) return -1;
  for (; *string1 && *string1 == *string2; string1++, string2++) {
  };
  int res = *string1 - *string2;
  if (res > 0) return 1;
  if (res < 0) return -1;
  return 0;
}

char *s21_strcpy(char *destination_str, const char *source_str) {
  for (; *source_str; source_str++, destination_str++) {
    *destination_str = *source_str;
  }
  *destination_str = '\0';
  return destination_str;
}

char *s21_strcat(char *dest_string, const char *src_string) {
  if (!src_string) return dest_string;
  char *result = dest_string;
  dest_string = dest_string + s21_strlen(dest_string);
  for (; *src_string; src_string++, dest_string++) {
    *dest_string = *src_string;
  }
  // dest_string = '\0';
  return result;
}

char *s21_strchr(char *string, int symbol) {
  if (!string) return NULL;
  if (!symbol) return string;
  for (; *string; string++) {
    if (*string == symbol) return string;
  }
  return NULL;
}

int s21_compare(const char *string1, const char *string2) {
  while (*string1 && *string2) {
    if (*string1 != *string2) {
      return 0;
    }
    string1++;
    string2++;
  }

  return (*string2 == '\0');
}

char *s21_strstr(char *string1, const char *string2) {
  while (*string1 != '\0') {
    if ((*string1 == *string2) && s21_compare(string1, string2)) return string1;
    string1++;
  }

  return NULL;
}

char *s21_strtok(char *string, const char *delimiter) {
  if (!string) return NULL;
  if (!delimiter) return string;
  char *temp_p = string;
  const char *temp_d = delimiter;
  for (; *string; string++) {
    delimiter = temp_d;
    for (; *delimiter; delimiter++) {
      if (*string == *delimiter) {
        *string = '\0';
        return temp_p;
      }
    }
  }
  return NULL;
}