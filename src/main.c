#include <pebble.h>
#include "mainMenu.h"

Window *window;

TextLayer *lineName_layer;
TextLayer *stopName_layer;

AppSync sync;
uint8_t sync_buffer[1024];

enum BusKey{
  LINENAME_KEY = 0x0,         // TUPLE_CSTRING
  STOPNAME_KEY = 0x1,         // TUPLE_CSTRING
  NUMBUSES_KEY = 0x2,         // TUPLE_INT
  BUS_KEY = 0x3,              // TUPLE_CSTRING
};

void sync_error_callback(DictionaryResult dict_error,
                         AppMessageResult app_message_error,
                         void *context){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple,
                                 const Tuple* old_tuple, void* context){
    switch(key){
    case LINENAME_KEY:
        text_layer_set_text(lineName_layer, new_tuple->value->cstring);
        break;
    case STOPNAME_KEY:
        text_layer_set_text(stopName_layer, new_tuple->value->cstring);
        break;
    case NUMBUSES_KEY:
        numBuses = new_tuple->value->uint8;
        menu_layer_reload_data(mainMenu_layer);
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

    // case STOPS_KEY:
    //     // Response format:
    //     // Header of one bytes containing the total number of results,
    //     // and one byte containing our current offset.  This is
    //     // followed by a sequence of entries, which consist of one
    //     // length byte followed by ASCII data
    //     numBuses = new_tuple->value->cstring[0];
    //     menuIdx = new_tuple->value->cstring[1];
    //     int messageLoc = 2;
    //     char entryLen;
    //     for(int i = 0; i < numBuses; i++){
    //         entryLen = new_tuple->value->cstring[messageLoc++];
    //         strncpy(stops+(menuIdx+i)*32, new_tuple->value->cstring+messageLoc, entryLen);
    //         messageLoc += entryLen;
    //         stops[(menuIdx+i+1)*32-1] = 0;
    //     }
    }
}

//  void select_click_handler(ClickRecognizerRef recognizer, void *context) {
//     send_cmd(0);
// }

//  void up_click_handler(ClickRecognizerRef recognizer, void *context){
//     send_cmd(1);
// }

//  void down_click_handler(ClickRecognizerRef recognizer, void *context){
//     send_cmd(-1);
// }

//  void click_config_provider(void *context){
//     window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
//     window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
//     window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
// }

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
    };

    app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
                  sync_tuple_changed_callback, sync_error_callback, NULL);

    mainMenu_load(window);
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
  // window_set_click_config_provider(window, click_config_provider);
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
