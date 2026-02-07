#include <gbdk/platform.h>
#include <stdint.h>

#include "common.h"

const unsigned char mouse_cursors[] = {
    // Classic Mouse style arrow pointer
    0x80, 0x80, 0xC0, 0xC0, 0xE0, 0xA0, 0xF0, 0x90,
    0xF8, 0x88, 0xF0, 0xB0, 0xD8, 0xD8, 0x08, 0x08,

    // ▒▒▒▒▒▒▒▒████████
    // ▒▒▓▓▓▓▒▒████████
    // ▒▒▓▓▓▓▒▒████████
    // ▒▒▒▒▒▒▒▒████████
    // ████████████████
    // ████████████████
    // ████████████████
    // ████████████████
    0X00, 0XF0, 0X60, 0X90, 0X60, 0X90, 0X00, 0XF0,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,

    // // Eraser square
    // // ░░░░░░░░████████
    // // ░░▓▓▓▓░░████████
    // // ░░▓▓▓▓░░████████
    // // ░░░░░░░░████████
    // // ████████████████
    // // ████████████████
    // // ████████████████
    // // ████████████████
    // 0XF0, 0XF0, 0XF0, 0X90, 0XF0, 0X90, 0XF0, 0XF0,
    // 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
  
    // Hand
    0x10, 0x10, 0x38, 0x28, 0x38, 0x28, 0x7E, 0x6E,
    0xFE, 0xA2, 0xFE, 0x82, 0x7E, 0x42, 0x3E, 0x3E
};


uint8_t mouse_cursors_count = ARRAY_LEN(mouse_cursors) / TILE_SZ_BYTES;