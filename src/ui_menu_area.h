#ifndef UI_MAIN_AREA_H
#define UI_MAIN_AREA_H


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


// ===== LOAD/SAVE MENU =====

#define FILE_MENU_ITEM_WIDTH  20u
#define FILE_MENU_ITEM_HEIGHT 16u

#define FILE_MENU_ITEM_COUNT   (FILE_MENU_COUNT) // Number of tool icons *vertically*

#define FILE_MENU_X_START 134u
#define FILE_MENU_Y_START 24u

#define FILE_MENU_WIDTH  (FILE_MENU_ITEM_WIDTH)
#define FILE_MENU_HEIGHT (FILE_MENU_ITEM_HEIGHT * FILE_MENU_ITEM_COUNT)

#define FILE_MENU_X_END  ((FILE_MENU_X_START) + FILE_MENU_WIDTH)
#define FILE_MENU_Y_END  ((FILE_MENU_Y_START) + (FILE_MENU_HEIGHT))


// ===== CLEAR BUTTON =====

#define CLEAR_BUTTON_WIDTH  24u
#define CLEAR_BUTTON_HEIGHT 8u

#define CLEAR_BUTTON_X_START 32u
#define CLEAR_BUTTON_Y_START 128u

#define CLEAR_BUTTON_X_END  ((CLEAR_BUTTON_X_START) + CLEAR_BUTTON_WIDTH)
#define CLEAR_BUTTON_Y_END  ((CLEAR_BUTTON_Y_START) + (CLEAR_BUTTON_HEIGHT))



void ui_redraw_menus_all(void) NONBANKED;
void ui_handle_menu_area(uint8_t cursor_8u_x, uint8_t cursor_8u_y) BANKED;

void ui_menu_tools_draw_highlight(uint8_t tool_num, uint8_t draw_color) BANKED;
void ui_menu_file_draw_highlight(uint8_t num, uint8_t draw_color) BANKED;

#endif // UI_MAIN_AREA_H