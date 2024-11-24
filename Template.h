#include <stdio.h>

extern "C" {
  class Template 
  {
    private:
    const char *filename;
    FILE *file;
    void load();
    char *fmt;
    void **args;
    char *txt;

    public:
    Template(const char *fn, ...);
    ~Template();
    char *text(){ return this->txt; };
  };
}