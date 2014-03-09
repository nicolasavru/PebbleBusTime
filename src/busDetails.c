#include <pebble.h>
#include "utils.h"

Window* detail_window;
TextLayer* detail_window_text;

char text[1024];

void detailUnload(Window* win){
    APP_LOG(APP_LOG_LEVEL_INFO, "Destroying Detail Window.");
    text_layer_destroy(detail_window_text);
    window_destroy(detail_window);
}

void showBusDetails(MenuIndex* index){
    WindowHandlers wh = { .unload = &detailUnload };
    detail_window = window_create();
    window_set_window_handlers(detail_window, wh);

    detail_window_text = text_layer_create(GRect(0,0,144,168));
    text_layer_set_text_alignment(detail_window_text, GTextAlignmentLeft);
    text_layer_set_font(detail_window_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text[0] = 0; // Ensure the message starts with a null so strcat will overwrite it.
    strcat(text, "Section: ");
    strcat(text, "\nRow: ");

    text_layer_set_text(detail_window_text, text);
    layer_add_child(window_get_root_layer(detail_window),
                    text_layer_get_layer(detail_window_text));

    // The back button will dismiss the current window, not close the app.
    window_stack_push(detail_window, true);
}
