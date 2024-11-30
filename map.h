#ifndef __MAP__
#define __MAP__

#include <ctype.h>
#include <stdlib.h>
#include <functional>
#include <stdarg.h>

template <typename K, typename V>
class map
{
private:
  K *_keys;
  V *_values;
  size_t _len = 0;
  size_t _sz = 0;
  inline K &_begin() { return _keys; }
  inline K &_end() { return _keys[_sz - 1]; }

public:
  map() : _keys{nullptr}, _values{nullptr}, _sz{512}
  {
    _keys = (K *)malloc(sizeof(K) * _sz);
    _values = (V *)malloc(sizeof(V) * _sz);
  }
  map(K k0, ...);
  ~map(void);
  inline size_t len() { return _len; }
  V &operator[](const K &key);

  inline void foreach (std::function<void(const K &, V &)> fn)
  {
    for (size_t i = 0; i < _len; i++)
      fn(_keys[i], _values[i]);
  }
};

template <typename K, typename V>
map<K, V>::map(K k0, ...) : _keys{(K *)malloc(512 * sizeof(K))},
                            _values{(V *)malloc(512 * sizeof(V))},
                            _sz{512}
{
  va_list ap;
  va_start(ap, k0);

  for (K k = va_arg(ap, K); k; k = (K)va_arg(ap, K))
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
  free(_keys);
  free(_values);
}

template <typename K, typename V>
V &map<K, V>::operator[](const K &key)
{

  size_t i;
  for (i = 0; i < _len && _keys[i] != key; i++)
    ;

  if (i < _len)
  {
    puts("i < _len");
    return _values[i];
  }

  if (_len + 1 > _sz)
  {
    puts("_len + 1 > _sz");
    _keys = (K *)realloc(_keys, _sz + 512);
    _values = (V *)realloc(_values, _sz + 512);
    _sz += 512;
    _keys[_len] = key;
    _values[_len] = *(V*)malloc(sizeof(V));
    puts("_values[_len] = V();");
    return _values[_len++];
  }

  _keys[_len] = key;
  _values[_len] = *(V*)malloc(sizeof(V));
  puts("_values[_len] = V();");
  return _values[_len++];
}

#endif
