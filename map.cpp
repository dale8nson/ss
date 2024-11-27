
#include "map.h"
#include <stdarg.h>

template <typename K, typename V>
map<K, V>::map(K k0, ...) : _keys{calloc(512, sizeof(K))},
                            _values{calloc(512, sizeof(V))},
                            _sz{512}
{
  va_list ap;
  va_start(ap, k0);

  for (K k = va_arg(ap, K); k; k = (K) va_arg(ap, K))
  {
    if (_sz < _len + 1)
    {
      _keys = (K *)realloc(_keys, _sz + 512);
      _values = (V *)realloc(_values, _sz + 512);
      _sz += 512;
    }
    V v = va_arg(ap, V);
    size_t i;
    for (i = 0; i < _len - 1 && _keys[i] != k; i++)
      ;
    if (i < _len - 1)
    {
      _values[i] = v;
      continue;
    }

    _keys[++_len] = k;
    _values[_len] = v;
  }
  va_end(ap);
}

template <typename K, typename V>
map<K, V>::~map()
{
  delete[] _keys;
  delete[] _values;
}
