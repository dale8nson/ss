#define PCRE2_CODE_UNIT_WIDTH 8
#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <time.h>
#include <pcre2.h>
#include "WS.h"
#include "Template.h"

typedef void (*CB)(char *, WS *ws);

extern "C"
{
  char *reMatch(const char *, char *);
  void cb(char *buf, WS *ws);
  class WS;

  void cb(char *buf, WS *ws)
  {
    extern void free(void *);

    const char *pattern = "(?<=/)\\w+?(?=\\s)";

    char *match = reMatch(pattern, buf);

    printf("ovector: %s\n", match);

    const char *fheader = "HTTP/1.1 200 OK\nAccess-Control-Allow-Origin: *\nContent-Type:text/html\nContent-Length:%zu\n\n";

    Template *tmp = new Template("template.html", match);
    printf("\n\ntmp->text():\n\n%s\n\n", tmp->text());

    char *header = (char *)calloc(1024, sizeof(char));
    snprintf(header, 1024, fheader, strlen(tmp->text()));
    char *response = (char *)calloc(2048, sizeof(char));

    strcat(response, header);
    strcat(response, tmp->text());

    printf("%s\n", response);

    ws->send(response);

    free(response);
    free(match);
  }

  int main(int argc, char *argv[])
  {
    WS *ws = new WS();
    ws->init(3170);
    ws->listen(cb);

    return 0;
  }

  char *reMatch(const char *pattern, char *subject)
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
}
