#define _GNU_SOURCE
#define __USE_GNU

#include "socket.h"
#include "error.h"

#include "pool.h"
#include "conn.h"
#include "uri.h"
#include "log.h"

#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <netdb.h>
#include <errno.h>

// stores data to pass to the threads
struct ctorm_socket_data {
  ctorm_app_t *app;
  ctorm_conn_t con;
};

// request thread lock/unlock macro
#define socket_lock()                                                          \
  if ((data)->app->config->lock_request)                                       \
  pthread_mutex_lock(&(data)->app->req_mutex)
#define socket_unlock()                                                        \
  if ((data)->app->config->lock_request)                                       \
  pthread_mutex_unlock(&(data)->app->req_mutex)

// a neat debug macro
#define socket_debug(f, ...)                                                   \
  debug("(" FG_BOLD "socket " FG_CYAN "%d" FG_RESET ") " f,                    \
      data->con.socket,                                                        \
      ##__VA_ARGS__)

void _ctorm_socket_free(struct ctorm_socket_data *data) {
  ctorm_conn_close(&data->con);
  memset(data, 0, sizeof(*data));
  free(data);
}

void _ctorm_socket_handle(void *_data) {
  struct ctorm_socket_data *data = _data;
  socket_debug("handling new connection");

  bool ret = false, persist = false, log = !data->app->config->disable_logging;
  struct timeval start, end;

  // define the HTTP request and the response
  ctorm_req_t req;
  ctorm_res_t res;

  do {
    // initialize the HTTP request and the response
    ctorm_req_init(&req, &data->con);
    ctorm_res_init(&res, &data->con);

    // if disabled, remove the server header from the response
    if (!data->app->config->server_header)
      ctorm_res_del(&res, CTORM_HTTP_SERVER);

    // receive the HTTP request
    ret         = ctorm_req_recv(&req);
    res.version = req.version;
    res.code    = req.code;

    /*

     * if request logging is not disabled, then store the current time to
     * later calculate the processing time of the request, which is used for
     * logging the request and response

    */
    if (log)
      gettimeofday(&start, NULL);

    // route the request if we successfuly received a HTTP request
    if (ret) {
      /*

       * we lock the request mutex of the app before actually routing it, this
       * means only one request can be routed in a given time, however this lock
       * can be disabled with the lock_request configuration option

      */
      socket_lock();
      ctorm_app_route(data->app, &req, &res);
      socket_unlock();
    }

    // debug print if we failed to receive a HTTP request
    else
      socket_debug("received an invalid HTTP request");

    persist = ctorm_req_persist(&req);
    socket_debug("sending a %d response", res.code);

    // send the complete response
    if (!ctorm_res_send(&res)) {
      socket_debug("failed to send the response: %s", ctorm_error());
      goto next;
    }

    // finish process time measurement and log the request
    if (ret && log) {
      gettimeofday(&end, NULL);

      uint64_t env_val   = 1000000 * end.tv_sec + end.tv_usec;
      uint64_t start_val = 1000000 * start.tv_sec + start.tv_usec;

      socket_lock();
      log(&req, &res, env_val - start_val);
      socket_unlock();
    }

  next:
    // reset the request and response data
    ctorm_req_free(&req);
    ctorm_res_free(&res);
  } while (persist);

  // close & free the connection
  socket_debug("closing connection");
  _ctorm_socket_free(data);
}

void _ctorm_socket_kill(void *_data) {
  struct ctorm_socket_data *data = _data;
  shutdown(data->con.socket, SHUT_RDWR);
}

bool _ctorm_socket_new(ctorm_app_t *app, int socket, struct sockaddr *addr) {
  struct ctorm_socket_data *data = calloc(1, sizeof(*data));

  if (NULL == data) {
    errno = CTORM_ERR_ALLOC_FAIL;
    return false;
  }

  // setup the socket data for the connection
  data->app        = app;
  data->con.socket = socket;
  memcpy(&data->con.addr, addr, sizeof(data->con.addr));

  // make sure we don't have too many connections in the pool
  if (ctorm_pool_remaining(app->pool) >= app->config->max_connections) {
    debug("reached connection limit, waiting for one to finish");
    ctorm_pool_wait(app->pool, 1);
  }

  // add new connection to the pool
  if (!ctorm_pool_add(
          app->pool, _ctorm_socket_handle, _ctorm_socket_kill, data)) {
    free(data);
    return false; // errno set by ctorm_pool_add()
  }

  return true;
}

bool ctorm_socket_resolve(char *addr, struct addrinfo *info) {
  struct addrinfo *hostinfo = NULL, *cur = NULL;
  ctorm_uri_t      uri;
  bool             ret = false;

  ctorm_uri_init(&uri);

  if (NULL == ctorm_uri_parse_host(&uri, addr))
    return false; // errno set by ctorm_uri_parse_host()

  // resolve the host name
  if (getaddrinfo(uri.host, NULL, NULL, &hostinfo) != 0 || NULL == info) {
    ctorm_uri_free(&uri);
    return false; // errno set by getaddrinfo()
  }

  for (cur = hostinfo; NULL != cur; cur = cur->ai_next) {
    if (AF_INET == cur->ai_family) {
      ((struct sockaddr_in *)cur->ai_addr)->sin_port = htons(uri.port);
      break;
    }

    else if (AF_INET6 == cur->ai_family) {
      ((struct sockaddr_in6 *)cur->ai_addr)->sin6_port = htons(uri.port);
      break;
    }
  }

  // copy the addrinfo for the host to the provided info structure
  if (NULL != cur) {
    memcpy(info, cur, sizeof(*cur));
    ret = true;
  }

  // free the host addrinfo
  freeaddrinfo(hostinfo);
  ctorm_uri_free(&uri);

  return ret;
}

bool ctorm_socket_set_opts(ctorm_app_t *app, int sockfd) {
  struct timeval timeout;
  int            flag = 1;

  // clear the timeout structure
  memset(&timeout, 0, sizeof(timeout));

#ifdef TCP_QUICKACK
  /*

   * TCP delayed acknowledgment, buffers and combines multiple ACKs to reduce
   * overhead it may delay the ACK response by up to 500ms, and we don't want
   * that because slow bad fast good

  */
  if (setsockopt(sockfd, IPPROTO_TCP, TCP_QUICKACK, &flag, sizeof(flag)) < 0) {
    ctorm_error_set(app, CTORM_ERR_SOCKET_OPT_FAIL);
    return false;
  }
#endif

  /*

   * Nagle's algorithm buffers and combines outgoing packets to solve the
   * "small-packet problem", we want to send all the packets as fast as possible
   * so we can disable this buffering with TCP_NODELAY

  */
  if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) < 0) {
    ctorm_error_set(app, CTORM_ERR_SOCKET_OPT_FAIL);
    return false;
  }

  // set the socket timeout
  if (app->config->tcp_timeout > 0) {
    timeout.tv_sec  = app->config->tcp_timeout;
    timeout.tv_usec = 0;

    setsockopt(
        sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
  }

  // make the socket blocking
  if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) & (~O_NONBLOCK)) == -1) {
    errno = CTORM_ERR_FCNTL_FAIL;
    return false;
  }

  return true;
}

bool ctorm_socket_start(ctorm_app_t *app, char *addr) {
  int             ssock = 0, csock = 0, flag = 1;
  struct sockaddr caddr;
  struct addrinfo info;
  socklen_t       clen = sizeof(caddr);
  bool            ret  = false;

  // clear the client address and address info structure
  memset(&caddr, 0, sizeof(caddr));
  memset(&info, 0, sizeof(info));

  // parse the host to get the addrinfo structure
  if (!ctorm_socket_resolve(addr, &info)) {
    debug("failed to resolve the address: %s", ctorm_error());
    ctorm_error_set(app, CTORM_ERR_RESOLVE_FAIL);
    goto end;
  }

  // create a new TCP socket
  if ((ssock = socket(info.ai_family, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    debug("failed to create socket: %s", strerror(errno));
    ctorm_error_set(app, CTORM_ERR_SOCKET_FAIL);
    goto end;
  }

  debug("created socket %d for %p", ssock, app);

  // prevent EADDRINUSE
  if (setsockopt(ssock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
    debug("failed to set the REUSEADDR: %s", strerror(errno));
    ctorm_error_set(app, CTORM_ERR_SOCKET_OPT_FAIL);
    goto end;
  }

  // bind and listen on the provided host
  if (bind(ssock, info.ai_addr, info.ai_addrlen) < 0) {
    debug("failed to bind the socket: %s", strerror(errno));
    ctorm_error_set(app, CTORM_ERR_BIND_FAIL);
    goto end;
  }

  if (listen(ssock, app->config->max_connections) < 0) {
    debug("failed to listen socket: %s", strerror(errno));
    ctorm_error_set(app, CTORM_ERR_LISTEN_FAIL);
    goto end;
  }

  // new connection handler loop
  while (app->running && (csock = accept(ssock, &caddr, &clen)) != -1) {
    debug("new connection: %d", csock);

    if (!ctorm_socket_set_opts(app, csock)) {
      debug("setsockopt failed for %d: %s", csock, strerror(errno));
      errno = CTORM_ERR_SOCKET_OPT_FAIL;
      goto end;
    }

    if (!_ctorm_socket_new(app, csock, &caddr)) {
      debug("failed to create a new socket for %d: %s", ctorm_error());
      goto end; // errno set by _ctorm_socket_new()
    }

    // clear client address and address length
    memset(&caddr, 0, sizeof(caddr));
    clen = sizeof(caddr);
  }

  // check if accept() got interrupted
  if (csock == -1 && errno != EINTR) {
    debug("failed to accept new connection: %s", strerror(errno));
    ctorm_error_set(app, CTORM_ERR_ACCEPT_FAIL);
    goto end;
  }

  // app is no longer running
  debug("stopping the connection handler");
  ctorm_error_clear(app);
  ret = true;

end:
  // close the server socket
  if (ssock != -1)
    close(ssock);

  // close the recent client socket
  if (csock != -1)
    close(csock);

  return ret;
}
