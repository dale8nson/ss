#include <cstdint>
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

  class DNSMessage
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

    char *_buf;
    DNSAnswer *_answers;

  public:
    DNSMessage(char *buf);
    ~DNSMessage();
  };
}