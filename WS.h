#include <netinet/in.h>
#include <pthread.h>

extern "C"
{


  class WS;
  typedef void (*CB)(char *buf, WS *ws);

  class WS
  {
  private:
    int socket;
    int new_socket;
    long port;
    struct sockaddr_in address;
    pthread_t _listen_thread;
    // static void *_listen_thread_f(void *arg);
    static void* _listen(void *arg);
    void (*_cb)(char *buf, WS *ws);

  public:
    WS();
    ~WS();
    void init(long port);
    void init(char *url);
    void listen(void (*cb)(char *buf, WS *ws));
    void await();
    void send(char *msg);
    void setPort(long port) { this->port = port; }
    long getPort(void) { return this->port; }
  };
}