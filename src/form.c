#include "../include/table.h"
#include "../include/form.h"

#include <stdlib.h>

char *form_get(form_t *f, char *k) {
  table_node_t *node = table_get(f, k);
  return NULL == node ? NULL : node->value;
}
