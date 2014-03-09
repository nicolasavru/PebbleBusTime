#ifndef UTILS_H
#define UTILS_H

enum BusKey{
  LINENAME_KEY = 0x0,         // TUPLE_CSTRING
  STOPNAME_KEY = 0x1,         // TUPLE_CSTRING
  NUMBUSES_KEY = 0x2,         // TUPLE_INT
  BUS_KEY = 0x3,              // TUPLE_CSTRING
  ROUTES_KEY = 0x4            // TUPLE_CSTRING
};

void print_foo(DictionaryIterator *iterator, AppMessageResult reason, void *context);
void send_cmd(int k, int v);
void send_str(int k, const char *v);

#endif /* UTILS_H */
