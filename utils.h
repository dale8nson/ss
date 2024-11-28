#pragma once
#ifndef __UTILS__
#define __UTILS__
#endif
#include <stddef.h>
#include <sys/socket.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <unistd.h>
#include <functional>

  namespace Utils
  {
    class action
    {
    private:
      std::function<void(void)> _fn;
      
    public:
      
      action(std::function<void(void)> fn) : _fn{fn} {}
      ~action() { }
      inline void operator()() { _fn(); }
    };
    static const char *digits = "0123456789";
    extern "C" char *reMatch(const char *pattern, const char *subject);
    extern "C" char *parseDNSName(char *buf, size_t *offset);
    extern "C" char *parseIPv4(void *addr);
    extern "C" struct addrinfo *getAddressInfo(const char *domain);
    extern "C" inline void freeAddrInfo(struct addrinfo *info) { freeaddrinfo(info); }
  }

