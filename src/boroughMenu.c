#include <pebble.h>
#include "utils.h"
#include "routeMenu.h"

Window* boroughMenu_window;
MenuLayer *boroughMenu_layer;
MenuLayerCallbacks boroughMenu_callbacks;

const char *boroughs[] = {"Bronx", "Brooklyn", "Manhattan", "Queens", "Staten Island", "Express"};
const char *borough_prefixes[] = {"Bx", "B", "M", "Q", "S", "X"};

void boroughMenu_draw_header(GContext *ctx, const Layer *cell_layer,
                             uint16_t section_index, void *callback_context){
    graphics_context_set_text_color(ctx, GColorBlack);
}

void boroughMenu_draw_row(GContext *ctx, const Layer *cell_layer,
                          MenuIndex *cell_index, void *callback_context){
    graphics_context_set_text_color(ctx, GColorBlack);
    // graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_draw_text(ctx, boroughs[cell_index->row],
                       fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
                       GRect(0,0,layer_get_frame(cell_layer).size.w,layer_get_frame(cell_layer).size.h),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    // Just saying layer_get_frame(cell_layer) for the 4th argument
    // doesn't work. Probably because the GContext is relative to the
    // cell already, but the cell_layer.frame is relative to the
    // menulayer or the screen or something.
}

int16_t boroughMenu_get_cell_height(struct MenuLayer *menu_layer,
                                    MenuIndex *cell_index,
                                    void *callback_context){
    return 35;
}

int16_t boroughMenu_get_header_height(struct MenuLayer *menu_layer,
                                      uint16_t section_index,
                                      void *callback_context){
    return 0;
}

// assumes the number of boroughs in NYC will not change
uint16_t boroughMenu_get_num_rows_in_section(struct MenuLayer *menu_layer,
                                             uint16_t section_index,
                                             void *callback_context){
    return 6;
}

uint16_t boroughMenu_get_num_sections(struct MenuLayer *menu_layer, void *callback_context){
    return 1;
}

void boroughMenu_select_click(struct MenuLayer *menu_layer,
                              MenuIndex *cell_index,
                              void *callback_context){
    send_str(ROUTES_KEY, borough_prefixes[cell_index->row]);
    routeMenu_load();
}

void boroughMenu_select_long_click(struct MenuLayer *menu_layer,
                                   MenuIndex *cell_index,
                                   void *callback_context){

}

void boroughMenu_unload(Window* win){
    APP_LOG(APP_LOG_LEVEL_INFO, "Destroying boroughMenu Window.");
    menu_layer_destroy(boroughMenu_layer);
    window_destroy(boroughMenu_window);
}

void boroughMenu_load(){
    WindowHandlers wh = { .unload = &boroughMenu_unload };
    boroughMenu_window = window_create();
    window_set_window_handlers(boroughMenu_window, wh);

    Layer *window_layer = window_get_root_layer(boroughMenu_window);
    GRect frame = layer_get_bounds(window_layer);

    boroughMenu_layer = menu_layer_create(GRect(0,0,frame.size.w,frame.size.h));
    // set window's button callbacks to the MenuLayer's default button callbacks
    menu_layer_set_click_config_onto_window(boroughMenu_layer, boroughMenu_window);

    // http://developer.getpebble.com/2/api-reference/group___menu_layer.html#struct_menu_layer_callbacks
    boroughMenu_callbacks.draw_header = &boroughMenu_draw_header;
    boroughMenu_callbacks.draw_row = &boroughMenu_draw_row;
    // boroughMenu_callbacks.draw_separator = NULL;       // get_seperator_height must also be null
    boroughMenu_callbacks.get_cell_height = &boroughMenu_get_cell_height;
    boroughMenu_callbacks.get_header_height = &boroughMenu_get_header_height;
    boroughMenu_callbacks.get_num_rows = &boroughMenu_get_num_rows_in_section;
    boroughMenu_callbacks.get_num_sections = &boroughMenu_get_num_sections;
    // boroughMenu_callbacks.get_separator_height = NULL; // defaults to 1
    boroughMenu_callbacks.select_click = &boroughMenu_select_click;
    boroughMenu_callbacks.select_long_click = &boroughMenu_select_long_click;
    boroughMenu_callbacks.selection_changed = NULL;    // ignore selection changes

    menu_layer_set_callbacks(boroughMenu_layer, NULL, boroughMenu_callbacks);
    layer_add_child(window_layer, menu_layer_get_layer(boroughMenu_layer));
    window_stack_push(boroughMenu_window, true);
}
