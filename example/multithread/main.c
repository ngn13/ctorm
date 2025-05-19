#include <ctorm.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

struct args {
  ctorm_app_t *app;
  const char  *addr;
};

char *name = NULL;

void hello_one(ctorm_req_t *req, ctorm_res_t *res) {
  char *new_name = REQ_QUERY("name");

  if (NULL == new_name) {
    RES_BODY("please specify a name");
    RES_CODE(404);
    return;
  }

  free(name);
  name = strdup(new_name);
  RES_BODY("name has been saved, visit the other server");
}

void hello_two(ctorm_req_t *req, ctorm_res_t *res) {
  if (NULL == name) {
    RES_BODY("visit the other server and specify your name");
    return;
  }

  RES_FMT("hello %s", name);
}

void *run(void *_args) {
  struct args *args = (void *)_args;
  ctorm_info("starting app %p on %s", args->app, args->addr);

  if (!ctorm_app_run(args->app, args->addr))
    ctorm_fail("failed to start %p: %s", args->app, ctorm_error());

  return NULL;
}

int main() {
  pthread_t thread;

  ctorm_app_t *app_one = ctorm_app_new(NULL);
  ctorm_app_t *app_two = ctorm_app_new(NULL);

  ctorm_app_add(app_one, CTORM_HTTP_GET, "/", hello_one);
  ctorm_app_add(app_two, CTORM_HTTP_GET, "/", hello_two);

  struct args args_one = {app_one, "127.0.0.1:8085"};
  struct args args_two = {app_two, "127.0.0.1:8086"};

  if (pthread_create(&thread, NULL, run, &args_two) != 0) {
    ctorm_fail("failed to create the thread: %s", ctorm_error());
    return EXIT_FAILURE;
  }

  run(&args_one);

  ctorm_app_stop(app_two);
  pthread_join(thread, NULL);

  ctorm_app_free(app_one);
  ctorm_app_free(app_two);

  return EXIT_SUCCESS;
}
