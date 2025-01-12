#include "util.h"

#include <sys/stat.h>
#include <stdbool.h>
#include <errno.h>

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>

pair_t *pair_add(pair_t **head, char *key, char *value) {
  if (NULL == key) {
    errno = EINVAL;
    return NULL;
  }

  pair_t *new = malloc(sizeof(pair_t));
  bzero(new, sizeof(pair_t));

  if (NULL != head) {
    new->next = *head;
    *head     = new;
  }

  new->key   = key;
  new->value = value;

  return new;
}

pair_t *pair_find(pair_t *head, char *key) {
  if (NULL == head || NULL == key) {
    errno = EINVAL;
    return NULL;
  }

  pair_next(head, cur) {
    if (eq(head->key, cur->key))
      return cur;
  }

  errno = EFAULT;
  return NULL;
}

void pair_free(pair_t *head) {
  if (NULL == head)
    return;

  pair_t *cur = NULL, *next = NULL;

  for (cur = head; cur != NULL; cur = next) {
    next = cur->next;
    free(cur);
  }
}

bool startswith(char *str, char *pre) {
  if (NULL == str || NULL == pre)
    return false;

  size_t prel = strlen(pre);
  if (prel > strlen(str))
    return false;

  return strncmp(pre, str, prel) == 0;
}

bool endswith(char *str, char *suf) {
  if (NULL == str || NULL == suf)
    return false;

  size_t sufl = strlen(suf);
  size_t strl = strlen(str);

  if (sufl > strl)
    return false;

  return strncmp(str + (strl - sufl), suf, sufl) == 0;
}

bool file_size(char *path, uint64_t *size) {
  struct stat st;

  if (stat(path, &st) < 0)
    return false;

  *size = st.st_size;
  return true;
}

int digits(int n) {
  if (n < 0)
    return digits((n == INT_MIN) ? INT_MAX : -n);
  if (n < 10)
    return 1;
  return digits(n / 10) + 1;
}

char *join(char *p1, char *p2) {
  char *fp = malloc(strlen(p1) + strlen(p2) + 2);
  sprintf(fp, "%s/%s", p1, p2);
  return fp;
}

void stolower(char *src, char *dst) {
  int i = 0;
  for (; src[i] != 0; i++)
    dst[i] = tolower(src[i]);
  dst[i] = 0;
}

bool contains(char *str, char s) {
  for (char *c = str; *c != 0; c++)
    if (*c == s)
      return true;
  return false;
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

bool path_matches(char *route, char *path) {
  if (NULL == route || NULL == path)
    return false;

  if (route[0] != '/' || path[0] != '/')
    return false;

  char  *c = NULL, *save = NULL;
  size_t rsc = 0, psc = 0, sc = 0;
  size_t rlen = 0, plen = 0;
  bool   wildcard = false;

  for (c = route; *c != 0; c++) {
    if (*c == '*')
      wildcard = true;

    if (*c == '/')
      rsc++;
    rlen++;
  }

  if (!wildcard)
    return eq(route, path);

  for (c = path; *c != 0; c++) {
    if (*c == '/')
      psc++;
    plen++;
  }

  char route_cp[rlen + 1], *rsub[rsc];
  char path_cp[plen + 1], *psub[psc];

  memcpy(route_cp, route, sizeof(route_cp));
  memcpy(path_cp, path, sizeof(path_cp));

  // remove trailing '/'
  if (route_cp[rlen - 1] == '/') {
    route_cp[rlen - 1] = 0;
    rlen--;
    rsc--;
  }

  if (path_cp[plen - 1] == '/') {
    path_cp[plen - 1] = 0;
    plen--;
    psc--;
  }

  if (rsc != psc)
    return false;
  sc = rsc = psc;

  if (sc == 1) {
    // "/*" matches "/..."
    if (rlen == 2 && route_cp[1] == '*')
      return true;
  }

  rsub[0] = strtok_r(route_cp, "/", &save);
  if (NULL == rsub[0])
    return false;

  for (int i = 1; i < sc; i++)
    rsub[i] = strtok_r(NULL, "/", &save);

  psub[0] = strtok_r(path_cp, "/", &save);
  if (NULL == psub[0])
    return false;

  for (int i = 1; i < sc; i++)
    psub[i] = strtok_r(NULL, "/", &save);

  for (int i = 0; i < sc; i++) {
    if (NULL == rsub[i] && NULL != psub[i])
      return false;

    if (NULL != rsub[i] && NULL == psub[i])
      return false;

    if (rsub[i][0] == '*')
      continue;

    if (!eq(rsub[i], psub[i]))
      return false;
  }

  return true;
}
