#ifndef __HEADERS__
#define __HEADERS__
#include <string.h>
// #ifndef __MAP__
#include "map.h"
// #endif

class Headers
{
private:
  const char *_end = "\r\n\r\n";
  char *_header_str;
  map<const char *, const char *> _headers;
  ;

public:
  Headers() : _headers{map<const char *,const char *>()}, _header_str{nullptr} {}
  inline ~Headers()
  {
    free(_header_str);
    _header_str = NULL;
  }
  const char *&operator[](const char *header);

  inline char *operator=(const char *value)
  {
    char *v = (char *)malloc(strlen(value) + 1 * sizeof(char));
    if(v == NULL) {
      fprintf(stderr, "%s\n", "Memory allocation failed");
      return NULL;
    }
    strcpy(v, value);
    return v;
  }
  inline char *operator=(char *value)
  {
    return value;
  }

  char *operator()();
};

#endif