#include <netinet/in.h>

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

  public:
    WS();
    ~WS();
    void init(long port);
    void listen(void (*cb)(char *buf, WS *ws));
    void send(char *http);
    void setPort(long port) { this->port = port; }
    long getPort(void) { return this->port; }
  };
}