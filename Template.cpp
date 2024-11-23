#include "Template.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

Template::~Template()
{
  delete[] this->fmt;
}

void Template::load()
{
  this->file = fopen(this->filename, "r");
  if (this->file == NULL)
  {
    printf("Error loading %s\n", this->filename);
    return;
  }

  printf("%s loaded\n", this->filename);

  char *buf = (char *) calloc(512, sizeof(char));
  int i = 0, c = -2;
  char *cur = buf;
  while ((char) c != '\0' && c != EOF)
  {
    c = getc(this->file);

    buf[i++] = (char) c;

    if (i == 511 ||(char) c == '\0' || c == EOF)
    {
      if(c == EOF) {
        buf[i - 1] = '\0';
      }

      this->fmt = (char *)realloc(this->fmt, sizeof(char[strlen(this->fmt) + i]));
      strcat(this->fmt, buf);
      
      buf = (char *) realloc(buf, 512);
      i = 0;
    }
  }
  putchar('\n');

  printf("\n\nloaded template:\n\n%s\n", this->fmt);
  fclose(this->file);
  free(buf);
}

char *Template::interpolate(...)
{
  return nullptr;
}