#include <ctorm/macros.h>
#include <stdlib.h>
#include <stdio.h>

void hello_world(req_t* req, res_t* res){
  RES_SEND("hello world!");
}

int main(){
  log_set(false);
  app_t *app = app_new();

  GET("/", hello_world);
  APP_RUN("127.0.0.1:8080");
}
