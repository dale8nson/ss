#include <netinet/in.h>
#include <pthread.h>

extern "C"
{
  typedef struct {
    char *NAME;
    uint16_t TYPE;
    uint16_t CLASS;
    uint32_t TTL;
    uint16_t RDLENGTH;
    uint8_t *RDATA;
  } DNSAnswer;

  typedef struct
  {
    uint16_t id;
    unsigned int QR;
    unsigned int OPCODE;
    unsigned int AA;
    unsigned int TC;
    unsigned int RD;
    unsigned int RA;
    unsigned int Z;
    unsigned int RCODE;
    uint16_t QDCOUNT;
    uint16_t ANCOUNT;
    uint16_t NSCOUNT;
    uint16_t ARCOUNT;
    char *NAME;
    uint16_t QTYPE;
    uint16_t QCLASS;
    uint16_t TYPE;
    uint16_t CLASS;
    uint32_t TTL;
    uint16_t RDLENGTH;
    uint8_t *RDATA;
    DNSAnswer *ANSWERS;

  } DNSMessage;

  

  class WS;
  typedef void (*CB)(char *buf, WS *ws);

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
  };
}