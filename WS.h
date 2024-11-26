#include <netinet/in.h>
#include <pthread.h>

extern "C"
{

  class WS
  {
  private:
    int socket;
    int new_socket;
    long port;
    size_t dstIPCount;
    char **dst_ipv4s;
    uint32_t **dst_addrs;
    struct sockaddr_in address;
    pthread_t _listen_thread;
    static void *_listen(void *arg);
    void (*_cb)(char *buf, WS *ws);
    char *domain;

  public:
    WS();
    ~WS();
    void init();
    void init(const char *url);
    void listen(void (*cb)(char *buf, WS *ws));
    void await();
    void send(char *msg);
    void setPort(long port) { this->port = port; }
    long getPort(void) { return this->port; }
    struct sockaddr_in getAddress() { return this->address; }
  };
}