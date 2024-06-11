#include "../include/util.h"
#include <sys/stat.h>
#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

bool eq(char *s1, char *s2){
  if(NULL == s1 || NULL == s2)
    return false;

  if(strlen(s1) != strlen(s2))
    return false;

  return strcmp(s1, s2) == 0;
}

bool startswith(char *str, char *pre){
  if(NULL == str || NULL == pre)
    return false;

  size_t prel = strlen(pre);
  if(prel > strlen(str))
    return false;

  return strncmp(pre, str, prel) == 0;
}

bool endswith(char *str, char *suf){
  if(NULL == str || NULL == suf)
    return false;

  size_t sufl = strlen(suf);
  size_t strl = strlen(str);

  if(sufl > strl)
    return false;

  return strncmp(str+(strl-sufl), suf, sufl) == 0;
}

bool file_read(char *path, char *buffer, size_t size) {
  int fd = open(path, O_RDONLY);
  if(fd < 0)
    return false;

  if(read(fd, buffer, size)<0)
    return false;

  close(fd);
  return true;
}

bool file_canread(char *path){
  return access(path, O_RDONLY) == 0;
}

size_t file_size(char *path) {
  struct stat st;
  if(stat(path, &st) < 0)
    return -1;

  return st.st_size;
}

int digits(int n) {
  if (n < 0)
    return digits((n == INT_MIN) ? INT_MAX : -n);
  if (n < 10)
    return 1;
  return digits(n / 10) + 1;
}

char *join(char *p1, char *p2) {
  char *fp = malloc(strlen(p1) + strlen(p2) + 1);
  sprintf(fp, "%s/%s", p1, p2);
  return fp;
}

void stolower(char *src, char *dst) {
  int i = 0;
  for (; src[i] != 0; i++)
    dst[i] = tolower(src[i]);
  dst[i] = 0;
}

bool contains(char *str, char c) {
  for (int i = 0; i < strlen(str); i++)
    if (str[i] == c)
      return true;
  return false;
}

bool is_letter(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool is_digit(char c) {
  return c >= '0' && c <= '9';
}

bool validate(char *str, char *valids, char end) {
  size_t validl = strlen(valids);
  size_t strl   = strlen(str);

  for (int c = 0; c < strl; c++) {
    bool pass = false;
    for (int v = 0; v < validl; v++) {
      if (str[c] == valids[v]) {
        pass = true;
        break;
      }
    }

    if (end != 0 && c == strl - 1 && str[c] == end)
      pass = true;

    if (!pass)
      return false;
  }
  return true;
}

void urldecode(char *str) {
  char         *curr_ptr = str;
  char         *step_ptr = str;
  unsigned char value;

  while (*curr_ptr) {
    if (*curr_ptr == '+')
      *step_ptr = ' ';
    else if (*curr_ptr == '%') {
      sscanf(curr_ptr + 1, "%02hhx", &value);
      *step_ptr = value;
      curr_ptr  = curr_ptr + 2;
    } else
      *step_ptr = *curr_ptr;
    curr_ptr++;
    step_ptr++;
  }

  *(step_ptr) = '\0';
}

char *replace(char *s, char c, char n) {
  char *copy = strdup(s);
  for (int i = 0; i < strlen(copy); i++) {
    if (copy[i] == c)
      copy[i] = n;
  }
  return copy;
}
