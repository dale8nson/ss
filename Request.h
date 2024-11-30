class Request
{
private:
  const char *_get = "GET /%s HTTP/1.1\n";
  const char *_connect = "CONNECT HTTP/1.1\n";

public:
  Request();
  ~Request();
};