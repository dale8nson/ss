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

  WS::WS() : dstIPCount{0} {}
  WS::~WS()
  {
    delete this->dst_ipv4s;
    delete this->dst_addrs;
  }

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
    const char *digits = "0123456789";

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
    res->OPCODE = (buf[2] >> 3) & 7;
    res->AA = (buf[2] >> 2) & 1;
    res->TC = (buf[2] >> 1) & 1;
    res->RD = buf[2] & 1;
    res->RA = (buf[3] >> 7) & 1;
    res->RCODE = buf[3] & 7;
    res->QDCOUNT = (buf[4] << 8) | buf[5];
    res->ANCOUNT = (buf[6] << 8) | buf[7];
    res->NSCOUNT = (buf[8] << 8) | buf[9];
    res->ARCOUNT = (buf[10] << 8) | buf[11];

    puts("----HEADER----");
    printf("id: %hu\n", res->id);
    printf("QR: %u\n", res->QR);
    printf("op: %u\n", res->OPCODE);
    printf("AA: %u\n", res->AA);
    printf("TC: %u\n", res->TC);
    printf("RD: %u\n", res->RD);
    printf("RA: %u\n", res->RA);
    printf("RCODE: %u\n", res->RCODE);
    printf("QDCOUNT: %hu\n", res->QDCOUNT);
    printf("ANCOUNT: %hu\n", res->ANCOUNT);
    printf("NSCOUNT: %hu\n", res->NSCOUNT);
    printf("ARCOUNT: %hu\n", res->ARCOUNT);

    puts("\n\n----QUESTION----");
    size_t i = 12;
    uint8_t len = buf[i];
    res->NAME = Utils::parseDNSName(buf, &i);
    printf("NAME: %s\n", res->NAME);

    res->QTYPE = (buf[i++] << 8) | buf[i++];
    res->QCLASS = (buf[i++] << 8) | buf[i++];
    printf("QTYPE: %hu\n", res->QTYPE);
    printf("QCLASS: %hu\n", res->QCLASS);

    for (size_t AN = 0; AN < res->ANCOUNT; AN++)
    {
      printf("\n\n----ANSWER %zu----\n", AN + 1);
      DNSAnswer *an = new DNSAnswer();
      an->NAME = Utils::parseDNSName(buf, &i);
      printf("NAME: %s\n", res->NAME);

      an->TYPE = (buf[i++] << 8) | buf[i++];
      an->CLASS = (buf[i++] << 8) | buf[i++];

      printf("TYPE: %hu\n", an->TYPE);
      printf("CLASS: %hu\n", an->CLASS);

      an->TTL = (buf[i++] << 24) | (buf[i++] << 16) | (buf[i++] << 8) | buf[i++];
      an->RDLENGTH = (buf[i++] << 8) | buf[i++];
      printf("TTL: %u\n", an->TTL);
      printf("RDLENGTH: %hu\n", an->RDLENGTH);

      char *RDATA_str;
      uint16_t k = 0;

      switch (an->TYPE)
      {
      case 1:
      {
        an->RDATA = (uint8_t *)calloc(an->RDLENGTH, sizeof(uint8_t));

        for (uint16_t j = 0; j < an->RDLENGTH; j++, i++)
          an->RDATA[j] = buf[i];

        RDATA_str = (char *)calloc(an->RDLENGTH * 4, sizeof(char));

        for (size_t j = 0; j < an->RDLENGTH; j++)
        {
          uint8_t datum = an->RDATA[j];

          switch (datum / 100)
          {
          case 0:
            switch (datum / 10)
            {
            case 0:
              RDATA_str[k++] = digits[datum];
              break;
            default:
              RDATA_str[k++] = digits[datum / 10];
              RDATA_str[k++] = digits[datum % 10];
              break;
            }
            break;
          default:
            RDATA_str[k++] = digits[datum / 100];
            RDATA_str[k++] = digits[(datum % 100) / 10];
            RDATA_str[k++] = digits[datum % 10];
            break;
          }

          if (j < an->RDLENGTH - 1)
            RDATA_str[k++] = '.';
        }
        RDATA_str[k] = '\0';
        this->dstIPCount++;
        this->dst_ipv4s = (char**)realloc(this->dst_ipv4s, sizeof(char*) * this->dstIPCount);
        this->dst_ipv4s[dstIPCount - 1] = (char *) calloc(strlen(RDATA_str), sizeof(char));
        strcat(this->dst_ipv4s[dstIPCount - 1], RDATA_str);
        printf("this->dst_ipv4s[%zu]: %s\n", dstIPCount - 1, this->dst_ipv4s[dstIPCount - 1]);
        this->dst_addrs = (uint32_t **)realloc(this->dst_addrs, sizeof(uint32_t) * this->dstIPCount);
        puts("this->dst_addrs = (uint32_t **)realloc(this->dst_addrs, dstIPCount);");
        this->dst_addrs[this->dstIPCount - 1] = (uint32_t*) malloc(sizeof(uint32_t));
        *(this->dst_addrs[this->dstIPCount - 1]) = (uint32_t) (an->RDATA[0] << 24) | (an->RDATA[1] << 16) | (an->RDATA[2] << 8) | an->RDATA[3];
        
      }

      break;

      case 16:
      {
        an->RDATA = (uint8_t *)calloc(an->RDLENGTH, sizeof(uint8_t));

        RDATA_str = (char *)calloc((size_t)an->RDLENGTH, sizeof(char));
        while (k < an->RDLENGTH)
          RDATA_str[k] = digits[an->RDATA[k++]];
      }
      break;
      default:

        RDATA_str = Utils::parseDNSName(buf, &i);

        break;
      }

      printf("RDATA: %s\n\n", RDATA_str);
      delete an;
      free(RDATA_str);
    }

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