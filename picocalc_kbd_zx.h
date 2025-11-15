// Glue between PicoCalc keyboard and zx2040 arrow keys

#pragma once

#include <stdbool.h>  // for bool type
#include "pico/stdlib.h"

// Keyboard key definitions
#define PICOCALC_KEY_MOD_ALT         (0xA1)
#define PICOCALC_KEY_MOD_SHL         (0xA2)
#define PICOCALC_KEY_MOD_SHR         (0xA3)
#define PICOCALC_KEY_MOD_SYM         (0xA4)
#define PICOCALC_KEY_MOD_CTRL        (0xA5)

#define PICOCALC_KEY_STATE_IDLE      (0)
#define PICOCALC_KEY_STATE_PRESSED   (1)
#define PICOCALC_KEY_STATE_HOLD      (2)
#define PICOCALC_KEY_STATE_RELEASED  (3)

#define PICOCALC_KEY_BACKSPACE       (0x08)
#define PICOCALC_KEY_TAB             (0x09)
#define PICOCALC_KEY_ENTER           (0x0A)
#define PICOCALC_KEY_RETURN          (0x0D)
#define PICOCALC_KEY_SPACE           (0x20)

#define PICOCALC_KEY_ESC             (0xB1)
#define PICOCALC_KEY_LEFT            (0xB4)
#define PICOCALC_KEY_UP              (0xB5)
#define PICOCALC_KEY_DOWN            (0xB6)
#define PICOCALC_KEY_RIGHT           (0xB7)

#define PICOCALC_KEY_BREAK           (0xD0)
#define PICOCALC_KEY_INSERT          (0xD1)
#define PICOCALC_KEY_HOME            (0xD2)
#define PICOCALC_KEY_DEL             (0xD4)
#define PICOCALC_KEY_END             (0xD5)
#define PICOCALC_KEY_PAGE_UP         (0xD6)
#define PICOCALC_KEY_PAGE_DOWN       (0xD7)

#define PICOCALC_KEY_CAPS_LOCK       (0xC1)

#define PICOCALC_KEY_F1              (0x81)
#define PICOCALC_KEY_F2              (0x82)
#define PICOCALC_KEY_F3              (0x83)
#define PICOCALC_KEY_F4              (0x84)
#define PICOCALC_KEY_F5              (0x85)
#define PICOCALC_KEY_F6              (0x86)
#define PICOCALC_KEY_F7              (0x87)
#define PICOCALC_KEY_F8              (0x88)
#define PICOCALC_KEY_F9              (0x89)
#define PICOCALC_KEY_F10             (0x90)

#define PICOCALC_KEY_POWER           (0x91)


typedef union {
    struct {
        unsigned up : 1;
        unsigned down : 1;
        unsigned left : 1;
        unsigned right : 1;
        unsigned fire : 1;
        unsigned reserved : 3;  // padding to fill one byte
    };
    uint8_t as_byte;  // arrow keys and fire button as a byte
} picocalc_arrow_state_t;

// zx picocalc key status structure
typedef struct {
    uint8_t state;
    uint8_t code;
} zx_key_t;

zx_key_t cur_key_state = {PICOCALC_KEY_STATE_IDLE, 0};

// Original mapping misses some keys from ZX keyboard
static void picocalc_init_kbd_matrix(zx_t* sys) {
    //kbd_register_key(&sys->kbd, ' ', 7, 0, 0);  // Space
    //kbd_register_key(&sys->kbd, 0x0F, 7, 1, 0); // SymShift
    kbd_register_key(&sys->kbd, PICOCALC_KEY_DEL, 4, 0, 1); // Delete (Shift+0)
    kbd_register_key(&sys->kbd, PICOCALC_KEY_F1, 3, 0, 1); // Edit (Shift+1)
    kbd_register_key(&sys->kbd, PICOCALC_KEY_F2, 0, 0, 3); // Extended Mode (Caps Shift + SymShift)
    kbd_register_key(&sys->kbd, PICOCALC_KEY_F3, 3, 4, 1); // Cursor Left (Shift+5)
    kbd_register_key(&sys->kbd, PICOCALC_KEY_F4, 4, 2, 1); // Cursor Right (Shift+8)
    //                          PICOCALC_KEY_F5 = Joystick Fire
    kbd_register_key(&sys->kbd, PICOCALC_KEY_F6, 4, 1, 1); // Graphics (Shift+9)
    kbd_register_key(&sys->kbd, PICOCALC_KEY_F7, 3, 1, 1); // Caps Lock (Shift+2)
    kbd_register_key(&sys->kbd, PICOCALC_KEY_F8, 3, 2, 1); // True Video (Shift+3)
    kbd_register_key(&sys->kbd, PICOCALC_KEY_F9, 3, 3, 1); // Inv Video (Shift+4)
    kbd_register_key(&sys->kbd, PICOCALC_KEY_PAGE_UP, 4, 3, 1); // Cursor Up (Shift+7)
    kbd_register_key(&sys->kbd, PICOCALC_KEY_PAGE_DOWN, 4, 4, 1); // Cursor Down (Shift+6)
    kbd_register_key(&sys->kbd, PICOCALC_KEY_ENTER, 6, 0, 0); // Enter (not Return)
    kbd_register_key(&sys->kbd, PICOCALC_KEY_MOD_CTRL, 0, 0, 1); // Shift (Picocalc Ctrl)
}

// Read PicoCalc I2C keyboard and store it in cur_state variables
void picocalc_read_kbd() {
    static bool is_alt = false;
    // EMU is a global variable defined in zx.c
    uint8_t zx_key_code; // Key code translated to ZX Spectrum map
    
    // Read keyboard from PicoCalc South Bridge
    uint16_t key = sb_read_keyboard();
    uint8_t key_state = (key >> 8) & 0xFF;
    uint8_t key_code = key & 0xFF;

    if(key_state != PICOCALC_KEY_STATE_IDLE) {
        switch(key_code) {
            case PICOCALC_KEY_UP:
                zx_key_code = 0xFB; // Joystick Up
                break;
            case PICOCALC_KEY_DOWN:
                zx_key_code = 0xFC; // Joystick Down
                break;
            case PICOCALC_KEY_RIGHT:
                zx_key_code = 0xFD; // Joystick Right
                break;
            case PICOCALC_KEY_LEFT:
                zx_key_code = 0xFE; // Joystick Left
                break;
            case PICOCALC_KEY_F5:
                zx_key_code = 0xFF; // Joystick Fire
                break;
            case PICOCALC_KEY_MOD_ALT: // Use Alt for ZX Symbol Shift
                zx_key_code = 0x0F; // ZX SymShift
                is_alt = key_state==PICOCALC_KEY_STATE_RELEASED?false:true; // Store Alt status to fix Picocalc Alt problem
                break;
            case PICOCALC_KEY_BACKSPACE:
                zx_key_code = PICOCALC_KEY_DEL; // ZX Delete also under Backspace
                break;
            case PICOCALC_KEY_TAB:
                zx_key_code = PICOCALC_KEY_F2; // Extended Mode (Caps Shift + SymShift)
                break;
            case '~':
                zx_key_code = 'A'; // Extended Mode needs to be entered first
                break;
            case '\\':
                zx_key_code = 'D'; // Extended Mode needs to be entered first
                break;
            case '|':
                zx_key_code = 'S'; // Extended Mode needs to be entered first
                break;
            case '[':
                zx_key_code = 'Y'; // Extended Mode needs to be entered first
                break;
            case ']':
                zx_key_code = 'U'; // Extended Mode needs to be entered first
                break;
            case '{':
                zx_key_code = 'F'; // Extended Mode needs to be entered first
                break;
            case '}':
                zx_key_code = 'G'; // Extended Mode needs to be entered first
                break;
            default:
                if(key_code >= 'A' && key_code <= 'Z') {
                    // Fix Picocalc Alt problem (working as CAPS)
                    if(is_alt)
                        zx_key_code = key_code + 0x20; // Convert to lowercase
                    else
                        zx_key_code = key_code;
                } else if((key_code >= 0x0A && key_code <= 0x90) || // ASCII codes and F1-F10
                           key_code == PICOCALC_KEY_PAGE_UP || 
                           key_code == PICOCALC_KEY_PAGE_DOWN ||
                           key_code == PICOCALC_KEY_MOD_CTRL ||
                           key_code == PICOCALC_KEY_ESC ||
                           key_code == PICOCALC_KEY_DEL
                         ) { 
                    // Mapped directly
                    zx_key_code = key_code;
                } else {
                    // Other keys: ignore
                    zx_key_code = 0;
                    key_state = PICOCALC_KEY_STATE_IDLE;
                }
                break;
        }
        //printf("At frame %lu. key_state=%X, key_code=%02X => %02X\n", EMU.tick, key_state, key_code, zx_key_code);
        cur_key_state.state = key_state;
        cur_key_state.code = zx_key_code;
    } else {
        //printf("At frame %lu.\r", EMU.tick);
    }
}

// Handle ZX joystick key presses from PicoCalc GPIOs
void picocalc_handle_zx_joystick_press(zx_t *zx) {
    static picocalc_arrow_state_t last_arrow_state = {0};

    if(!get_device_button(KEY_UP) == last_arrow_state.up) {
        if(last_arrow_state.up = !last_arrow_state.up)
            zx_key_down(zx, 0xFB); // Joystick Up
        else
            zx_key_up(zx, 0xFB);
    }
    if(!get_device_button(KEY_DOWN) == last_arrow_state.down) {
        if(last_arrow_state.down = !last_arrow_state.down)
            zx_key_down(zx, 0xFC); // Joystick Down
        else
            zx_key_up(zx, 0xFC);
    }
    if(!get_device_button(KEY_RIGHT) == last_arrow_state.right) {
        if(last_arrow_state.right = !last_arrow_state.right)
            zx_key_down(zx, 0xFD); // Joystick Right
        else
            zx_key_up(zx, 0xFD);
    }
    if(!get_device_button(KEY_LEFT) == last_arrow_state.left) {
        if(last_arrow_state.left = !last_arrow_state.left)
            zx_key_down(zx, 0xFE); // Joystick Left
        else
            zx_key_up(zx, 0xFE);
    }
    if(!get_device_button(KEY_FIRE) == last_arrow_state.fire) {
        if(last_arrow_state.fire = !last_arrow_state.fire)
            zx_key_down(zx, 0xFF); // Joystick Fire
        else
            zx_key_up(zx, 0xFF);
    }
}

void picocalc_handle_zx_key_press(zx_t *zx) {
    switch (cur_key_state.state) {
        case PICOCALC_KEY_STATE_PRESSED: // Fall through
            if(cur_key_state.code == PICOCALC_KEY_ESC && EMU.tick > EMU.menu_left_at_tick+10) { // ESC key to enter menu
                EMU.menu_active = 1; // Show menu
                vram_force_dirty(); // Force screen redraw
                break;
            }
        case PICOCALC_KEY_STATE_HOLD:
            zx_key_down(zx, cur_key_state.code);
            break;
        case PICOCALC_KEY_STATE_RELEASED:
            zx_key_up(zx, cur_key_state.code);
            break;
    }

    // Reset current key state after processing
    cur_key_state.state = PICOCALC_KEY_STATE_IDLE;
}
