#include <sys/socket.h>
#include <stdio.h>
#include "WS.h"
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <_stdlib.h>

extern "C"
{

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
  }

  void WS::listen(void (*cb)(char *buf, WS *ws))
  {
    extern int listen(int, int);

    if (listen(this->socket, 10) < 0)
    {
      puts("error: server not listening");
      exit(1);
    }

    char hostBuffer[1024], serviceBuffer[NI_MAXSERV];
    int error = getnameinfo(
        (struct sockaddr *)&(this->address), sizeof(struct sockaddr),
        hostBuffer, sizeof(hostBuffer),
        serviceBuffer, sizeof(serviceBuffer),
        0);
    if (error != 0)
    {
      printf("error: %s\n", gai_strerror(error));
      return;
    }

    printf("Server is listening on http://%s:%hu/\n\n", hostBuffer, htons(this->address.sin_port));

    unsigned int sz = sizeof(struct sockaddr_in);
    for (;;)
    {
      if ((this->new_socket = accept(this->socket, (struct sockaddr *)&(this->address), &sz)) < 0)
      {
        printf("error: %s\n", gai_strerror(new_socket));
      }

      ssize_t valread = read(this->new_socket, hostBuffer, sizeof(char[1024]));

      cb(hostBuffer, this);
      close(this->new_socket);
    }
    
    close(this->socket);
  }

  void WS::send(char *http)
  {
    extern ssize_t send(int, const void *, size_t, int);

    int sendRes;
    if ((sendRes = send(this->new_socket,(const void*) http, strlen(http), 0)) < 0)
    {
      printf("send error: %s\n", gai_strerror(sendRes));
    }
  }
}