#include <pebble.h>

// http://stackoverflow.com/questions/21150193/logging-enums-on-the-pebble-watch/21172222#21172222
char *translate_error(AppMessageResult result){
    switch (result) {
    case APP_MSG_OK: return "APP_MSG_OK";
    case APP_MSG_SEND_TIMEOUT: return "APP_MSG_SEND_TIMEOUT";
    case APP_MSG_SEND_REJECTED: return "APP_MSG_SEND_REJECTED";
    case APP_MSG_NOT_CONNECTED: return "APP_MSG_NOT_CONNECTED";
    case APP_MSG_APP_NOT_RUNNING: return "APP_MSG_APP_NOT_RUNNING";
    case APP_MSG_INVALID_ARGS: return "APP_MSG_INVALID_ARGS";
    case APP_MSG_BUSY: return "APP_MSG_BUSY";
    case APP_MSG_BUFFER_OVERFLOW: return "APP_MSG_BUFFER_OVERFLOW";
    case APP_MSG_ALREADY_RELEASED: return "APP_MSG_ALREADY_RELEASED";
    case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
    case APP_MSG_CALLBACK_NOT_REGISTERED: return "APP_MSG_CALLBACK_NOT_REGISTERED";
    case APP_MSG_OUT_OF_MEMORY: return "APP_MSG_OUT_OF_MEMORY";
    case APP_MSG_CLOSED: return "APP_MSG_CLOSED";
    case APP_MSG_INTERNAL_ERROR: return "APP_MSG_INTERNAL_ERROR";
    default: return "UNKNOWN ERROR";
  }
}

void outbox_failed_handler(DictionaryIterator *iterator, AppMessageResult reason, void *context){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Outbox message failed; reason: %d - %s",
            reason, translate_error(reason));
}

void inbox_dropped_handler(AppMessageResult reason, void *context){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Inbox message dropped; reason: %d - %s",
            reason, translate_error(reason));
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
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    if(iter == NULL){
        APP_LOG(APP_LOG_LEVEL_DEBUG, "iter is null");
        return;
    }

    dict_write_cstring(iter, k, v);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "about to send %d, %s", k, v);
    AppMessageResult r;
    r = app_message_outbox_send();
    APP_LOG(APP_LOG_LEVEL_DEBUG, "result: %d", r);
}
