# Rendering 
Ctorm supports simple and limited dynamic rendering.

### Example template
```html
<h1>Message: {{msg}}</h1>
```
To render this template, you can use the request render method:
```c
req_render_add(req, "msg", "hello world!");
// or if the req_t object is called 'req', then you
// can use the macro
REQ_RENDER_ADD("msg", "hello world!");

req_render(req, "templates/example.html");
// again, you can also use the macro
REQ_RENDER("templates/example.html");
```
