// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

enum custom_keycodes {
    GM_TOGGLE,
};

bool game_mode = false;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /*
     * ┌───┬───┬───┬───┐
     * │ 7 │ 8 │ 9 │ / │
     * ├───┼───┼───┼───┤
     * │ 4 │ 5 │ 6 │ * │
     * ├───┼───┼───┼───┤
     * │ 1 │ 2 │ 3 │ - │
     * ├───┼───┼───┼───┤
     * │ 0 │ . │Ent│ + │
     * └───┴───┴───┴───┘
     */
    [0] = LAYOUT(
        GM_TOGGLE 
    )
};

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

bool oled_task_user(void) {
    render_logo();
    oled_write_P(PSTR("Game Mode "), false);
    oled_write_ln_P(game_mode ? PSTR("ON") : PSTR("OFF"), false);

    oled_invert(game_mode);


    return false;
}
#endif

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case GM_TOGGLE:
            if (record->event.pressed)
                game_mode = !game_mode;
            break;
        default:
            break;
    }

    return true;
}


report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    if (game_mode){
        if (mouse_report.x >= 2) {
            SEND_STRING("L");
        }
        if (mouse_report.x <= -2) {
            SEND_STRING("J");
        }

        if (mouse_report.y >= 2) {
            SEND_STRING("I");
        }
        if (mouse_report.y <= -2) {
            SEND_STRING("K");
        }
        mouse_report.x = 0;
        mouse_report.y = 0;
    }

    return mouse_report;
}


/*
bool pointing_device_task(void) {
    report_mouse_t mouse_report = pointing_device_get_report();

    if (game_mode){
        if (mouse_report.x >= 10) {
            SEND_STRING("L");
        }
        if (mouse_report.x <= -10) {
            SEND_STRING("J");
        }

        if (mouse_report.y >= 10) {
            SEND_STRING("I");
        }
        if (mouse_report.y <= -10) {
            SEND_STRING("K");
        }
        return false;
    }

    pointing_device_set_report(mouse_report);

    return pointing_device_send();
}
    */