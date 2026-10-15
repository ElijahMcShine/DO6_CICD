#ifndef S21_STRING_H
#define S21_STRING_H

int s21_strlen(const char *string);
int s21_strcmp(const char *string1, const char *string2);
char *s21_strcpy(char *dest_string, const char *src_string);
char *s21_strcat(char *dest_string, const char *src_string);
char *s21_strchr(char *string, int symbol);
char *s21_strstr(char *string1, const char *string2);
char *s21_strtok(char *string, const char *delimiter);
int s21_compare(const char *string1, const char *string2);

#endif