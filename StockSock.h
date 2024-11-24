#include <cjson/cJSON.h>
#include "WS.h"

extern "C"
{

  typedef struct
  {
    char *response;
    size_t sz;
  } memory;

  class StockSock
  {
  private:
    CURL *_curl;
    int _socket;
    long _port;
    char *_url;
    cJSON _json;
    static void *_listen(void* args);
    
  public:
    StockSock(char *url);
    ~StockSock();
    void connect();
    cJSON get() { return this->_json; }
  };
}