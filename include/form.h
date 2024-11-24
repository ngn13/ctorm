#pragma once

#include "table.h"

typedef table_t form_t;

#define form_init(f) table_init(f, NULL, NULL)
#define form_free(f) table_free(f)

char *form_get(form_t *f, char *k);
