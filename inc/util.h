#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct pair {
  char        *key, *value;
  struct pair *next;
} pair_t;

pair_t *pair_add(pair_t **head, char *key, char *value);
#define pair_next(head, cur) for (pair_t *cur = head; cur != NULL; cur = cur->next)
pair_t *pair_find(pair_t *head, char *key);
void    pair_free(pair_t *head);

#define eq(s1, s2) (strcmp(s1, s2) == 0)
#define truncate_buf(buf, size, indx, ch)                                                                              \
  if (size >= indx && buf[size - indx] == ch)                                                                          \
  buf[size - indx] = 0

bool startswith(char *, char *);
bool endswith(char *, char *);

#define file_canread(p) (access(p, O_RDONLY) == 0)
bool file_size(char *, uint64_t *);

void  urldecode(char *);
void  stolower(char *, char *);
char *join(char *, char *);
bool  contains(char *, char);
bool  validate(char *, char *, char);
int   digits(int);
bool  is_digit(char);
bool  is_letter(char);
bool  path_matches(char *, char *);
