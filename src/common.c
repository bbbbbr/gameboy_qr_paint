#include <gbdk/platform.h>
#include <stdint.h>

#include "common.h"

#pragma bank 255  // Autobanked

app_state_t app_state;

void app_state_reset(void) BANKED {

    // Save related
    app_state.save_slot_current = DRAWING_SAVE_SLOT_MIN;

    // UI related
    app_state.cursor_x = CURSOR_8U_TO_16U(DEVICE_SCREEN_PX_WIDTH / 2);
    app_state.cursor_y = CURSOR_8U_TO_16U(DEVICE_SCREEN_PX_HEIGHT / 2);

    app_state.cursor_8u_cache_x = CURSOR_TO_8U_X();
    app_state.cursor_8u_cache_y = CURSOR_TO_8U_Y();

    app_state.cursor_8u_last_x = CURSOR_POS_UNSET_8U;
    app_state.cursor_8u_last_y = CURSOR_POS_UNSET_8U;

    // Cursor UI teleport defaults
    app_state.cursor_draw_saved_x = app_state.cursor_x;
    app_state.cursor_draw_saved_y = app_state.cursor_y;

    app_state.cursor_left_menu_saved_x = CURSOR_8U_TO_16U(DEVICE_SCREEN_PX_WIDTH / 10);
    app_state.cursor_left_menu_saved_y = CURSOR_8U_TO_16U(DEVICE_SCREEN_PX_HEIGHT / 2);
    
    app_state.cursor_right_menu_saved_x = CURSOR_8U_TO_16U(DEVICE_SCREEN_PX_WIDTH - (DEVICE_SCREEN_PX_WIDTH / 10));
    app_state.cursor_right_menu_saved_y = CURSOR_8U_TO_16U(DEVICE_SCREEN_PX_HEIGHT / 2);

    uint16_t cursor_left_menu_saved_x;
    uint16_t cursor_left_menu_saved_y;
    uint16_t cursor_right_menu_saved_x;
    uint16_t cursor_right_menu_saved_y;


    app_state.buttons_up_pending = false;

    app_state.cursor_speed_mode = CURSOR_SPEED_MODE_DEFAULT;
    app_state.cursor_teleport_zone = CURSOR_TELEPORT_DEFAULT;
}
