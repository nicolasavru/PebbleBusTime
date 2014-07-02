#include <pebble.h>
#include "utils.h"
#include "routeBusMenu.h"

Window* routeBusMenu_window;
MenuLayer *routeBusMenu_layer;
MenuLayerCallbacks routeBusMenu_callbacks;

char lineName[LINENAME_LEN];

// support up to 32 buses with each stop name and distance string less
// than 64 characters
// char menuIdx;

void routeBusMenu_draw_header(GContext *ctx, const Layer *cell_layer,
                          uint16_t section_index, void *callback_context){
    graphics_context_set_text_color(ctx, GColorBlack);
    if(section_index == 0){
        graphics_draw_text(ctx, lineName,
                           fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
                           GRect(0,0,layer_get_frame(cell_layer).size.w,layer_get_frame(cell_layer).size.h-15),
                           GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    }
    else{
        graphics_draw_text(ctx, section_index == 1 ? "Direction 0" : "Direction 1",
                           fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
                           GRect(0,0,layer_get_frame(cell_layer).size.w,layer_get_frame(cell_layer).size.h-15),
                           GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    }
    // graphics_draw_text(ctx, hex+2*section_index,
    //                    fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
    //                    GRect(0,0,layer_get_frame(cell_layer).size.w,layer_get_frame(cell_layer).size.h),
    //                    GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

void routeBusMenu_draw_row(GContext *ctx, const Layer *cell_layer,
                       MenuIndex *cell_index, void *callback_context){
    graphics_context_set_text_color(ctx, GColorBlack);
    // graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_draw_text(ctx, stops+32*((cell_index->section-1)*numBuses[0] + cell_index->row),
                       fonts_get_system_font(FONT_KEY_GOTHIC_14),
                       GRect(0,0,layer_get_frame(cell_layer).size.w,layer_get_frame(cell_layer).size.h-15),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    graphics_draw_text(ctx, distances+32*((cell_index->section-1)*numBuses[0] + cell_index->row),
                       fonts_get_system_font(FONT_KEY_GOTHIC_14),
                       GRect(0,15,layer_get_frame(cell_layer).size.w,layer_get_frame(cell_layer).size.h-15),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    // Just saying layer_get_frame(cell_layer) for the 4th argument
    // doesn't work. Probably because the GContext is relative to the
    // cell already, but the cell_layer.frame is relative to the
    // menulayer or the screen or something.
}

int16_t routeBusMenu_get_cell_height(struct MenuLayer *menu_layer,
                                 MenuIndex *cell_index,
                                 void *callback_context){
    return 35;
}

int16_t routeBusMenu_get_header_height(struct MenuLayer *menu_layer,
                                   uint16_t section_index,
                                   void *callback_context){
    return 40;
}

uint16_t routeBusMenu_get_num_rows_in_section(struct MenuLayer *menu_layer,
                                              uint16_t section_index,
                                              void *callback_context){
    if(section_index == 0){
        return 0;
    }
    else{
        return numBuses[section_index-1];
    }
}

// one section for each direction
uint16_t routeBusMenu_get_num_sections(struct MenuLayer *menu_layer, void *callback_context){
    return 3;
}

void routeBusMenu_select_click(struct MenuLayer *menu_layer,
                           MenuIndex *cell_index,
                           void *callback_context){
    // send_cmd(LINENAME_KEY, 1);
}

void routeBusMenu_select_long_click(struct MenuLayer *menu_layer,
                                MenuIndex *cell_index,
                                void *callback_context){
    // boroughMenu_load();
}

void routeBusMenu_unload(Window* win){
    APP_LOG(APP_LOG_LEVEL_INFO, "Destroying routeBusMenu Window.");
    menu_layer_destroy(routeBusMenu_layer);
    window_destroy(routeBusMenu_window);
}

void routeBusMenu_load(){
    WindowHandlers wh = { .unload = &routeBusMenu_unload };
    routeBusMenu_window = window_create();
    window_set_window_handlers(routeBusMenu_window, wh);

    Layer *window_layer = window_get_root_layer(routeBusMenu_window);
    GRect frame = layer_get_bounds(window_layer);

    routeBusMenu_layer = menu_layer_create(GRect(0,0,frame.size.w,frame.size.h));
    // set window's button callbacks to the MenuLayer's default button callbacks
    menu_layer_set_click_config_onto_window(routeBusMenu_layer, routeBusMenu_window);

    // http://developer.getpebble.com/2/api-reference/group___menu_layer.html#struct_menu_layer_callbacks
    routeBusMenu_callbacks.draw_header = &routeBusMenu_draw_header;
    routeBusMenu_callbacks.draw_row = &routeBusMenu_draw_row;
    // routeBusMenu_callbacks.draw_separator = NULL;       // get_seperator_height must also be null
    routeBusMenu_callbacks.get_cell_height = &routeBusMenu_get_cell_height;
    routeBusMenu_callbacks.get_header_height = &routeBusMenu_get_header_height;
    routeBusMenu_callbacks.get_num_rows = &routeBusMenu_get_num_rows_in_section;
    routeBusMenu_callbacks.get_num_sections = &routeBusMenu_get_num_sections;
    // routeBusMenu_callbacks.get_separator_height = NULL; // defaults to 1
    routeBusMenu_callbacks.select_click = &routeBusMenu_select_click;
    routeBusMenu_callbacks.select_long_click = &routeBusMenu_select_long_click;
    routeBusMenu_callbacks.selection_changed = NULL;    // ignore selection changes

    menu_layer_set_callbacks(routeBusMenu_layer, NULL, routeBusMenu_callbacks);
    layer_add_child(window_layer, menu_layer_get_layer(routeBusMenu_layer));
    window_stack_push(routeBusMenu_window, true);
}
