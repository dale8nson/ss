#include <sys/socket.h>
#include <stdio.h>
#include "WS.h"
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <dns.h>
#include "utils.h"

extern "C"
{
  void *WS::_listen(void *args)
  {
    WS *self = (WS *)(args);
    extern int listen(int, int);

    if (!self->port)
    {
      perror("Socket error: port not set.");
      pthread_exit(nullptr);
    }

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

  WS::WS() {}
  WS::~WS() {}

  void WS::init()
  {
    if (!this->port)
    {
      perror("Socket error: port not set.");
      pthread_exit(nullptr);
    }
    extern int socket(int, int, int);
    extern int setsockopt(int, int, int, const void *, socklen_t);
    struct sockaddr_in serverAddress;
    socklen_t addrlen = sizeof(serverAddress);
    this->address.sin_family = AF_INET;
    this->address.sin_port = htons(this->port);
    this->address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    this->socket = socket(this->address.sin_family, SOCK_STREAM, 0);

    setsockopt(this->socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, (void *)1, sizeof(int));

    if (bind(this->socket, (struct sockaddr *)&(this->address), sizeof(sockaddr_in)) < 0)
    {
      puts("error: server not bound to address.");
      return;
    }
    puts("Socket initialised");
  }

  void WS::init(const char *url)
  {

    this->address.sin_family = AF_INET;
    this->address.sin_port = htons(this->port);

    char *protocol = Utils::reMatch(R"(^\w+?(?=://))", url);
    printf("protocol: %s\n", protocol);
    char *domain = Utils::reMatch(R"((?<=://)(([.]?([\d\w])+?)+)(?=[?/]|$))", url);
    printf("domain: %s\n", domain);

    dns_handle_t dns_handle = dns_open(NULL);
    DNSMessage *res = new DNSMessage();
    char buf[2048];
    uint32_t fromLen;
    fromLen = 2048;
    int32_t ip = dns_query(dns_handle, domain, 1, 1, buf, 2048, (struct sockaddr *)&(this->address), &fromLen);

    res->id = (buf[0] << 8) | buf[1];
    res->QR = buf[2] >> 7;
    res->op = (buf[2] >> 3) & 7;
    res->AA = (buf[2] >> 2) & 1;
    res->TC = (buf[2] >> 1) & 1;
    res->RD = buf[2] & 1;
    res->RA = (buf[3] >> 7) & 1;
    res->RCODE = buf[3] & 7;
    res->QDCOUNT = (buf[4] << 8) | buf[5];
    res->ANCOUNT = (buf[6] << 8) | buf[7];
    res->NSCOUNT = (buf[8] << 8) | buf[9];
    res->ARCOUNT = (buf[10] << 8) | buf[11];

    puts("----HEADER----\n");
    printf("id: %hu\n", res->id);
    printf("QR: %u\n", res->QR);
    printf("op: %u\n", res->op);
    printf("AA: %u\n", res->AA);
    printf("TC: %u\n", res->TC);
    printf("RD: %u\n", res->RD);
    printf("RA: %u\n", res->RA);
    printf("RCODE: %u\n", res->RCODE);
    printf("QDCOUNT: %hu\n", res->QDCOUNT);
    printf("ANCOUNT: %hu\n", res->ANCOUNT);
    printf("NSCOUNT: %hu\n", res->NSCOUNT);
    printf("ARCOUNT: %hu\n", res->ARCOUNT);

    puts("\n\n----QUESTION----\n");
    size_t i = 12;
    uint8_t len = buf[i];
    res->NAME = Utils::parseDNSName(buf, &i);
    printf("NAME: %s\n", res->NAME);

    res->QTYPE = (buf[i++] << 8) | buf[i++];
    res->QCLASS = (buf[i++] << 8) | buf[i++];
    printf("QTYPE: %hu\n", res->QTYPE);
    printf("QCLASS: %hu\n", res->QCLASS);

    puts("\n\n----ANSWER----\n");
    free(res->NAME);
    res->NAME = Utils::parseDNSName(buf, &i);
    printf("NAME: %s\n", res->NAME);

    res->TYPE = (buf[i++] << 8) | buf[i++];
    res->CLASS = (buf[i++] << 8) | buf[i++];

    printf("TYPE: %hu\n", res->TYPE);
    printf("CLASS: %hu\n", res->CLASS);

    res->TTL = (buf[i++] << 24) | (buf[i++] << 16) | (buf[i++] << 8) | buf[i++];
    res->RDLENGTH = (buf[i++] << 8) | buf[i++];
    printf("TTL: %u\n", res->TTL);
    printf("RDLENGTH: %hu\n", res->RDLENGTH);

    res->RDATA = (uint8_t *)calloc(res->RDLENGTH, sizeof(uint8_t));
    for (size_t j = 0; j < res->RDLENGTH; j++) res->RDATA[j] = buf[i++];
    
    char RDATA_str[res->RDLENGTH * 4 - 1];
    for(size_t j = 0; j < res->RDLENGTH; j++)
    {
      char num_str[5];
      snprintf(num_str, 5, "%u", res->RDATA[j]);
      if(j <= res->RDLENGTH - 2) strcat(num_str, ".");
      strcat(RDATA_str, num_str);
    }
    printf("RDATA: %s\n\n", RDATA_str);
    free(res->NAME);
    free(res->RDATA);
    delete res;
  }

  void WS::listen(void (*cb)(char *buf, WS *ws))
  {
    this->_cb = cb;
    extern int listen(int, int);

    int result = pthread_create(&(this->_listen_thread), NULL, WS::_listen, this);

    if (result)
    {
      printf("Error creating thread: %d\n", result);
    }
  }

  void WS::await()
  {
    int result = pthread_join(this->_listen_thread, NULL);
    if (result)
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