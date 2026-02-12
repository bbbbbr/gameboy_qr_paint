#ifndef UI_MAIN_AREA_H
#define UI_MAIN_AREA_H


#define UI_BOTTOM_BORDER_START 120u

// ===== TOOLS MENU =====

#define TOOLS_MENU_ITEM_WIDTH  16u
#define TOOLS_MENU_ITEM_HEIGHT 16u

#define TOOLS_MENU_ITEM_COUNT   (DRAW_TOOL_COUNT) // Number of tool icons *vertically*

#define TOOLS_MENU_X_START 8u
#define TOOLS_MENU_Y_START 24u

#define TOOLS_MENU_WIDTH  (TOOLS_MENU_ITEM_WIDTH)
#define TOOLS_MENU_HEIGHT (TOOLS_MENU_ITEM_HEIGHT * TOOLS_MENU_ITEM_COUNT)

#define TOOLS_MENU_X_END  ((TOOLS_MENU_X_START) + TOOLS_MENU_WIDTH)
#define TOOLS_MENU_Y_END  ((TOOLS_MENU_Y_START) + (TOOLS_MENU_HEIGHT))


// ===== RIGHT MENU =====

#define RIGHT_MENU_ITEM_WIDTH  20u
#define RIGHT_MENU_ITEM_HEIGHT 16u

#define RIGHT_MENU_ITEM_COUNT   (RIGHT_MENU_COUNT) // Number of tool icons *vertically*

#define RIGHT_MENU_X_START 134u
#define RIGHT_MENU_Y_START 24u

#define RIGHT_MENU_WIDTH  (RIGHT_MENU_ITEM_WIDTH)
#define RIGHT_MENU_HEIGHT (RIGHT_MENU_ITEM_HEIGHT * RIGHT_MENU_ITEM_COUNT)

#define RIGHT_MENU_X_END  ((RIGHT_MENU_X_START) + RIGHT_MENU_WIDTH)
#define RIGHT_MENU_Y_END  ((RIGHT_MENU_Y_START) + (RIGHT_MENU_HEIGHT))

    // ===== SWAP COLOR BUTTON =====

    #define COLOR_CHANGE_BUTTON_WIDTH  16u
    #define COLOR_CHANGE_BUTTON_HEIGHT 16u

    #define COLOR_CHANGE_BUTTON_X_START (RIGHT_MENU_X_START + 2u)
    #define COLOR_CHANGE_BUTTON_Y_START (RIGHT_MENU_Y_START + (RIGHT_MENU_COLOR_SWAP * RIGHT_MENU_ITEM_HEIGHT))

    #define COLOR_MAIN_WIDTH   7u
    #define COLOR_MAIN_HEIGHT  7u
    #define COLOR_ALT_WIDTH    6u
    #define COLOR_ALT_HEIGHT   6u

    #define COLOR_MAIN_X_START COLOR_CHANGE_BUTTON_X_START + 2u
    #define COLOR_MAIN_Y_START COLOR_CHANGE_BUTTON_Y_START + 2u
    #define COLOR_ALT_X_START  COLOR_CHANGE_BUTTON_X_START + 6u
    #define COLOR_ALT_Y_START  COLOR_CHANGE_BUTTON_Y_START + 6u

    #define COLOR_MAIN_X_END   (COLOR_MAIN_X_START + COLOR_MAIN_WIDTH)
    #define COLOR_MAIN_Y_END   (COLOR_MAIN_Y_START + COLOR_MAIN_HEIGHT)
    #define COLOR_ALT_X_END   (COLOR_ALT_X_START   + COLOR_ALT_WIDTH)
    #define COLOR_ALT_Y_END   (COLOR_ALT_Y_START   + COLOR_ALT_HEIGHT)

    // Undo button is actually a sprite to make it cheaper to turn on/off
    #define UNDO_BUTTON_SPR_X   ((RIGHT_MENU_X_START + 7u) + DEVICE_SPRITE_PX_OFFSET_X)
    #define UNDO_BUTTON_SPR_Y   ((RIGHT_MENU_Y_START + (RIGHT_MENU_UNDO * RIGHT_MENU_ITEM_HEIGHT) + 4u) + DEVICE_SPRITE_PX_OFFSET_Y)


// ===== (BOTTOM) FILE MENU =====

#define FILE_MENU_ITEM_WIDTH  16u
#define FILE_MENU_ITEM_HEIGHT 16u

#define FILE_MENU_ITEM_COUNT   (FILE_MENU_COUNT) // Number of tool icons *vertically*

#define FILE_MENU_X_START 24u
#define FILE_MENU_Y_START 124u

#define FILE_MENU_WIDTH  (FILE_MENU_ITEM_WIDTH * FILE_MENU_ITEM_COUNT)
#define FILE_MENU_HEIGHT (FILE_MENU_ITEM_HEIGHT)

#define FILE_MENU_X_END  ((FILE_MENU_X_START) + FILE_MENU_WIDTH)
#define FILE_MENU_Y_END  ((FILE_MENU_Y_START) + (FILE_MENU_HEIGHT))




void ui_redraw_menus_all(void) NONBANKED;
void ui_handle_menu_area(uint8_t cursor_8u_x, uint8_t cursor_8u_y) BANKED;

void ui_menu_tools_draw_highlight(uint8_t tool_num, uint8_t draw_color) BANKED;
void ui_menu_file_draw_highlight(uint8_t num, uint8_t draw_color) BANKED;

void ui_undo_button_enable(void) NONBANKED;
void ui_undo_button_disable(void) NONBANKED;

#endif // UI_MAIN_AREA_H