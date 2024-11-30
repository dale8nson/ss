#include "Headers.h"
#include <string.h>
#ifndef __MAP__
#include "map.h"
#endif

char *Headers::operator()()
{
  if (_header_str != NULL)
  {
    puts("_header_str != NULL");
    free(_header_str);
    _header_str = NULL;
  }
  _header_str = (char *)calloc(512, sizeof(char));
  puts("_header_str = (char *)realloc(_header_str, 512);");
  _headers.foreach ([&](const char *header, const char *value)
                    {
                      printf("%s: %s\n", header, value);
                      char str[strlen(header) + strlen(value) + 5]; 
                      snprintf(str, strlen(header) + strlen(value) + 5, "%s: %s\r\n", header, value);
                      strcat(_header_str, str);
                      printf("_header_str:\n%s\n", _header_str); });
  strcat(_header_str, _end);
  printf("_header_str:\n%s\n", _header_str);
  return _header_str;
}

const char *&Headers::operator[](const char *header)
{
  return _headers[header];
}
