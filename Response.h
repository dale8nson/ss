

class Response
{
private:
  char _buf[4096];
  char _txt[4096];

public:
  Response();
  ~Response();
  static Response& json(const char* str);
};