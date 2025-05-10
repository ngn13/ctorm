#pragma once

#ifndef CTROM_EXPORT

typedef struct ctorm_pair {
  char              *key, *value;
  struct ctorm_pair *next;
} ctorm_pair_t;

ctorm_pair_t *ctorm_pair_add(ctorm_pair_t **head, char *key, char *value);
#define ctorm_pair_next(head, cur)                                             \
  for (ctorm_pair_t *cur = head; cur != NULL; cur = cur->next)
ctorm_pair_t *ctorm_pair_find(ctorm_pair_t *head, char *key);
void          ctorm_pair_free(ctorm_pair_t *head);

#endif
