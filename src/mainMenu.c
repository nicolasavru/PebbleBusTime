#include <pebble.h>
#include "utils.h"
#include "busDetails.h"

MenuLayer *mainMenu_layer;
MenuLayerCallbacks mainMenu_callbacks;

// support up to 32 buses with each stop name and distance string less
// than 64 characters
char stops[32*64];
char distances[32*64];
char numBuses;
char menuIdx;

void mainMenu_draw_header(GContext *ctx, const Layer *cell_layer,
                          uint16_t section_index, void *callback_context){
    graphics_context_set_text_color(ctx, GColorBlack); // This is important.
    // graphics_draw_text(ctx, hex+2*section_index,
    //                    fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
    //                    GRect(0,0,layer_get_frame(cell_layer).size.w,layer_get_frame(cell_layer).size.h),
    //                    GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

void mainMenu_draw_row(GContext *ctx, const Layer *cell_layer,
                       MenuIndex *cell_index, void *callback_context){
    graphics_context_set_text_color(ctx, GColorBlack); // This is important.
    // graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_draw_text(ctx, stops+32*cell_index->row,
                       fonts_get_system_font(FONT_KEY_GOTHIC_14),
                       GRect(0,0,layer_get_frame(cell_layer).size.w,layer_get_frame(cell_layer).size.h-15),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    graphics_draw_text(ctx, distances+32*cell_index->row,
                       fonts_get_system_font(FONT_KEY_GOTHIC_14),
                       GRect(0,15,layer_get_frame(cell_layer).size.w,layer_get_frame(cell_layer).size.h-15),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    // Just saying layer_get_frame(cell_layer) for the 4th argument
    // doesn't work. Probably because the GContext is relative to the
    // cell already, but the cell_layer.frame is relative to the
    // menulayer or the screen or something.
}

int16_t mainMenu_get_cell_height(struct MenuLayer *menu_layer,
                                 MenuIndex *cell_index,
                                 void *callback_context){
    return 35;
}

int16_t mainMenu_get_header_height(struct MenuLayer *menu_layer,
                                   uint16_t section_index,
                                   void *callback_context){
    return 0;
}

uint16_t mainMenu_get_num_rows_in_section(struct MenuLayer *menu_layer,
                                          uint16_t section_index,
                                          void *callback_context){
    return numBuses;
}

uint16_t mainMenu_get_num_sections(struct MenuLayer *menu_layer, void *callback_context){
    return 1;
}

void mainMenu_select_click(struct MenuLayer *menu_layer,
                           MenuIndex *cell_index,
                           void *callback_context){
    send_cmd(1);
}

void mainMenu_select_long_click(struct MenuLayer *menu_layer,
                                MenuIndex *cell_index,
                                void *callback_context){
    showBusDetails(cell_index);
}

void mainMenu_load(Window *window){
    Layer *window_layer = window_get_root_layer(window);
    GRect frame = layer_get_bounds(window_layer);

    mainMenu_layer = menu_layer_create(GRect(0,40,frame.size.w,frame.size.h-40));
    // set window's button callbacks to the MenuLayer's default button callbacks
    menu_layer_set_click_config_onto_window(mainMenu_layer, window);

    // http://developer.getpebble.com/2/api-reference/group___menu_layer.html#struct_menu_layer_callbacks
    mainMenu_callbacks.draw_header = &mainMenu_draw_header;
    mainMenu_callbacks.draw_row = &mainMenu_draw_row;
    // mainMenu_callbacks.draw_separator = NULL;       // get_seperator_height must also be null
    mainMenu_callbacks.get_cell_height = &mainMenu_get_cell_height;
    mainMenu_callbacks.get_header_height = &mainMenu_get_header_height;
    mainMenu_callbacks.get_num_rows = &mainMenu_get_num_rows_in_section;
    mainMenu_callbacks.get_num_sections = &mainMenu_get_num_sections;
    // mainMenu_callbacks.get_separator_height = NULL; // defaults to 1
    mainMenu_callbacks.select_click = &mainMenu_select_click;
    mainMenu_callbacks.select_long_click = &mainMenu_select_long_click;
    mainMenu_callbacks.selection_changed = NULL;    // ignore selection changes

    menu_layer_set_callbacks(mainMenu_layer, NULL, mainMenu_callbacks);
    layer_add_child(window_layer, menu_layer_get_layer(mainMenu_layer));
    window_stack_push(window, true);
}
