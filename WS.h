#pragma once
#ifndef __WS__
#define __WS__
#endif
#include <netinet/in.h>
#include <pthread.h>
#include "utils.h"

extern "C"
{

  class WS
  {
  private:
    int _socket;
    int _new_socket;
    char *_protocol;
    char *_host;
    long _port;
    char *_query;
    const char *_dst_url;
    size_t _dstIPCount;
    char **_dst_ipv4s;
    uint32_t **_dst_addrs;
    struct sockaddr_in _address;
    pthread_t *_threads[64];
    size_t _thread_count;
    void _await_all();
    pthread_t _listen_thread;
    struct sockaddr_in _dst_addr;
    static void *_listen(void *arg);
    static void *_send_async(void *arg);
    void (*_listen_cb)(char *buf, WS *ws);
    void (*_connect_cb)(char *buf, WS *ws);

  public:
    WS();
    ~WS();
    void init();
    void init(const char *url);
    void connect();
    void listen(void (*cb)(char *buf, WS *ws));
    
    void send(char *msg);
    void sendAsync(char *buf);
    void setPort(long port) { _port = htons(port);}
    long port(void) { return ntohs(_port); }
    void setDestinationIPs(char**ips) { _dst_ipv4s = ips;}
    void setDestinationAddresses(uint32_t**addrs) { _dst_addrs = addrs; }
    struct sockaddr_in address() { return _address; }
  };
}