#include "../include/ctorm.h"

void handle_notfound(req_t *req, res_t *res) {
  res->code = 404;
  res_sendfile(res, "./example/404.html");
}

void handle_post(req_t *req, res_t *res) {
  body_t *body = req_body_parse(req);
  if (NULL == body)
    return res_send(res, "bad body");

  char *msg = req_body_get(body, "msg");
  if (NULL == msg)
    return res_send(res, "bad body");

  res_render_add(res, "msg", msg);
  res_render(res, "./example/template/post.html");

  info("Message: %s", msg);
  res_set(res, "Cool", "yes");
  req_body_free(body);
}

void handle_get(req_t *req, res_t *res) {
  res_render(res, "./example/template/index.html");
}

int main() {
  app_init();

  GET("/", handle_get);
  POST("/post", handle_post);

  app_static("/static", "./example/static");
  app_all(handle_notfound);

  if (!app_run("0.0.0.0:8080"))
    error("app failed: %s\n", geterror());
}
