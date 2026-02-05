// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later
// TODO: enable/disable slow movement for games which only support one movement speed.

// If `_DEBUG` is defined, instead of sending movement keycodes, strings are sent to `qmk console`.
// #define _DEBUG

#include QMK_KEYBOARD_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mouse_2_movement.h"

enum custom_keycodes {
    GM_TOGGLE,
};

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
    [0] = LAYOUT(GM_TOGGLE)};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case GM_TOGGLE:
            if (record->event.pressed) game_mode = !game_mode;
            break;
        default:
            break;
    }

    return true;
}

