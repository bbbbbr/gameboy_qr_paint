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

#define TOOL_ERASER_SIZE  4u

static uint8_t get_radius(uint8_t cursor_8u_x, uint8_t cursor_8u_y);

static void draw_tool_pencil(uint8_t cursor_8u_x, uint8_t cursor_8u_y);
static void draw_tool_line(uint8_t cursor_8u_x, uint8_t cursor_8u_y);
static void draw_tool_rect(uint8_t cursor_8u_x, uint8_t cursor_8u_y);
static void draw_tool_circle(uint8_t cursor_8u_x, uint8_t cursor_8u_y);
static void draw_tool_eraser(uint8_t cursor_8u_x, uint8_t cursor_8u_y);


static uint8_t tool_start_x, tool_start_y;
static bool    tool_currently_drawing = false;
static bool    tool_fillstyle = M_NOFILL;

// TODO: REORG: split sram load and save out to to new file: file_loadsave.c
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


// // TODO: For testing, not final Controls UI
// static void test_load_save(void) {
//
//     switch (GET_KEYS_TICKED(~J_SELECT)) {
//         case J_UP:   if (app_state.save_slot_current > DRAWING_SAVE_SLOT_MIN) app_state.save_slot_current--;
//             break;
//         case J_DOWN: if (app_state.save_slot_current < DRAWING_SAVE_SLOT_MAX) app_state.save_slot_current++;
//             break;
//         case J_A:    drawing_restore_from_sram(SRAM_BANK_DRAWING_SAVES, app_state.save_slot_current);
//             break;
//         case J_B:    drawing_save_to_sram(SRAM_BANK_DRAWING_SAVES, app_state.save_slot_current);
//             break;
//     }
// }


// Draws the paint working area
void drawing_restore_default_colors(void) BANKED {
    // For pixel drawing
    color(BLACK,WHITE,SOLID);
}


void drawing_clear(void) BANKED {

    // Fill active image area in white
    color(WHITE, WHITE, SOLID);
    box(IMG_X_START, IMG_Y_START, IMG_X_END, IMG_Y_END, M_FILL);

    drawing_restore_default_colors();
}

void draw_init(void) BANKED {

    // Set default brush For pixel drawing
    color(BLACK,WHITE,SOLID);
}


// Expects UPDATE_KEYS() to have been called before each invocation
void draw_update(uint8_t cursor_8u_x, uint8_t cursor_8u_y) BANKED {

    if (KEY_TICKED(J_SELECT)) ui_cycle_cursor_speed();  // TODO: Possibly buggy right now?

    switch (app_state.drawing_tool) {
        case DRAW_TOOL_PENCIL: draw_tool_pencil(cursor_8u_x,cursor_8u_y);
            break;
        case DRAW_TOOL_LINE: draw_tool_line(cursor_8u_x,cursor_8u_y);
            break;
        case DRAW_TOOL_ERASER: draw_tool_eraser(cursor_8u_x,cursor_8u_y);
            break;
        case DRAW_TOOL_RECT:  draw_tool_rect(cursor_8u_x,cursor_8u_y);
            break;
        case DRAW_TOOL_CIRCLE: draw_tool_circle(cursor_8u_x,cursor_8u_y);
            break;
        case DRAW_TOOL_FLOODFILL:
            break;
    }

    app_state.draw_cursor_8u_last_x = cursor_8u_x;
    app_state.draw_cursor_8u_last_y = cursor_8u_y;
}


// Clear any pending tool behavior and state
// May be called when switching tools/etc
void draw_tools_cancel_and_reset(void) BANKED {  // TODO

    // Clear any reservation on the B button
    app_state.draw_tool_using_b_button_action = false;

    tool_currently_drawing = false;

    // switch (app_state.drawing_tool) {
    //     case DRAW_TOOL_PENCIL: // Nothing to reset for pencil
    //         break;
    //     case DRAW_TOOL_LINE: tool_currently_drawing = false; // Undraw any pending lines
    //         break;
    //     case DRAW_TOOL_ERASER:
    //         break;
    //     case DRAW_TOOL_RECT:
    //         break;
    //     case DRAW_TOOL_CIRCLE:
    //         break;
    //     case DRAW_TOOL_FLOODFILL:
    //         break;
    // }
}


static void draw_tool_pencil(uint8_t cursor_8u_x, uint8_t cursor_8u_y) {

    if (KEY_PRESSED(DRAW_MAIN_BUTTON)) {
        plot_point(cursor_8u_x, cursor_8u_y);
    }
}


static void draw_tool_line(uint8_t cursor_8u_x, uint8_t cursor_8u_y) {

    if (tool_currently_drawing == false) {

        // Start drawing a line
        if (KEY_TICKED(DRAW_MAIN_BUTTON)) {
            tool_start_x = cursor_8u_x;
            tool_start_y = cursor_8u_y;
            // Draw the first line(1 pixel) XOR style so it can be undrawn
            color(BLACK,WHITE,XOR);
            line(tool_start_x, tool_start_y, cursor_8u_x, cursor_8u_y);

            // Set line starting point
            app_state.draw_tool_using_b_button_action = true;
            tool_currently_drawing = true;
        }

    } else {
        // Line drawing is active, currently previewing position

        // Un-draw the line from the last frame (XOR)
        // But only if the cursor moved, so that it remains visible otherwise
        bool new_draw_position = false;
        if ((cursor_8u_x != app_state.draw_cursor_8u_last_x) || (cursor_8u_y !=app_state.draw_cursor_8u_last_y)) {
            color(BLACK,WHITE,XOR);
            line(tool_start_x, tool_start_y, app_state.draw_cursor_8u_last_x, app_state.draw_cursor_8u_last_y);
            new_draw_position = true;
        }

        // If finalizing the line is requested, draw it normally
        if (KEY_TICKED(DRAW_MAIN_BUTTON)) {
            // Finalize the line
            drawing_restore_default_colors();
            line(tool_start_x, tool_start_y, cursor_8u_x, cursor_8u_y);

            // Finalizing the line doesn't end line drawing, instead
            // it begins a new line at the current position
            tool_start_x = cursor_8u_x;
            tool_start_y = cursor_8u_y;
        }
        else {
            // Otherwise the Line is still being actively drawn

            // B Button cancels drawing the current line
            if (KEY_TICKED(DRAW_CANCEL_BUTTON)) {

                // Cancel all line drawing
                tool_currently_drawing = false;
                app_state.draw_tool_using_b_button_action = false;

                // If it's the same position as before then need to undraw the line to cancel
                if (!new_draw_position) {
                    color(BLACK,WHITE,XOR);
                    line(tool_start_x, tool_start_y, app_state.draw_cursor_8u_last_x, app_state.draw_cursor_8u_last_y);
                }
            }
            else {
                // Otherwise, if moved, update the line preview to the new position
                if (new_draw_position) {
                    // Again, XOR draw so it can be un-drawn later
                    color(BLACK,WHITE,XOR);
                    line(tool_start_x, tool_start_y, cursor_8u_x, cursor_8u_y);
                }
            }
        }

        drawing_restore_default_colors();
    }
}


static void draw_tool_rect(uint8_t cursor_8u_x, uint8_t cursor_8u_y) {

    if (tool_currently_drawing == false) {

        // Start drawing a rect
        if (KEY_TICKED(DRAW_MAIN_BUTTON)) {
            tool_start_x = cursor_8u_x;
            tool_start_y = cursor_8u_y;
            // Draw the first rect(1 pixel) XOR style so it can be undrawn
            color(BLACK,WHITE,XOR);
            box(tool_start_x, tool_start_y, cursor_8u_x, cursor_8u_y, tool_fillstyle);

            // Set rect starting point
            app_state.draw_tool_using_b_button_action = true;
            tool_currently_drawing = true;
        }

    } else {
        // rect drawing is active, currently previewing position

        // Un-draw the rect from the last frame (XOR)
        // But only if the cursor moved, so that it remains visible otherwise
        bool new_draw_position = false;
        if ((cursor_8u_x != app_state.draw_cursor_8u_last_x) || (cursor_8u_y !=app_state.draw_cursor_8u_last_y)) {
            color(BLACK,WHITE,XOR);
            box(tool_start_x, tool_start_y, app_state.draw_cursor_8u_last_x, app_state.draw_cursor_8u_last_y, tool_fillstyle);
            new_draw_position = true;
        }

        // If finalizing the rect is requested, draw it normally
        if (KEY_TICKED(DRAW_MAIN_BUTTON)) {
            // Finalize the rect
            drawing_restore_default_colors();
            box(tool_start_x, tool_start_y, cursor_8u_x, cursor_8u_y, tool_fillstyle);

            app_state.draw_tool_using_b_button_action = false;
            tool_currently_drawing = false;
        }
        else {
            // Otherwise still being actively drawn

            // B Button cancels drawing the current one
            if (KEY_TICKED(DRAW_CANCEL_BUTTON)) {

                // Cancel all drawing
                tool_currently_drawing = false;
                app_state.draw_tool_using_b_button_action = false;

                // If it's the same position as before then need to undraw to cancel
                if (!new_draw_position) {
                    color(BLACK,WHITE,XOR);
                    box(tool_start_x, tool_start_y, app_state.draw_cursor_8u_last_x, app_state.draw_cursor_8u_last_y, tool_fillstyle);
                }
            }
            else {
                // If moved, update the preview to the new position
                if (new_draw_position) {
                    // Again, XOR draw so it can be un-drawn later
                    color(BLACK,WHITE,XOR);
                    box(tool_start_x, tool_start_y, cursor_8u_x, cursor_8u_y, tool_fillstyle);
                }
            }
        }

        drawing_restore_default_colors();
    }
}


// TODO: could do a 96x96 LUT to get actual line distance
// Quick but inaccurate line length
static uint8_t get_radius(uint8_t cursor_8u_x, uint8_t cursor_8u_y) {
    uint8_t x_dist, y_dist;
    if (tool_start_x > cursor_8u_x) x_dist = (tool_start_x - cursor_8u_x);
    else                            x_dist = (cursor_8u_x - tool_start_x);

    if (tool_start_y > cursor_8u_y) y_dist = (tool_start_y - cursor_8u_y);
    else                            y_dist = (cursor_8u_y - tool_start_y);

    // Return whichever is longer
    uint8_t result = (x_dist > y_dist) ? x_dist : y_dist;
    // Circle drawing doesn't handle radius of zero well
    if (result == 0) result++;

    // Clamp distance to not exceed drawing area
    // There is gating at the start of line drawing to make sure that this shouldn't
    // result in a radius 0, which would crash the circle drawing
    if (result > (tool_start_x - IMG_X_START)) result = (tool_start_x - IMG_X_START);
    if (result > (IMG_X_END - tool_start_x))   result = (IMG_X_END - tool_start_x);
    if (result > (tool_start_y - IMG_Y_START)) result = (tool_start_y - IMG_Y_START);
    if (result > (IMG_Y_END - tool_start_y))   result = (IMG_Y_END - tool_start_y);

    // EMU_printf("xd=%hu, yd=%hu, len=%hu\n", (uint8_t)x_dist, (uint8_t)y_dist, (uint8_t)result);
    return result;
}


static void draw_tool_circle(uint8_t cursor_8u_x, uint8_t cursor_8u_y) {

    if (tool_currently_drawing == false) {

        // Start drawing
        if (KEY_TICKED(DRAW_MAIN_BUTTON)) {

            // Block starting a circle on any edge of the drawing area
            // since radius of 1+ would spill into the UI area
            if ((cursor_8u_x != IMG_X_START) && (cursor_8u_x != IMG_X_END) &&
                (cursor_8u_y != IMG_Y_START) && (cursor_8u_y != IMG_Y_END)) {

                tool_start_x = cursor_8u_x;
                tool_start_y = cursor_8u_y;
                // Draw the first XOR style so it can be undrawn
                color(BLACK,WHITE,XOR);
                circle(tool_start_x, tool_start_y, get_radius(cursor_8u_x, cursor_8u_y), tool_fillstyle);

                // Set starting point
                app_state.draw_tool_using_b_button_action = true;
                tool_currently_drawing = true;
            }
        }

    } else {
        // Drawing is active, currently previewing position

        // Un-draw from the last frame (XOR)
        // But only if the cursor moved, so that it remains visible otherwise
        bool new_draw_position = false;
        if ((cursor_8u_x != app_state.draw_cursor_8u_last_x) || (cursor_8u_y !=app_state.draw_cursor_8u_last_y)) {
            color(BLACK,WHITE,XOR);
            circle(tool_start_x, tool_start_y, get_radius(app_state.draw_cursor_8u_last_x, app_state.draw_cursor_8u_last_y), tool_fillstyle);
            new_draw_position = true;
        }

        // If finalizing is requested, draw it normally
        if (KEY_TICKED(DRAW_MAIN_BUTTON)) {
            // Finalize
            drawing_restore_default_colors();
            circle(tool_start_x, tool_start_y, get_radius(cursor_8u_x, cursor_8u_y), tool_fillstyle);

            app_state.draw_tool_using_b_button_action = false;
            tool_currently_drawing = false;
        }
        else {
            // Otherwise still being actively drawn

            // B Button cancels drawing the current one
            if (KEY_TICKED(DRAW_CANCEL_BUTTON)) {

                // Cancel all drawing
                tool_currently_drawing = false;
                app_state.draw_tool_using_b_button_action = false;

                // If it's the same position as before then need to undraw to cancel
                if (!new_draw_position) {
                    color(BLACK,WHITE,XOR);
                    circle(tool_start_x, tool_start_y, get_radius(app_state.draw_cursor_8u_last_x, app_state.draw_cursor_8u_last_y), tool_fillstyle);
                }
            }
            else {
                // If moved, update the preview to the new position
                if (new_draw_position) {
                    // Again, XOR draw so it can be un-drawn later
                    color(BLACK,WHITE,XOR);
                    circle(tool_start_x, tool_start_y, get_radius(cursor_8u_x, cursor_8u_y), tool_fillstyle);
                }
            }
        }

        drawing_restore_default_colors();
    }
}


static void draw_tool_eraser(uint8_t cursor_8u_x, uint8_t cursor_8u_y) {

    if (KEY_PRESSED(DRAW_MAIN_BUTTON)) {
        uint8_t end_x = cursor_8u_x + (TOOL_ERASER_SIZE - 1u);
        uint8_t end_y = cursor_8u_y + (TOOL_ERASER_SIZE - 1u);

        if (end_x > IMG_X_END) end_x = IMG_X_END;
        if (end_y > IMG_Y_END) end_y = IMG_Y_END;

        color(WHITE,WHITE,SOLID);
        box(cursor_8u_x, cursor_8u_y, end_x, end_y, M_FILL);
    }
    drawing_restore_default_colors();
}


