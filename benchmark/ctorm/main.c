#include <ctorm/ctorm.h>
#include <stdlib.h>
#include <stdio.h>

void hello_world(req_t* req, res_t* res){
  res_send(res, "hello world!");
}

int main(){
  log_set(false);
  app_init();
  GET("/", hello_world);
  app_run("127.0.0.1:8080");
}
