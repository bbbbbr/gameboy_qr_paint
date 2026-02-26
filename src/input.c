#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>

#include "input.h"
#include "sgb_mouse_on_gb.h"

uint8_t keys = 0x00;
uint8_t previous_keys = 0x00;
uint8_t key_repeat_count = 0x00;
uint8_t key_repeat_count_last = 0x00;
joypads_t joypads;
bool sgb_found = false;
bool sgb_mouse_input_is_valid = false;

void UPDATE_KEYS(void) {
    previous_keys = keys;
    if (sgb_found) {
        joypad_ex(&joypads);
        keys = joypads.joy0;

        sgb_mouse_input_is_valid = sgb_mouse_input_update();
        if (sgb_mouse_input_is_valid) {

            // Merge (OR) mouse buttons into main button state
            if (mouse_buttons & SNES_MOUSE_BUTTON_LEFT)  keys |= J_A;
            if (mouse_buttons & SNES_MOUSE_BUTTON_RIGHT) keys |= J_B;
        }
    }
    else {
        keys = joypad();
    }
}


// Reduce CPU usage by only checking once per frame
void waitpadticked_lowcpu(uint8_t button_mask) {

    while (1) {

        vsync(); // yield CPU
        UPDATE_KEYS();
        if (KEY_TICKED(button_mask))
            break;
    };

    // Prevent passing through any key ticked
    // event that may have just happened
    UPDATE_KEYS();
}

// Reduce CPU usage by only checking once per frame
void waitpadup_lowcpu(uint8_t button_mask) {

    while (1) {

        vsync(); // yield CPU
        UPDATE_KEYS();
        if (!(KEY_PRESSED(button_mask)))
            break;
    };

    // Prevent passing through any key ticked
    // event that may have just happened
    UPDATE_KEYS();
}
