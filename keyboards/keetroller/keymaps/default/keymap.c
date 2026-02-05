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
    [0] = LAYOUT(TD(GM_TOGGLE))
};

void toggle_game_mode(tap_dance_state_t *state, void *user_data) {
    switch (state->count) {
        case 1:
            game_mode = !game_mode;
            break;
        case 2:
            if (game_mode) { slow_movement_enabled = !slow_movement_enabled; }
            break;
    }
    reset_tap_dance(state);
}

// Tap Dance definitions
tap_dance_action_t tap_dance_actions[] = {
    [GM_TOGGLE] = ACTION_TAP_DANCE_FN(toggle_game_mode),
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        default:
            break;
    }

    return true;
}

