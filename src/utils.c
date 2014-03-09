#include <pebble.h>

void send_cmd(int i){
    Tuplet value = TupletInteger(0, i);
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    if (iter == NULL) {
        return;
    }

    dict_write_tuplet(iter, &value);
    app_message_outbox_send();
}
