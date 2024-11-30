#include <sys/socket.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <dns.h>
#include "DNSQuery.h"
#include <cerrno>
#include "WS.h"
#include <openssl/err.h>
#include <openssl/ssl.h>
#ifndef __HEADERS__
#include "Headers.h"
#endif

void *WS::_listen(void *args)
{
  WS *self = (WS *)(args);

  if (!self->_port)
  {
    perror("Socket error: port not set.");
    pthread_exit(nullptr);
  }

  if (::listen(self->_socket, 10) < 0)
  {
    printf("error: server not listening: %s\n", gai_strerror(errno));
    exit(1);
  }

  char hostBuffer[1024], serviceBuffer[NI_MAXSERV];

  int error = getnameinfo(
      (struct sockaddr *)&(self->_address), sizeof(struct sockaddr),
      hostBuffer, sizeof(hostBuffer),
      serviceBuffer, sizeof(serviceBuffer),
      0);
  if (error != 0)
  {
    printf("error: %s\n", gai_strerror(errno));
    return NULL;
  }

  printf("Server is listening on http://%s:%ld/\n\n", hostBuffer, self->port());

  unsigned int sz = sizeof(struct sockaddr_in);
  for (;;)
  {
    if ((self->_new_socket = accept(self->_socket, (struct sockaddr *)&(self->_address), &sz)) < 0)
    {
      printf("error: %s\n", gai_strerror(self->_new_socket));
    }

    puts("Message received...");

    ssize_t valread = read(self->_new_socket, hostBuffer, sizeof(char[1024]));

    self->_listen_cb(hostBuffer, self);
    close(self->_new_socket);
  }

  close(self->_socket);

  return NULL;
}

void *WS::_send_async(void *arg)
{
  return nullptr;
}

WS::WS() : _dstIPCount{0} {}
WS::~WS()
{
  delete _dst_ipv4s;
  delete _dst_addrs;
  _await_all();
}

void WS::init()
{
  if (!_port)
  {
    perror("Socket error: port not set.");
    pthread_exit(nullptr);
  }
  extern int socket(int, int, int);
  // extern int setsockopt(int, int, int, const void *, socklen_t);

  _address.sin_family = AF_INET;
  _address.sin_port = _port;
  _address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  _socket = socket(AF_INET, SOCK_STREAM, 0);

  setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, (void *)1, sizeof(int));

  if (bind(_socket, (struct sockaddr *)&(_address), sizeof(sockaddr_in)) < 0)
  {
    printf("error: server not bound to address: %s\n", gai_strerror(_socket));
    close(_socket);
    return;
  }
  puts("Socket initialised");
}

void WS::init(const char *url)
{
  extern int socket(int, int, int);
  _dst_url = url;
  _protocol = Utils::reMatch(R"(^\w+?(?=://))", url);
  _host = Utils::reMatch(R"((?<=://)(([.]?([\d\w])+?)+)(?=[?/]|$))", url);
  char *p = Utils::reMatch(R"((?<=:)\d{2,4})", url);
  printf("port: %s\n", p);
  size_t portNum = 0;
  if (p)
  {
    for (size_t i = 0; i < strlen(p); i++)
    {
      char c = p[i];
    }
  }
  _query = Utils::reMatch(R"((?<=\?)(.+?$))", url);
  printf("query: %s\n", _query);
  _address.sin_family = AF_INET;
  _address.sin_port = htons(_port);

  DNSQuery *res = new DNSQuery(url);
  setDestinationAddresses(res->destinationAddresses());
  setDestinationIPs(res->destinationIPs());
  // setPort(htons(3020));
  _dst_addr.sin_addr.s_addr = htonl(*_dst_addrs[0]);
  _dst_addr.sin_family = AF_INET;
  _dst_addr.sin_port = htons(443);
  _address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  _address.sin_family = AF_INET;

  _socket = socket(_address.sin_family, SOCK_STREAM, 0);

  setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, (void *)1, sizeof(int));

  if (bind(_socket, (struct sockaddr *)&(_address), sizeof(sockaddr_in)) < 0)
  {
    fprintf(stderr, "%s%s\n", "error: server not bound to address:", gai_strerror(_socket));
    return;
  }
  fprintf(stderr, "Socket bound to %s:%hu\n", Utils::parseIPv4((void *)&(_address.sin_addr.s_addr)), _address.sin_port);

  delete res;
}

void WS::connect()
{

  struct addrinfo *info = Utils::getAddressInfo(_host);
  puts("struct addrinfo *info = Utils::getAddressInfo(\"ws.finnhub.io\");");

  close(_socket);

  printf("info->ai_protocol: %d\n", info->ai_protocol);

  _socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);

  if (::connect(_socket, (sockaddr *)&_dst_addr, info->ai_addrlen) < 0)
  {
    printf("Failed to connect: %s.\n", strerror(errno));
    close(_socket);
    freeaddrinfo(info);
    exit(EXIT_FAILURE);
  }

  char queryString[strlen(_query) + 1];
  queryString[0] = '?';
  queryString[1] = '\0';
  strcat(queryString, _query);

  Headers hh = Headers();
  hh["Host"] = _host;
  printf("hh[\"Host\"]: %s\n", hh["Host"]);
  hh["Upgrade"] = "websocket";
  printf("hh[\"Upgrade\"]: %s\n", hh["Upgrade"]);
  hh["Connection"] = "Upgrade";
  printf("hh[\"Connection\"]: %s\n", hh["Connection"]);

  printf("hh():\n%s\n", hh());

  const char *requestFmt = "GET /%s HTTP/1.1\r\nHost: %s\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Protocol: chat, superchat\r\nSec-WebSocket-Version: 13\r\n\r\n";

  char requestStr[2048];
  snprintf(requestStr, strlen(requestFmt) + strlen(queryString) + strlen(_host), requestFmt, queryString, _host);

  printf("requestStr:\n%s\n", requestStr);

  SSL_library_init();
  SSL_load_error_strings();
  OpenSSL_add_ssl_algorithms();
  const SSL_METHOD *method = TLS_client_method();

  SSL_CTX *ctx = SSL_CTX_new(method);
  if (!ctx)
  {
    perror("Unable to create ssl context.");
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }

  SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);

  SSL *ssl = SSL_new(ctx);
  SSL_set_fd(ssl, _socket);

  Utils::action abort = Utils::action([&]() -> void
                                      {
      ERR_print_errors_fp(stderr);
      SSL_free(ssl);
      close(_socket);
      freeaddrinfo(info);
      SSL_CTX_free(ctx);
      exit(EXIT_FAILURE); });

  if (!SSL_set_tlsext_host_name(ssl, "ws.finnhub.io"))
    abort();

  if (SSL_connect(ssl) <= 0)
    abort();

  ssize_t bytes_sent = SSL_write(ssl, requestStr, strlen(requestStr));
  if (bytes_sent <= 0)
    abort();

  char recv_buf[2048];
  ssize_t bytes_received = SSL_read(ssl, recv_buf, sizeof(recv_buf) - 1);
  if (bytes_received <= 0)
    abort();

  printf("recv_buf:\n%s\n", recv_buf);

  char *socket_url = Utils::reMatch(R"(https:\\/\\/.+?(?="\}\]))", recv_buf);

  printf("socket_url: %s\n", socket_url);

  char *host = Utils::reMatch(R"((?<=https:\\/\\/)(\.?[\w\d])+?(?=\\/))", socket_url);

  printf("host: %s\n", host);

  const char *conn_str_fmt = "CONNECT %s HTTP/1.1\r\nHost: %s\r\n\r\n";
  char conn_str[2048];
  snprintf(conn_str, strlen(conn_str_fmt) + strlen(socket_url) + strlen(host), conn_str_fmt, socket_url, host);

  printf("conn_str: %s\n", conn_str);

  bytes_sent = SSL_write(ssl, conn_str, strlen(conn_str));
  if (bytes_sent <= 0)
    abort();

  bytes_received = SSL_read(ssl, recv_buf, sizeof(recv_buf) - 1);
  if (bytes_received <= 0)
    abort();

  printf("recv_buf:\n%s\n", recv_buf);

  SSL_free(ssl);
  close(_socket);
  freeaddrinfo(info);
  SSL_CTX_free(ctx);
  EVP_cleanup();
}

void WS::listen(void (*cb)(char *buf, WS *ws))
{
  _listen_cb = cb;

  pthread_t *thread = (pthread_t *)malloc(sizeof(pthread_t));

  int result = pthread_create(thread, NULL, WS::_listen, this);

  if (result)
  {
    printf("Error creating thread: %s\n", gai_strerror(result));
    exit(1);
  }
  _threads[_thread_count++] = thread;
}

void WS::sendAsync(char *buf)
{
}

void WS::_await_all()
{
  for (size_t i = 0; i < _thread_count; i++)
  {
    int result = pthread_join(*_threads[i], NULL);
    if (result)
    {
      printf("thread %zu failed to join: %d\n", i, result);
    }
    free(_threads[i]);
  }
}

void WS::send(char *msg)
{
  extern ssize_t send(int, const void *, size_t, int);

  int sendRes;
  if ((sendRes = ::send(_new_socket, (const void *)msg, strlen(msg), 0)) < 0)
  {
    printf("send error: %s\n", gai_strerror(_new_socket));
  }
}
