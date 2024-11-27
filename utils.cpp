#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include <stdio.h>
#include <string.h>
#ifndef __UTILS__
#include "utils.h"
#endif

extern "C"
{

  char *Utils::reMatch(const char *pattern, const char *subject)

  {

    int errorcode;
    PCRE2_SIZE erroroffset;

    pcre2_code *code = pcre2_compile((PCRE2_SPTR)pattern, PCRE2_ZERO_TERMINATED, 0, &errorcode, &erroroffset, NULL);
    if (code == NULL)
    {
      puts("compile error");
    }

    pcre2_match_data *data = pcre2_match_data_create_from_pattern(code, NULL);

    int result = pcre2_match(code, (PCRE2_SPTR)subject, strlen(subject), 0, 0, data, NULL);

    if(!result) return NULL;

    PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(data);

    int match_length = ovector[1] - ovector[0];

    char *match = (char *)calloc(match_length, sizeof(char));

    int i;
    for (i = 0; i < match_length; i++)
    {
      match[i] = subject[ovector[0] + i];
    }
    match[i] = '\0';

    return match;
  }

  char *Utils::parseDNSName(char *buf, size_t *offset)
  {
    char *name = (char *)calloc(128, sizeof(char));
    uint8_t len = buf[*offset];
    uint8_t k = 0;
    while (buf[(*offset)++] != '\0')
    {
      // printf("len: %u\n", len);
      char label[len];
      uint8_t j;

      for (j = 0; j < len; j++)
        name[k++] = buf[*offset + j];
      *offset += len;
      strcat(name, label);
      if (buf[*offset] != '\0')
        name[k++] = '.';
      len = buf[*offset];
    }
    name[k] = '\0';
    return name;
  }

  char *Utils::parseIPv4(void *addr)
  {
    char *IPv4 = (char *)calloc(16, sizeof(char));
    size_t k = 0;

    uint8_t *num = (uint8_t *)addr;

    for (size_t i = 0; i < 4; i++)
    {
      uint8_t datum = num[i];
      switch (datum / 100)
      {
      case 0:
        switch (datum / 10)
        {
        case 0:
          IPv4[k++] = digits[datum];
          break;
        default:
          IPv4[k++] = digits[datum / 10];
          IPv4[k++] = digits[datum % 10];
          break;
        }
        break;
      default:
        IPv4[k++] = digits[datum / 100];
        IPv4[k++] = digits[(datum % 100) / 10];
        IPv4[k++] = digits[datum % 10];
        break;
      }

      if (i < 3)
        IPv4[k++] = '.';
    }
    IPv4[k] = '\0';

    return IPv4;
  }
}
