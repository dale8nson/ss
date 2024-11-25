#include <stddef.h>

extern "C"
{
  namespace Utils {
  char *reMatch(const char *pattern, const char *subject);
  char *parseDNSName(char *buf, size_t *offset);
  }
}

