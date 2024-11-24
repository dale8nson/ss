#include "StockSock.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

extern "C"
{

  static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);

  void *StockSock::_listen(void *args)
  {



    return nullptr;
  }

  StockSock::StockSock(char *url)
  {
    this->url = url;
    this->_curl = curl_easy_init();
    memory chunk;

    if (this->_curl)
    {
      CURLcode res;
      curl_easy_setopt(this->_curl, CURLOPT_URL, this->_url);
      curl_easy_setopt(this->_curl, CURLOPT_CA_CACHE_TIMEOUT, 604800L);
      curl_easy_setopt(this->_curl, CURLOPT_WRITEFUNCTION, write_callback);
      curl_easy_setopt(this->_curl, CURLOPT_WRITEDATA, (void *)&chunk);
      res = curl_easy_perform(this->_curl);

      if (res != CURLE_OK)
      {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        
      }

      char *str = (char *)calloc(chunk.sz, sizeof(char));
      str = (chunk.response);

      cJSON *json = cJSON_Parse(str);

      // printf("%s\n", str);

      puts("CURRENT\tCHANGE\t%%\tHIGH\tLOW\tOPEN\tCLOSE");
      printf("%0.3lf\t", json->child->valuedouble);
      cJSON *datum = json->child->next;
      while (*(datum->string) != 't')
      {
        printf("%0.3lf\t", datum->valuedouble);
        datum = datum->next;
      }

      putchar('\n');

      cJSON_Delete(json);

      curl_easy_cleanup(this->_curl);
    }
    else
    {
      printf("%s\n", "failed to initialise curl");
    }

    free(url);
    free(chunk.response);

    curl_global_cleanup();
  }

  size_t write_callback(char *data, size_t size, size_t nmemb, void *userdata)
  {
    size_t sz = size * nmemb;
    memory *mem = (memory *)userdata;
    char *ptr = (char *) realloc(mem->response, mem->sz + sz + 1);
    if (!ptr)
    {
      return 0;
    }
    mem->response = ptr;
    memcpy(&(mem->response[mem->sz]), data, sz);
    mem->sz += sz;
    mem->response[mem->sz] = 0;

    return sz;
  }

  StockSock::~StockSock()
  {

  }

  void StockSock::connect()
  {
  }
}
