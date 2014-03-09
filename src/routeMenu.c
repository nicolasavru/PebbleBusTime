#include <pebble.h>
#include "utils.h"

Window* routeMenu_window;
MenuLayer *routeMenu_layer;
MenuLayerCallbacks routeMenu_callbacks;

// support up to 128 routes with each route name less than 10 chars
char routes[128*10];
int numRoutes;

void routeMenu_draw_header(GContext *ctx, const Layer *cell_layer,
                             uint16_t section_index, void *callback_context){
    graphics_context_set_text_color(ctx, GColorBlack);
}

void routeMenu_draw_row(GContext *ctx, const Layer *cell_layer,
                          MenuIndex *cell_index, void *callback_context){
    graphics_context_set_text_color(ctx, GColorBlack);
    // graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_draw_text(ctx, routes+10*cell_index->row,
                       fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
                       GRect(0,0,layer_get_frame(cell_layer).size.w,layer_get_frame(cell_layer).size.h),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    // Just saying layer_get_frame(cell_layer) for the 4th argument
    // doesn't work. Probably because the GContext is relative to the
    // cell already, but the cell_layer.frame is relative to the
    // menulayer or the screen or something.
}

int16_t routeMenu_get_cell_height(struct MenuLayer *menu_layer,
                                    MenuIndex *cell_index,
                                    void *callback_context){
    return 35;
}

int16_t routeMenu_get_header_height(struct MenuLayer *menu_layer,
                                      uint16_t section_index,
                                      void *callback_context){
    return 0;
}

uint16_t routeMenu_get_num_rows_in_section(struct MenuLayer *menu_layer,
                                             uint16_t section_index,
                                           void *callback_context){
    return numRoutes;
}

uint16_t routeMenu_get_num_sections(struct MenuLayer *menu_layer, void *callback_context){
    return 1;
}

void routeMenu_select_click(struct MenuLayer *menu_layer,
                              MenuIndex *cell_index,
                              void *callback_context){
    send_cmd(ROUTES_KEY, cell_index->row);
}

void routeMenu_select_long_click(struct MenuLayer *menu_layer,
                                   MenuIndex *cell_index,
                                   void *callback_context){
    // showBusDetails(cell_index);
}

void routeMenu_unload(Window* win){
    APP_LOG(APP_LOG_LEVEL_INFO, "Destroying routeMenu Window.");
    menu_layer_destroy(routeMenu_layer);
    window_destroy(routeMenu_window);
}

void routeMenu_load(){
    WindowHandlers wh = { .unload = &routeMenu_unload };
    routeMenu_window = window_create();
    window_set_window_handlers(routeMenu_window, wh);

    Layer *window_layer = window_get_root_layer(routeMenu_window);
    GRect frame = layer_get_bounds(window_layer);

    routeMenu_layer = menu_layer_create(GRect(0,0,frame.size.w,frame.size.h));
    // set window's button callbacks to the MenuLayer's default button callbacks
    menu_layer_set_click_config_onto_window(routeMenu_layer, routeMenu_window);

    // http://developer.getpebble.com/2/api-reference/group___menu_layer.html#struct_menu_layer_callbacks
    routeMenu_callbacks.draw_header = &routeMenu_draw_header;
    routeMenu_callbacks.draw_row = &routeMenu_draw_row;
    // routeMenu_callbacks.draw_separator = NULL;       // get_seperator_height must also be null
    routeMenu_callbacks.get_cell_height = &routeMenu_get_cell_height;
    routeMenu_callbacks.get_header_height = &routeMenu_get_header_height;
    routeMenu_callbacks.get_num_rows = &routeMenu_get_num_rows_in_section;
    routeMenu_callbacks.get_num_sections = &routeMenu_get_num_sections;
    // routeMenu_callbacks.get_separator_height = NULL; // defaults to 1
    routeMenu_callbacks.select_click = &routeMenu_select_click;
    routeMenu_callbacks.select_long_click = &routeMenu_select_long_click;
    routeMenu_callbacks.selection_changed = NULL;    // ignore selection changes

    menu_layer_set_callbacks(routeMenu_layer, NULL, routeMenu_callbacks);
    layer_add_child(window_layer, menu_layer_get_layer(routeMenu_layer));
    window_stack_push(routeMenu_window, true);
}
