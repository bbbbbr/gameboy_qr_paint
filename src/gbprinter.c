#include <gbdk/platform.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "gbprinter.h"
#include "common.h"

#pragma bank 255  // Autobanked

#define REINIT_SEIKO

#define START_TRANSFER 0x81
#if (CGB_FAST_TRANSFER==1)
    // 0b10000011 - start, CGB double speed, internal clock
    #define START_TRANSFER_FAST 0x83
#else
    // 0b10000001 - start, internal clock
    #define START_TRANSFER_FAST 0x81
#endif
#define PRN_BUSY_TIMEOUT        PRN_SECONDS(2)
#define PRN_COMPLETION_TIMEOUT  PRN_SECONDS(20)
#define PRN_SEIKO_RESET_TIMEOUT 10
#define PRN_FULL_TIMEOUT        PRN_SECONDS(2)

#define PRN_FINAL_MARGIN        0x03
#define PRN_STATUS_MASK_ERRORS_AND_SUM (PRN_STATUS_MASK_ERRORS | PRN_STATUS_SUM)

static const uint8_t PRN_PKT_INIT[]    = { PRN_LE(PRN_MAGIC), PRN_LE(PRN_CMD_INIT),   PRN_LE(0), PRN_LE(0x01), PRN_LE(0) };
static const uint8_t PRN_PKT_STATUS[]  = { PRN_LE(PRN_MAGIC), PRN_LE(PRN_CMD_STATUS), PRN_LE(0), PRN_LE(0x0F), PRN_LE(0) };
static const uint8_t PRN_PKT_EOF[]     = { PRN_LE(PRN_MAGIC), PRN_LE(PRN_CMD_DATA),   PRN_LE(0), PRN_LE(0x04), PRN_LE(0) };
static const uint8_t PRN_PKT_CANCEL[]  = { PRN_LE(PRN_MAGIC), PRN_LE(PRN_CMD_BREAK),  PRN_LE(0), PRN_LE(0x01), PRN_LE(0) };

start_print_pkt_t PRN_PKT_START = {
    .magic = PRN_MAGIC, .command = PRN_CMD_PRINT, .length = 4,
    .print = TRUE, .margins = 0, .palette = PRN_PALETTE_NORMAL, .exposure = PRN_EXPOSURE_DARK,
    .crc = 0, .trail = 0
};

static uint16_t printer_status;
static uint8_t printer_tile_num;

uint8_t printer_completion = 0;
static uint8_t printer_send_receive(uint8_t b) {
    SB_REG = b;
    SC_REG = 0x81;
    while (SC_REG & 0x80);
    return SB_REG;
}

static uint8_t printer_send_byte(uint8_t b) {
    return (uint8_t)(printer_status = ((printer_status << 8) | printer_send_receive(b)));
}

static uint8_t printer_send_command(const uint8_t *command, uint8_t length) {
    uint8_t index = 0;
    while (index++ < length) printer_send_byte(*command++);
    return ((uint8_t)(printer_status >> 8) == PRN_MAGIC_DETECT) ? (uint8_t)printer_status : PRN_STATUS_MASK_ERRORS;
}
#define PRINTER_SEND_COMMAND(CMD) printer_send_command((const uint8_t *)&(CMD), sizeof(CMD))

static uint8_t printer_print_tile(const uint8_t *tiledata) {
    static const uint8_t PRINT_TILE[] = { 0x88,0x33,0x04,0x00,0x80,0x02 };
    static uint16_t printer_CRC;
    if (printer_tile_num == 0) {
        const uint8_t * data = PRINT_TILE;
        for (uint8_t i = sizeof(PRINT_TILE); i != 0; i--) printer_send_receive(*data++);
        printer_CRC = 0x04 + 0x80 + 0x02;
    }
    for(uint8_t i = 0x10; i != 0; i--, tiledata++) {
        printer_CRC += *tiledata;
        printer_send_receive(*tiledata);
    }
    if (++printer_tile_num == 40) {
        printer_send_receive((uint8_t)printer_CRC);
        printer_send_receive((uint8_t)(printer_CRC >> 8));
        printer_send_receive(0x00);
        printer_send_receive(0x00);
        printer_CRC = printer_tile_num = 0;
        return TRUE;
    }
    return FALSE;
}

inline void printer_init(void) {
    printer_tile_num = 0;
    PRINTER_SEND_COMMAND(PRN_PKT_INIT);
}

extern bool printer_check_cancel(void) BANKED;

static uint8_t printer_wait(uint16_t timeout, uint8_t mask, uint8_t value) {
    uint8_t error;
    while (((error = PRINTER_SEND_COMMAND(PRN_PKT_STATUS)) & mask) != value) {
        if (printer_check_cancel()) {
            PRINTER_SEND_COMMAND(PRN_PKT_CANCEL);
            return PRN_STATUS_CANCELLED;
        }
        if (timeout-- == 0) return PRN_STATUS_MASK_ERRORS;
        if (error & PRN_STATUS_MASK_ERRORS_AND_SUM) break;
        vsync();
    }
    return error;
}

uint8_t gbprinter_detect(uint8_t delay) BANKED {
    printer_init();
    return printer_wait(delay, PRN_STATUS_MASK_ANY, PRN_STATUS_OK);
}


#define TILE_BANK_0 _VRAM8800
#define TILE_BANK_1 _VRAM8000

#define TILE_BYTES_SZ              (16u)
#define APA_TILE_SRC_TOGGLE_TILE_Y (72u / 8u)
#define APA_TILE_NUM_UPPER_START   (128u)
#define PRN_TILE_CACHE_MAX_TILES   (IMG_WIDTH_TILES * IMG_HEIGHT_TILES)

// Cached tile data for the draw canvas (12x12 tiles).
// Capture first, then print from this stable buffer.
static uint8_t printer_tile_cache[PRN_TILE_CACHE_MAX_TILES * TILE_BYTES_SZ];

static uint8_t capture_screen_rect_tiles_apa(uint8_t sx, uint8_t sy, uint8_t sw, uint8_t sh) {
    const uint16_t tile_count = (uint16_t)sw * (uint16_t)sh;
    if (tile_count > PRN_TILE_CACHE_MAX_TILES) return PRN_STATUS_MASK_ERRORS;

    uint8_t * p_out = printer_tile_cache;

    DISPLAY_OFF;
    for (uint8_t y = 0; y != sh; y++) {
        uint8_t * map_addr = get_bkg_xy_addr(sx, y + sy);
        for (uint8_t x = 0; x != sw; x++) {
            uint8_t tile = get_vram_byte(map_addr++);
            uint8_t * source = (((y + sy) >= APA_TILE_SRC_TOGGLE_TILE_Y) && (tile < APA_TILE_NUM_UPPER_START)) ? _VRAM9000 : _VRAM8000;
            vmemcpy(p_out, source + ((uint16_t)tile << 4), TILE_BYTES_SZ);
            p_out += TILE_BYTES_SZ;
        }
    }
    DISPLAY_ON;

    return PRN_STATUS_OK;
}

// Prints the requested tile region of the screen in APA mode, tiles outside the screen are printed WHITE
uint8_t gbprinter_print_screen_rect(uint8_t sx, uint8_t sy, uint8_t sw, uint8_t sh, uint8_t centered) BANKED {
    static uint8_t error;

    // call printer progress: zero progress
    printer_completion = 0; // call_far(&printer_progress_handler);

    uint8_t tile_data[0x10], rows = ((sh & 0x01) ? (sh + 1) : sh), pkt_count = 0, x_ofs = (centered) ? ((PRN_TILE_WIDTH - sw) >> 1) : 0;

    printer_tile_num = 0;

    if ((sw == 0u) || (sh == 0u)) return PRN_STATUS_OK;

    // Capture the requested tile rectangle up front to avoid VRAM reads
    // during timing-sensitive serial printer transfers.
    if ((error = capture_screen_rect_tiles_apa(sx, sy, sw, sh)) != PRN_STATUS_OK) return error;

    for (uint8_t y = 0; y != rows; y++) {
        for (uint8_t x = 0; x != PRN_TILE_WIDTH; x++) {
            if ((x >= x_ofs) && (x < (x_ofs + sw)) && (y < sh))  {
                uint16_t tile_index = ((uint16_t)y * (uint16_t)sw) + ((uint16_t)x - (uint16_t)x_ofs);
                memcpy(tile_data, printer_tile_cache + (tile_index * TILE_BYTES_SZ), sizeof(tile_data));
            } else memset(tile_data, 0x00, sizeof(tile_data));
            if (printer_print_tile(tile_data)) {
                pkt_count++;
                if (printer_check_cancel()) {
                    PRINTER_SEND_COMMAND(PRN_PKT_CANCEL);
                    return PRN_STATUS_CANCELLED;
                }
            }
            if (pkt_count == 9) {
                pkt_count = 0;
                if ((error = PRINTER_SEND_COMMAND(PRN_PKT_EOF)) & PRN_STATUS_MASK_ERRORS_AND_SUM) return error;
                if ((error = printer_wait(PRN_FULL_TIMEOUT, PRN_STATUS_FULL, PRN_STATUS_FULL)) & PRN_STATUS_MASK_ERRORS_AND_SUM) return error;
                gbprinter_set_print_params((y == (rows - 1)) ? PRN_FINAL_MARGIN : PRN_NO_MARGINS, PRN_PALETTE_NORMAL, PRN_EXPOSURE_DARK);
                if ((error = PRINTER_SEND_COMMAND(PRN_PKT_START)) & PRN_STATUS_MASK_ERRORS_AND_SUM) return error;
                // query printer status
                if ((error = printer_wait(PRN_BUSY_TIMEOUT, PRN_STATUS_BUSY, PRN_STATUS_BUSY)) & PRN_STATUS_MASK_ERRORS_AND_SUM) return error;
                if ((error = printer_wait(PRN_COMPLETION_TIMEOUT, PRN_STATUS_BUSY, 0)) & PRN_STATUS_MASK_ERRORS_AND_SUM) return error;
#ifdef REINIT_SEIKO
                // reinit printer (required by Seiko?)
                if (y != (rows - 1)) {
                    PRINTER_SEND_COMMAND(PRN_PKT_INIT);
                    if ((error = printer_wait(PRN_SEIKO_RESET_TIMEOUT, PRN_STATUS_MASK_ANY, PRN_STATUS_OK)) & PRN_STATUS_MASK_ERRORS_AND_SUM) return error;
                }
#endif
                // call printer progress callback
                uint8_t current_progress = (((uint16_t)y * PRN_MAX_PROGRESS) / rows);
                if (printer_completion != current_progress) {
                    printer_completion = current_progress; //, call_far(&printer_progress_handler);
                }
            }
        }
    }
    if (pkt_count) {
        if ((error = PRINTER_SEND_COMMAND(PRN_PKT_EOF)) & PRN_STATUS_MASK_ERRORS_AND_SUM) return error;
        if ((error = printer_wait(PRN_FULL_TIMEOUT, PRN_STATUS_FULL, PRN_STATUS_FULL)) & PRN_STATUS_MASK_ERRORS_AND_SUM) return error;
        // setup printing if required

        gbprinter_set_print_params(PRN_FINAL_MARGIN, PRN_PALETTE_NORMAL, PRN_EXPOSURE_DARK);
        if ((error = PRINTER_SEND_COMMAND(PRN_PKT_START)) & PRN_STATUS_MASK_ERRORS_AND_SUM) return error;
        // query printer status
        if ((error = printer_wait(PRN_BUSY_TIMEOUT, PRN_STATUS_BUSY, PRN_STATUS_BUSY)) & PRN_STATUS_MASK_ERRORS_AND_SUM) return error;
        if ((error = printer_wait(PRN_COMPLETION_TIMEOUT, PRN_STATUS_BUSY, 0)) & PRN_STATUS_MASK_ERRORS_AND_SUM) return error;
        // indicate 100% completion
        printer_completion = PRN_MAX_PROGRESS; //, call_far(&printer_progress_handler);
    }
    return PRINTER_SEND_COMMAND(PRN_PKT_STATUS);
}
