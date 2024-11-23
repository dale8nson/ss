#include "Template.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

void Template::load()
{
  this->file = fopen(this->filename, "r");
  if(this->file == NULL) {
    printf("Error loading %s\n", this->filename);
    return;
  }

  printf("%s loaded\n", this->filename);

  char buf[512];

  while(buf[fread(buf, sizeof(char), 512, this->file) - 1] != EOF)
  {
    realloc(this->fmt, strlen(this->fmt) + 512);
    strcat(this->fmt, buf);
  }

  printf("loaded template:\n%s\n", this->fmt);

}

char *Template::interpolate(...)
{
  return nullptr;
}