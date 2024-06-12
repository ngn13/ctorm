#include <ctorm/all.h>
#include <stdlib.h>
#include <stdio.h>

void hello_world(req_t* req, res_t* res){
  // send the "hello world!" message
  RES_SEND("hello world!");
}

int main(){
  // create a new config
  app_config_t config;
  app_config_new(&config);

  // disable logging for better benchmarking
  config.disable_logging = true;

  // create new app
  app_t *app = app_new(&config);

  // setup the hello_world route
  GET("/", hello_world);

  // start the app
  APP_RUN("127.0.0.1:8080");

  // cleanup when done
  app_free(app);
}
