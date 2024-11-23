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

extern "C" char *reMatch(const char *, char *);
extern "C" void cb(char *buf, WS *ws);
extern "C" class WS;
extern "C" int main(int argc, char *argv[]);

void cb(char *buf, WS *ws)
{
  extern void free(void*);

  Template *tmp = new Template("template.html");
  tmp->load();
  
  const char *pattern = "(?<=/)\\w+?(?=\\s)";

  char *match = reMatch(pattern, buf);

  printf("ovector: %s\n", match);

  const char *fheader = "HTTP/1.1 200 OK\nAccess-Control-Allow-Origin: *\nContent-Type:text/html\nContent-Length:%zu\n\n";

  const char *fhtml =
      "\
  <!DOCTYPE html>\
  <html>\
    <body style=\"display:flex;justify-content:center;align-content:center;background-color:#000;width:100vw;height:100vh\">\
      <div style=\"display:flex;justify-content:center;align-content:center;border-style:solid;border-width:2px;border-color:#e00;border-radius:30px;width:33.33333%%; box-shadow:10px 10px 5px #e007;height:auto;margin:auto;padding: 5px;\">\
          <h1 style=\"color:#e00;font-family:sans-serif;\">%s</h1>\
      </div>\
    </body>\
  </html>\n";

  char fresponse[strlen(fheader) + strlen(fhtml)];
  strcat(fresponse, fheader);
  strcat(fresponse, fhtml);

  char html[1024];
  snprintf(html, 1024, fhtml, match);

  char *response = (char *)calloc(2048, sizeof(char));

  snprintf(response, 2048, fresponse, strlen(html), match);

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
