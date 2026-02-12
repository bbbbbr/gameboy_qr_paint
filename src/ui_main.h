#ifndef UI_MAIN_H
#define UI_MAIN_H


void ui_init(void) NONBANKED;
void ui_update(void) BANKED;

void ui_redraw_after_qrcode(void) BANKED;

void ui_cycle_cursor_speed(void) BANKED;
void ui_cycle_cursor_teleport(void) BANKED;

#endif // UI_MAIN_H