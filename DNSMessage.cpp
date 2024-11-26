#include <cstdint>
#include <sys/socket.h>
#include <dns.h>
#include "DNSMessage.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C"
{

  DNSQuery::DNSQuery(const char *url)
  {
    const char *digits = "0123456789";
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(53);

    char *protocol = Utils::reMatch(R"(^\w+?(?=://))", url);
    printf("protocol: %s\n", protocol);
    char *domain = Utils::reMatch(R"((?<=://)(([.]?([\d\w])+?)+)(?=[?/]|$))", url);
    printf("domain: %s\n", domain);
    dns_handle_t dns_handle = dns_open(NULL);
    char buf[2048];
    uint32_t fromLen;
    fromLen = 2048;
    int32_t ip = dns_query(dns_handle, domain, 1, 1, buf, 2048, (struct sockaddr *)&address, &fromLen);

    id = (buf[0] << 8) | buf[1];
    QR = buf[2] >> 7;
    OPCODE = (buf[2] >> 3) & 7;
    AA = (buf[2] >> 2) & 1;
    TC = (buf[2] >> 1) & 1;
    RD = buf[2] & 1;
    RA = (buf[3] >> 7) & 1;
    RCODE = buf[3] & 7;
    QDCOUNT = (buf[4] << 8) | buf[5];
    ANCOUNT = (buf[6] << 8) | buf[7];
    NSCOUNT = (buf[8] << 8) | buf[9];
    ARCOUNT = (buf[10] << 8) | buf[11];

    puts("----HEADER----");
    printf("id: %hu\n", id);
    printf("QR: %u\n", QR);
    printf("OP: %u\n", OPCODE);
    printf("AA: %u\n", AA);
    printf("TC: %u\n", TC);
    printf("RD: %u\n", RD);
    printf("RA: %u\n", RA);
    printf("RCODE: %u\n", RCODE);
    printf("QDCOUNT: %hu\n", QDCOUNT);
    printf("ANCOUNT: %hu\n", ANCOUNT);
    printf("NSCOUNT: %hu\n", NSCOUNT);
    printf("ARCOUNT: %hu\n", ARCOUNT);

    puts("\n\n----QUESTION----");
    size_t i = 12;
    uint8_t len = buf[i];
    NAME = Utils::parseDNSName(buf, &i);
    printf("NAME: %s\n", NAME);

    QTYPE = (buf[i++] << 8) | buf[i++];
    QCLASS = (buf[i++] << 8) | buf[i++];
    printf("QTYPE: %hu\n", QTYPE);
    printf("QCLASS: %hu\n", QCLASS);

    for (size_t AN = 0; AN < ANCOUNT; AN++)
    {
      printf("\n\n----ANSWER %zu----\n", AN + 1);
      DNSAnswer *an = new DNSAnswer();
      an->NAME = Utils::parseDNSName(buf, &i);
      printf("NAME: %s\n", NAME);

      an->TYPE = (buf[i++] << 8) | buf[i++];
      an->CLASS = (buf[i++] << 8) | buf[i++];

      printf("TYPE: %hu\n", an->TYPE);
      printf("CLASS: %hu\n", an->CLASS);

      an->TTL = (buf[i++] << 24) | (buf[i++] << 16) | (buf[i++] << 8) | buf[i++];
      an->RDLENGTH = (buf[i++] << 8) | buf[i++];
      printf("TTL: %u\n", an->TTL);
      printf("RDLENGTH: %hu\n", an->RDLENGTH);

      char *RDATA_str;
      uint16_t k = 0;

      switch (an->TYPE)
      {
      case 1:
      {
        an->RDATA = (uint8_t *)calloc(an->RDLENGTH, sizeof(uint8_t));

        for (uint16_t j = 0; j < an->RDLENGTH; j++, i++)
          an->RDATA[j] = buf[i];

        RDATA_str = (char *)calloc(an->RDLENGTH * 4, sizeof(char));

        for (size_t j = 0; j < an->RDLENGTH; j++)
        {
          uint8_t datum = an->RDATA[j];

          switch (datum / 100)
          {
          case 0:
            switch (datum / 10)
            {
            case 0:
              RDATA_str[k++] = digits[datum];
              break;
            default:
              RDATA_str[k++] = digits[datum / 10];
              RDATA_str[k++] = digits[datum % 10];
              break;
            }
            break;
          default:
            RDATA_str[k++] = digits[datum / 100];
            RDATA_str[k++] = digits[(datum % 100) / 10];
            RDATA_str[k++] = digits[datum % 10];
            break;
          }

          if (j < an->RDLENGTH - 1)
            RDATA_str[k++] = '.';
        }
        RDATA_str[k] = '\0';
        dstIPCount++;
        dst_ipv4s = (char**)realloc(dst_ipv4s, sizeof(char*) * dstIPCount);
        dst_ipv4s[dstIPCount - 1] = (char *) calloc(strlen(RDATA_str), sizeof(char));
        strcat(dst_ipv4s[dstIPCount - 1], RDATA_str);
        dst_addrs = (uint32_t **)realloc(dst_addrs, sizeof(uint32_t) * dstIPCount);
        dst_addrs[dstIPCount - 1] = (uint32_t*) malloc(sizeof(uint32_t));
        *(dst_addrs[dstIPCount - 1]) = (uint32_t) (an->RDATA[0] << 24) | (an->RDATA[1] << 16) | (an->RDATA[2] << 8) | an->RDATA[3];
      }

      break;

      case 16:
      {
        an->RDATA = (uint8_t *)calloc(an->RDLENGTH, sizeof(uint8_t));

        RDATA_str = (char *)calloc((size_t)an->RDLENGTH, sizeof(char));
        while (k < an->RDLENGTH)
          RDATA_str[k] = digits[an->RDATA[k++]];
      }
      break;
      default:

        RDATA_str = Utils::parseDNSName(buf, &i);

        break;
      }

      printf("RDATA: %s\n\n", RDATA_str);
      delete an;
      free(RDATA_str);
    }
  }

  DNSQuery::~DNSQuery()
  {
    free(NAME);
    free(RDATA);
    free(dst_addrs);
    free(dst_ipv4s);
  }
}