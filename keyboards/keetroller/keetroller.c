/* License header
 *
 */

#include QMK_KEYBOARD_H
#include "mouse_2_movement.h"

bool game_mode             = false;
bool slow_movement_enabled = true;

#ifdef OLED_ENABLE
static void render_logo(void) {
    // clang-format off
    static const char PROGMEM qmk_logo[] = {
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93, 0x94,
        0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4,
        0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0x00
    };
    // clang-format on

    oled_write_P(qmk_logo, false);
}

bool oled_task_kb(void) {
    render_logo();
    //oled_write_P(PSTR("Game Mode "), false);
    oled_write_P(game_mode ? PSTR("Game Mode") : PSTR("Boring Mode"), false);
    if (game_mode) {
    oled_write_P(slow_movement_enabled ? PSTR(" * SLW EN") : PSTR(" * SLW DA"), false);
    } else {
        oled_write_ln_P(PSTR(""), false);
    }
    oled_invert(game_mode);

    return false;
}
#endif

report_mouse_t pointing_device_task_kb(report_mouse_t mouse_report) {
    move_player_character(mouse_report, game_mode);

    return mouse_report;
}

bool pointing_device_send(void) {
    static report_mouse_t old_report         = {};
    report_mouse_t        current_report     = pointing_device_get_report();
    bool                  should_send_report = has_mouse_report_changed(&current_report, &old_report) && !game_mode;

    if (should_send_report) {
        host_mouse_send(&current_report);
    }
    // send it and 0 it out except for buttons, so those stay until they are explicitly over-ridden using update_pointing_device
    uint8_t buttons = current_report.buttons;
    if (!game_mode) {
        memset(&current_report, 0, sizeof(current_report));
        current_report.buttons = buttons;
        memcpy(&old_report, &current_report, sizeof(current_report));
        pointing_device_set_report(current_report);
    }

    return should_send_report || buttons;
}