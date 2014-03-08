#include "pebble.h"

static Window *window;

static TextLayer *stopName_layer;
static TextLayer *lineName_layer;
static TextLayer *distance_layer;

static AppSync sync;
static uint8_t sync_buffer[64];

enum BusKey {
  STOPNAME_KEY = 0x0,         // TUPLE_CSTRING
  LINENAME_KEY = 0x1,         // TUPLE_CSTRING
  DISTANCE_KEY = 0x2,         // TUPLE_CSTRING
};

static void sync_error_callback(DictionaryResult dict_error,
                                AppMessageResult app_message_error,
                                void *context){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple,
                                        const Tuple* old_tuple, void* context){
    switch(key){
    case STOPNAME_KEY:
        text_layer_set_text(stopName_layer, new_tuple->value->cstring);
        break;
    case LINENAME_KEY:
        // App Sync keeps new_tuple in sync_buffer, so we may use it directly
        text_layer_set_text(lineName_layer, new_tuple->value->cstring);
        break;
    case DISTANCE_KEY:
        text_layer_set_text(distance_layer, new_tuple->value->cstring);
        break;
    }
}

static void send_cmd(void){
  Tuplet value = TupletInteger(1, 1);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &value);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void window_load(Window *window){
  Layer *window_layer = window_get_root_layer(window);

  stopName_layer = text_layer_create(GRect(0, 65, 144, 68));
  text_layer_set_text_color(stopName_layer, GColorWhite);
  text_layer_set_background_color(stopName_layer, GColorClear);
  text_layer_set_font(stopName_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(stopName_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(stopName_layer));

  lineName_layer = text_layer_create(GRect(0, 95, 144, 68));
  text_layer_set_text_color(lineName_layer, GColorWhite);
  text_layer_set_background_color(lineName_layer, GColorClear);
  text_layer_set_font(lineName_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(lineName_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(lineName_layer));

  distance_layer = text_layer_create(GRect(0, 125, 144, 68));
  text_layer_set_text_color(distance_layer, GColorWhite);
  text_layer_set_background_color(distance_layer, GColorClear);
  text_layer_set_font(distance_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(distance_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(distance_layer));

  Tuplet initial_values[] = {
    TupletCString(STOPNAME_KEY, "foo"),
    TupletCString(LINENAME_KEY, "bar"),
    TupletCString(DISTANCE_KEY, "baz"),
  };

  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);

  send_cmd();
}

static void window_unload(Window *window){
  app_sync_deinit(&sync);

  text_layer_destroy(stopName_layer);
  text_layer_destroy(lineName_layer);
  text_layer_destroy(distance_layer);
}

static void init(void){
  window = window_create();
  window_set_background_color(window, GColorBlack);
  window_set_fullscreen(window, true);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });

  const int inbound_size = 64;
  const int outbound_size = 64;
  app_message_open(inbound_size, outbound_size);

  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void){
  window_destroy(window);
}

int main(void){
  init();
  app_event_loop();
  deinit();
}