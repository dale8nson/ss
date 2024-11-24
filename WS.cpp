#include <sys/socket.h>
#include <stdio.h>
#include "WS.h"
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

extern "C"
{
  void *WS::_listen(void* args)
  {
    WS *self = (WS*)(args);
    extern int listen(int, int);

    if (listen(self->socket, 10) < 0)
    {
      puts("error: server not listening");
      exit(1);
    }

    char hostBuffer[1024], serviceBuffer[NI_MAXSERV];

    int error = getnameinfo(
        (struct sockaddr *)&(self->address), sizeof(struct sockaddr),
        hostBuffer, sizeof(hostBuffer),
        serviceBuffer, sizeof(serviceBuffer),
        0);
    if (error != 0)
    {
      printf("error: %s\n", gai_strerror(error));
      return NULL;
    }

    printf("Server is listening on http://%s:%hu/\n\n", hostBuffer, htons(self->address.sin_port));

    unsigned int sz = sizeof(struct sockaddr_in);
    for (;;)
    {
      if ((self->new_socket = accept(self->socket, (struct sockaddr *)&(self->address), &sz)) < 0)
      {
        printf("error: %s\n", gai_strerror(self->new_socket));
      }

      ssize_t valread = read(self->new_socket, hostBuffer, sizeof(char[1024]));

      self->_cb(hostBuffer, self);
      close(self->new_socket);
    }

    close(self->socket);

    return NULL;
  }

  // void *WS::_listen_thread_f(void *arg)
  // {
  //   WS *self = static_cast<WS*>(arg);
  //   puts("WS *self = static_cast<WS *>(arg);");
  //   self->_listen();
  //   puts("self->_listen();");

  //   return NULL;
  // }

  WS::WS() {}
  WS::~WS() {}

  void WS::init(long port)
  {
    extern int socket(int, int, int);
    extern int setsockopt(int, int, int, const void *, socklen_t);
    struct sockaddr_in serverAddress;
    socklen_t addrlen = sizeof(serverAddress);
    this->address.sin_family = AF_INET;
    this->address.sin_port = htons(port);
    this->address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    this->socket = socket(AF_INET, SOCK_STREAM, 0);

    int v = 1;

    setsockopt(this->socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &v, sizeof(int));

    if (bind(this->socket, (struct sockaddr *)&(this->address), sizeof(sockaddr_in)) < 0)
    {
      puts("error: server not bound to address.");
      return;
    }
    puts("Socket initialised");
  }

  void WS::init(char *url)
  {

  }

  void WS::listen(void (*cb)(char *buf, WS *ws))
  {
    this->_cb = cb;
    extern int listen(int, int);

    // ThreadArgs *args = new ThreadArgs({this, this->socket});

    int result = pthread_create(&(this->_listen_thread), NULL, WS::_listen, this);

    if(result) {
      printf("Error creating thread: %d\n", result);
    }
  }

  void WS::await()
  {
    int result = pthread_join(this->_listen_thread, NULL);
    if(result)
    {
      printf("thread failed to join: %d\n", result);
    }
  }

  void WS::send(char *msg)
  {
    extern ssize_t send(int, const void *, size_t, int);

    int sendRes;
    if ((sendRes = send(this->new_socket, (const void *)msg, strlen(msg), 0)) < 0)
    {
      printf("send error: %s\n", gai_strerror(sendRes));
    }
  }
}