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

extern "C"
{

  void *WS::_listen(void *args)
  {
    WS *self = (WS *)(args);
    extern int listen(int, int);

    if (!self->_port)
    {
      perror("Socket error: port not set.");
      pthread_exit(nullptr);
    }

    if (listen(self->_socket, 10) < 0)
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
    extern int setsockopt(int, int, int, const void *, socklen_t);

    _address.sin_family = AF_INET;
    _address.sin_port = _port;
    _address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    _socket = socket(_address.sin_family, SOCK_STREAM, 0);

    setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, (void *)1, sizeof(int));

    if (bind(_socket, (struct sockaddr *)&(_address), sizeof(sockaddr_in)) < 0)
    {
      printf("error: server not bound to address: %s\n", gai_strerror(_socket));
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
    if(p) {
      for(size_t i = 0; i < strlen(p); i++) 
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
    _dst_addr.sin_addr.s_addr = htonl(*_dst_ipv4s[0]);
    _dst_addr.sin_family = AF_INET6;
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
    extern ssize_t send(int sockfd, const void *buf, size_t len, int flags);
    const char *requestFmt = "GET /%s HTTP/1.1\
        Host: %s\
        Upgrade: websocket\
        Connection: Upgrade\
        Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\
        Sec-WebSocket-Protocol: chat, superchat\
        Sec-WebSocket-Version: 13\n\n";

    struct addrinfo *info;

    // printf("sizeof(*info): %lu\n", sizeof(*info));

    if (int err = getaddrinfo("ws.finnhub.io", "http", NULL, &info) < 0)
    {
      printf("failed to obtain address information: %s\n", strerror(err));
    }

    printf("family: %d\nname: %s\nprotocol:%d\nsocket:%d\n", info->ai_family, info->ai_canonname, info->ai_protocol, info->ai_socktype);


    char *requestStr = (char *)calloc(strlen(requestFmt) + strlen(_query) + strlen(_host), sizeof(char));
    snprintf(requestStr, strlen(requestStr), requestFmt, _query, _host);

    if (ssize_t err = send(_socket, requestStr, strlen(requestStr), 0) < 0)
    {
      printf("failed to establish connection: %s\n", gai_strerror(err));
    }

    free(requestStr);
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
    if ((sendRes = send(_new_socket, (const void *)msg, strlen(msg), 0)) < 0)
    {
      printf("send error: %s\n", gai_strerror(_new_socket));
    }
  }
}