#include "../include/http.h"
#include "../include/util.h"

#include <stdlib.h>
#include <string.h>

size_t http_method_sz = 6;
t_method_map http_method_map[] = {
  {.code=METHOD_GET,     .name="GET",     .body=false},
  {.code=METHOD_HEAD,    .name="HEAD",    .body=false},
  {.code=METHOD_POST,    .name="POST",    .body=true},
  {.code=METHOD_PUT,     .name="PUT",     .body=true},
  {.code=METHOD_DELETE,  .name="DELETE",  .body=true},
  {.code=METHOD_OPTIONS, .name="OPTIONS", .body=false},
};

int http_methodid(char* name) {
  for(int i = 0; i < http_method_sz; i++)
    if(eq(http_method_map[i].name, name))
      return http_method_map[i].code;
  return -1;
}

void pair_init(pairs_t* pairs){
  pairs->list = NULL;
  pairs->size = 0;
}

void pair_add(pairs_t* pairs, char* name, bool alloced){
  pairs->size++;
  if(NULL == pairs->list)
    pairs->list = malloc(sizeof(pair_t));
  else
    pairs->list = realloc(pairs->list, 
        sizeof(pair_t)*pairs->size);

  pairs->list[pairs->size-1].name = name;
  pairs->list[pairs->size-1].alloced = alloced;
  pairs->list[pairs->size-1].value = NULL;
}

void pair_set(pairs_t* pairs, char* value){
  pairs->list[pairs->size-1].value = value;
}

void pair_free(pairs_t* pairs){
  for(int i = 0; i < pairs->size; i++){
    if(pairs->list[i].alloced){
      free(pairs->list[i].name);
      free(pairs->list[i].value);
    }
  }
  free(pairs->list);
}
