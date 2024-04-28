#include "../include/macros.h"

void handle_notfound(req_t *req, res_t *res) {
  res->code = 404;
  RES_SENDFILE("./example/404.html");
}

void handle_post(req_t *req, res_t *res) {
  body_t *body = req_body_parse(req);
  if (NULL == body)
    return RES_SEND("bad body");

  char *msg = req_body_get(body, "msg");
  if (NULL == msg)
    return RES_SEND("bad body");

  RES_RENDER_ADD("msg", msg);
  RES_RENDER("./example/template/post.html");

  info("Message: %s", msg);
  RES_SET("Cool", "yes");
  req_body_free(body);
}

void handle_get(req_t *req, res_t *res) {
  RES_RENDER("./example/template/index.html");
}

int main() {
  app_t *app = app_new();

  GET("/", handle_get);
  POST("/post", handle_post);

  APP_STATIC("/static", "./example/static");
  APP_ALL(handle_notfound);

  if (!APP_RUN("0.0.0.0:8080"))
    error("app failed: %s\n", geterror());
}
