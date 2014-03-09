#include <pebble.h>

void print_foo(DictionaryIterator *iterator, AppMessageResult reason, void *context){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "reason: %d", reason);
}

void send_cmd(int k, int v){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "trying to send %d, %d", k, v);
    Tuplet value = TupletInteger(k, v);
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    if(iter == NULL){
        APP_LOG(APP_LOG_LEVEL_DEBUG, "iter is null");
        return;
    }

    dict_write_tuplet(iter, &value);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "about to send %d, %d", k, v);
    AppMessageResult r;
    r = app_message_outbox_send();
    APP_LOG(APP_LOG_LEVEL_DEBUG, "result: %d", r);
}

void send_str(int k, const char *v){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "trying to send %d, %s", k, v);
    Tuplet value = TupletCString(k, v);
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    if(iter == NULL){
        APP_LOG(APP_LOG_LEVEL_DEBUG, "iter is null");
        return;
    }

    dict_write_tuplet(iter, &value);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "about to send %d, %s", k, v);
    AppMessageResult r;
    r = app_message_outbox_send();
    APP_LOG(APP_LOG_LEVEL_DEBUG, "result: %d", r);
}
