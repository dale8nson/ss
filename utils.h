#pragma once
#include <stddef.h>
#ifndef __UTILS__
#define __UTILS__
#endif

extern "C"
{
  namespace Utils
  {
    static const char *digits = "0123456789";
    char *reMatch(const char *pattern, const char *subject);
    char *parseDNSName(char *buf, size_t *offset);
    char *parseIPv4(void *addr);
  }
}
