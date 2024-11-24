#include "Template.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

Template::Template(const char *fn, ...) : filename{fn}, fmt{new char[0]}, txt{new char[0]}
{

  puts("this->load();");
  this->load();

  char buf[512];

  va_list ap;
  va_start(ap, fn);
  // puts("va_start(ap, fn);");
  char *c = (char *)this->fmt;
  // puts("char *c = (char *)this->fmt;");
  // printf("initial c: %c\n", *c);
  size_t i = 0;
  while (*c != '\0')
  {

    if (*c++ == '%')
    {
      // puts("*c++ == '%'\n");
      // printf("i = %zu\t *c = %c\n", i, *c);
      switch (*c)
      {
      case '%':
      puts("case '%':");
        buf[i++] = '%';
        c++;
        break;
      case 'z':
        switch (++*c)
        {
        case 'u':
          size_t num = va_arg(ap, size_t);
          char str[10];
          snprintf(str, 10, "%zu", num);
          size_t str_len = strlen(str);
          size_t buf_rem = 510 - i;
          if (buf_rem >= str_len)
          {
            strcat(buf, str);
            i += str_len;
          }
          else
          {
            char substr[buf_rem + 1];
            strncpy(substr, str, buf_rem);
            strcat(buf, substr);
            for (i = 0; i < str_len - buf_rem; i++, c++)
            {
              buf[i++] = *c;
            }
          }
        }
        break;
      case 's':
      {
        char *str = va_arg(ap, char *);
        printf("str: %s\n", str);
        size_t str_len = strlen(str);
        size_t buf_rem = 510 - i;
        if (buf_rem >= str_len)
        {
          buf[i - 1] ='\0';
          strcat(buf, str);
          i += str_len - 1;
          
        }
        else
        {
          char substr[buf_rem + 1];
          strncpy(substr, str, buf_rem);
          strcat(buf, substr);
          for (i = 0; i < str_len - buf_rem; i++, c++)
          {
            buf[i++] = *c;
          }
        }
      }

        break;
      default:
        buf[i] = '%';
        buf[i++] = *c;
        break;
      }
    }
    else
    {
      buf[i++] = *c;
    }
    if (i == 512 || *c == '\0')
    {
      this->txt = (char *)realloc(this->txt, strlen(this->txt) + strlen(buf));
      strcat(this->txt, buf);
      i = 0;
    }
    if (*c == '\0')
      break;
  }
  printf("Template::text():\n%s", this->text());
}

Template::~Template()
{
  delete[] this->fmt;
  delete[] this->txt;
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

  char *buf = (char *)calloc(512, sizeof(char));
  int i = 0, c = -2;

  while ((char)c != '\0' && c != EOF)
  {
    c = getc(this->file);

    buf[i++] = (char)c;

    if (i == 511 || (char)c == '\0' || c == EOF)
    {
      if (c == EOF)
      {
        buf[i - 1] = '\0';
      }

      this->fmt = (char *)realloc(this->fmt, sizeof(char[strlen(this->fmt) + i]));
      strcat(this->fmt, buf);

      buf = (char *)realloc(buf, 512);
      i = 0;
    }
  }
  putchar('\n');

  // printf("\n\nloaded template:\n\n%s\n", this->fmt);
  fclose(this->file);
  free(buf);
}
