#include <gbdk/platform.h>
#include <stdint.h>

#include <gb/drawing.h>

#include "common.h"
#include "input.h"

#include "ui_main.h"

#include <gbdk/emu_debug.h>  // Sensitive to duplicated line position across source files

#pragma bank 255  // Autobanked

#define DRAWING_ROW_OF_TILES_SZ   (IMG_WIDTH_TILES * TILE_SZ_BYTES)
#define SCREEN_ROW_SZ             (DEVICE_SCREEN_WIDTH * TILE_SZ_BYTES)
#define DRAWING_SAVE_SLOT_SIZE    (IMG_WIDTH_TILES * IMG_HEIGHT_TILES * TILE_SZ_BYTES)
#define DRAWING_VRAM_START        (APA_MODE_VRAM_START + (((IMG_TILE_Y_START * DEVICE_SCREEN_WIDTH) + IMG_TILE_X_START) * TILE_SZ_BYTES))


static void test_load_save(void);


void drawing_save_to_sram(uint8_t sram_bank, uint8_t save_slot) BANKED {

    SWITCH_RAM(sram_bank);
    DISPLAY_OFF;
    uint8_t * p_sram_save_slot = (uint8_t *)(SRAM_BASE_A000 + (DRAWING_SAVE_SLOT_SIZE * save_slot));
    uint8_t * p_vram_drawing   = (uint8_t *)(DRAWING_VRAM_START);

    for (uint8_t tile_row = 0u; tile_row < IMG_HEIGHT_TILES; tile_row++) {
        vmemcpy(p_sram_save_slot, p_vram_drawing, DRAWING_ROW_OF_TILES_SZ); // Copy all tile patterns
        p_sram_save_slot += DRAWING_ROW_OF_TILES_SZ;
        p_vram_drawing   += SCREEN_ROW_SZ;
    }
    DISPLAY_ON;
}

void drawing_restore_from_sram(uint8_t sram_bank, uint8_t save_slot) BANKED {

    SWITCH_RAM(sram_bank);
    DISPLAY_OFF;
    uint8_t * p_sram_save_slot = (uint8_t *)(SRAM_BASE_A000 + (DRAWING_SAVE_SLOT_SIZE * save_slot));
    uint8_t * p_vram_drawing   = (uint8_t *)(DRAWING_VRAM_START);

    for (uint8_t tile_row = 0u; tile_row < IMG_HEIGHT_TILES; tile_row++) {
        vmemcpy(p_vram_drawing, p_sram_save_slot, DRAWING_ROW_OF_TILES_SZ); // Copy all tile patterns
        p_sram_save_slot += DRAWING_ROW_OF_TILES_SZ;
        p_vram_drawing   += SCREEN_ROW_SZ;
    }
    DISPLAY_ON;

}


// TODO: For testing, not final Controls UI
static void test_load_save(void) {
    
    switch (GET_KEYS_TICKED(~J_SELECT)) {
        case J_UP:   if (app_state.save_slot_current > DRAWING_SAVE_SLOT_MIN) app_state.save_slot_current--;
            break;
        case J_DOWN: if (app_state.save_slot_current < DRAWING_SAVE_SLOT_MAX) app_state.save_slot_current++;
            break;
        case J_A:    drawing_restore_from_sram(SRAM_BANK_DRAWING_SAVES, app_state.save_slot_current);
            break;
        case J_B:    drawing_save_to_sram(SRAM_BANK_DRAWING_SAVES, app_state.save_slot_current);
            break;
    }
}


// Draws the paint working area
void drawing_restore_default_colors(void) BANKED {
    // For pixel drawing
    color(BLACK,WHITE,SOLID);    
}



void draw_init(void) BANKED {

    // Set default brush For pixel drawing
    color(BLACK,WHITE,SOLID);
}


// Expects UPDATE_KEYS() to have been called before each invocation
void draw_update(uint8_t cursor_8u_x, uint8_t cursor_8u_y) BANKED {

    // if (KEY_PRESSED(J_SELECT)) {
    //     test_load_save();
    //     return;
    // }
    if (KEY_TICKED(J_SELECT)) ui_cycle_cursor_speed();

    switch (KEYS() & (J_A | J_B)) {
        case (J_A | J_B):
            // Clear the screen and reset some things

            // Fill active image area in white
            color(WHITE, WHITE, SOLID);
            box(IMG_X_START, IMG_Y_START, IMG_X_END, IMG_Y_END, M_FILL);
            // // For pixel drawing
            color(BLACK, WHITE, SOLID);
            app_state.cursor_8u_last_x = app_state.cursor_8u_last_y = CURSOR_POS_UNSET_8U;

            // Wait for both buttons up before drawing again to avoid leaving a dot after clearing screen
            app_state.buttons_up_pending = true;
            break;

        case J_A: if (!app_state.buttons_up_pending) {
                        plot_point(cursor_8u_x, cursor_8u_y);
                        app_state.cursor_8u_last_x = cursor_8u_x;
                        app_state.cursor_8u_last_y = cursor_8u_y;
                    }
            break;

        case J_B: if (!app_state.buttons_up_pending) {
                        // if (app_state.cursor_8u_last_x != CURSOR_POS_UNSET_8U) {
                        //     line(app_state.cursor_8u_last_x, app_state.cursor_8u_last_y, cursor_8u_x, cursor_8u_y);
                        // }
                        app_state.cursor_8u_last_x = cursor_8u_x;
                        app_state.cursor_8u_last_y = cursor_8u_y;
                    }
            break;


        default: // no buttons pressed
            app_state.buttons_up_pending = false;
            break;
    }
}
