#include <cstdint>
#include "WS.h"
extern "C"
{
  typedef struct
  {
    char *NAME;
    uint16_t TYPE;
    uint16_t CLASS;
    uint32_t TTL;
    uint16_t RDLENGTH;
    uint8_t *RDATA;
  } DNSAnswer;

  class DNSQuery
  {
  private:
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
    size_t dstIPCount;
    char **dst_ipv4s;
    uint32_t **dst_addrs;

    char *_buf;
    DNSAnswer *_answers;

  public:
    DNSQuery(const char *url);
    ~DNSQuery();
    char **destinationIPs() { return dst_ipv4s; }
    uint32_t **destinationAddresses() { return dst_addrs; }
  };
}