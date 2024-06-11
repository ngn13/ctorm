#include <ctorm/macros.h>
#include <stdlib.h>
#include <stdio.h>

void hello_world(req_t* req, res_t* res){
  RES_SEND("hello world!");
}

int main(){
  app_config_t config;
  app_config_new(&config);

  config.disable_logging = true;
  app_t *app = app_new(&config);

  GET("/", hello_world);
  APP_RUN("127.0.0.1:8080");

  app_free(app);
}
