#include <stdio.h>

extern "C" {
  class Template 
  {
    private:
    const char *filename;
    FILE *file;
    char *fmt;

    public:
    Template(const char *fn): filename{fn}, fmt {new char[0]} {}
    ~Template();
    void load();
    char *interpolate(...);
  };
}