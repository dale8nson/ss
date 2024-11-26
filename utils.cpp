#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"

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

    int err = pcre2_match(code, (PCRE2_SPTR)subject, strlen(subject), 0, 0, data, NULL);

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
}
