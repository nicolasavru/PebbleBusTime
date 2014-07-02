#include <pebble.h>
#include "utils.h"
#include "mainMenu.h"
#include "boroughMenu.h"
#include "routeMenu.h"
#include "routeBusMenu.h"
#include "main.h"

Window *window;

TextLayer *lineName_layer;
TextLayer *stopName_layer;

AppSync sync;
uint8_t sync_buffer[1024];

char stopName[STOPNAME_LEN];

int seqnum;
char buf[1024];

void sync_error_callback(DictionaryResult dict_error,
                         AppMessageResult app_message_error,
                         void *context){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d - %s",
            app_message_error, translate_error(app_message_error));
}

void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple,
                                 const Tuple* old_tuple, void* context){
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "key: %u", (unsigned int) key);
    switch(key){
    case LINENAME_KEY:
        // APP_LOG(APP_LOG_LEVEL_DEBUG, "lineName changed: %d, %s",
        //         strlen(new_tuple->value->cstring),
        //         new_tuple->value->cstring);
        strncpy(lineName, new_tuple->value->cstring, LINENAME_LEN-1);
        lineName[LINENAME_LEN-1] = 0;
        text_layer_set_text(lineName_layer, lineName);
        break;
    case STOPNAME_KEY:
        strncpy(stopName, new_tuple->value->cstring, STOPNAME_LEN-1);
        stopName[STOPNAME_LEN-1] = 0;
        text_layer_set_text(stopName_layer, stopName);
        break;
    case NUMBUSES_KEY:
        numBuses[0] = new_tuple->value->data[0];
        numBuses[1] = new_tuple->value->data[1];
        menu_layer_reload_data(mainMenu_layer);
        if(routeBusMenu_layer){
            menu_layer_reload_data(routeBusMenu_layer);
        }
        break;

    case BUS_KEY:
        // Response format:
        // 1 byte: bus index
        // 1 byte: stop string length
        // n bytes: stop name
        // 1 byte: distance string length
        // n byes: distance string
        menuIdx = new_tuple->value->cstring[0];
        int messageLoc = 1;
        char entryLen;
        entryLen = new_tuple->value->cstring[messageLoc++];
        strncpy(stops+menuIdx*32, new_tuple->value->cstring+messageLoc, entryLen);
        stops[menuIdx*32+entryLen] = 0;
        messageLoc += entryLen;
        // APP_LOG(APP_LOG_LEVEL_DEBUG, "stop: %s", stops+menuIdx*32);

        entryLen = new_tuple->value->cstring[messageLoc++];
        strncpy(distances+menuIdx*32, new_tuple->value->cstring+messageLoc, entryLen);
        distances[menuIdx*32+entryLen] = 0;

        menu_layer_reload_data(mainMenu_layer);
        break;

    case ROUTES_KEY:
        // Response format:
        // 112 byte string
        // 1 byte: number of packets
        // 1 byte: current packet number
        // data: 1 byte specifying size of data, followed by n bytes of data

        seqnum = new_tuple->value->cstring[1];
        // first two bytes (number of packets and seqnum) are not
        // needed locally, so don't copy them
        memcpy(buf+seqnum*112, new_tuple->value->cstring+2, 112);

        // got last segment
        // APP_LOG(APP_LOG_LEVEL_DEBUG, "seqnum, numseqs: %d, %d", seqnum, new_tuple->value->cstring[0]);
        if(seqnum == new_tuple->value->cstring[0]-1){
            int bufLoc = 0;
            int entryNum = 0;
            int entryLen;
            // APP_LOG(APP_LOG_LEVEL_DEBUG, "buf[%d] = %c", 108, buf[108]);
            // APP_LOG(APP_LOG_LEVEL_DEBUG, "buf[%d] = %c", 109, buf[109]);
            // APP_LOG(APP_LOG_LEVEL_DEBUG, "buf[%d] = %c", 110, buf[110]);
            // APP_LOG(APP_LOG_LEVEL_DEBUG, "buf[%d] = %c", 111, buf[111]);
            // APP_LOG(APP_LOG_LEVEL_DEBUG, "buf[%d] = %c", 112, buf[112]);
            // APP_LOG(APP_LOG_LEVEL_DEBUG, "buf[%d] = %c", 113, buf[113]);
            // APP_LOG(APP_LOG_LEVEL_DEBUG, "buf[%d] = %c", 114, buf[114]);

            while(buf[bufLoc] != 255){
                // APP_LOG(APP_LOG_LEVEL_DEBUG, "buf[%d] = %c", bufLoc, buf[bufLoc]);
                entryLen = buf[bufLoc++];
                // APP_LOG(APP_LOG_LEVEL_DEBUG, "entryLen: %d", entryLen);
                // APP_LOG(APP_LOG_LEVEL_DEBUG, "entryNum: %d", entryNum);
                memcpy(routes+entryNum*10, buf+bufLoc, entryLen);
                routes[entryNum*10+entryLen] = 0;
                entryNum++;
                bufLoc+=entryLen;
            }
            numRoutes = entryNum;
            menu_layer_reload_data(routeMenu_layer);
            // APP_LOG(APP_LOG_LEVEL_DEBUG, "numRoutes: %d", numRoutes);
        }
        break;
    }
}

void window_load(Window *window){
    Layer *window_layer = window_get_root_layer(window);

    lineName_layer = text_layer_create(GRect(0, -5, 144, 30));
    text_layer_set_text_color(lineName_layer, GColorWhite);
    text_layer_set_background_color(lineName_layer, GColorBlack);
    text_layer_set_font(lineName_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_alignment(lineName_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(lineName_layer));

    stopName_layer = text_layer_create(GRect(0, 20, 144, 20));
    text_layer_set_text_color(stopName_layer, GColorWhite);
    text_layer_set_background_color(stopName_layer, GColorBlack);
    text_layer_set_font(stopName_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(stopName_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(stopName_layer));

    Tuplet initial_values[] = {
        TupletCString(LINENAME_KEY, "foo"),
        TupletCString(STOPNAME_KEY, "bar"),
        TupletInteger(NUMBUSES_KEY, (uint8_t) 0),
        TupletCString(BUS_KEY, "\0\0\0\0\0\0"),
        TupletCString(ROUTES_KEY, "\0\0\0\0\0\0"),
    };

    app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
                  sync_tuple_changed_callback, sync_error_callback, NULL);
    app_message_register_outbox_failed(&outbox_failed_handler);
    app_message_register_inbox_dropped(&inbox_dropped_handler);

    mainMenu_load();
}

void window_unload(Window *window){
    app_sync_deinit(&sync);

    text_layer_destroy(lineName_layer);
    text_layer_destroy(stopName_layer);
    menu_layer_destroy(mainMenu_layer);
}

void init(void){
  window = window_create();
  // window_set_background_color(window, GColorBlack);
  window_set_fullscreen(window, true);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });

  const int inbound_size = 128;
  const int outbound_size = 64;
  app_message_open(inbound_size, outbound_size);

  const bool animated = true;
  window_stack_push(window, animated);
}

void deinit(void){
  window_destroy(window);
}

int main(void){
  init();
  app_event_loop();
  deinit();
}
