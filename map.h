#include <ctype.h>
#include <stdlib.h>

template <typename K, typename V>
class map
{
private:
  K *_keys;
  V *_values;
  size_t _len;
  size_t _sz;

public:
  map(K k0, ...);
  ~map();
  size_t len() { return _len; }
  inline V &operator[](K &key)
  {
    size_t i;
    for (i = 0; i < _len; i++)
      if (_keys[i] == key)
        break;

    if (i == _sz)
      return nullptr;
    return _values[i];
  }
};
