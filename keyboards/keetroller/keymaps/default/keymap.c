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

// Aliases for readability
#define QWERTY   DF(_QWERTY)

enum custom_keycodes {
    GM_TOGGLE,
};

enum layers {
    _QWERTY = 0,
    _G_MODE,
    _NAV,
    _SYM,
    _FUNCTION,
    _ADJUST,
};

// LT(layer, kc) - momentarily activates layer when held, and sends kc when tapped.
#define SYM_ENT  LT(_SYM, KC_ENT) 
#define NAV_SPC  LT(_NAV, KC_SPC)
#define FK_ESC   LT(_FUNCTION, KC_ESC)
#define ADJ_TAB  LT(_ADJUST, KC_TAB)

#define CTL_QUOT MT(MOD_RCTL, KC_QUOTE)
#define CAPS_CTL MT(MOD_LCTL, KC_CAPS)


// Note: LAlt/Enter (ALT_ENT) is not the same thing as the keyboard shortcut Alt+Enter.
// The notation `mod/tap` denotes a key that activates the modifier `mod` when held down, and
// produces the key `tap` when tapped (i.e. pressed and released).

// clang-format off

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {    
    // Base Layer: QWERTY
    [_QWERTY] = LAYOUT(
     FK_ESC  , KC_1 ,  KC_2   ,  KC_3  ,   KC_4 ,   KC_5 ,       KC_6  ,   KC_7  ,  KC_8 ,   KC_9 ,  KC_0  ,  KC_DEL,
     ADJ_TAB , KC_Q ,  KC_W   ,  KC_E  ,   KC_R ,   KC_T ,       KC_Y  ,   KC_U  ,  KC_I ,   KC_O ,  KC_P  ,  KC_BSPC,
     CAPS_CTL, KC_A ,  KC_S   ,  KC_D  ,   KC_F ,   KC_G ,       KC_H  ,   KC_J  ,  KC_K ,   KC_L ,  KC_SCLN, CTL_QUOT,
     KC_LSFT , KC_Z ,  KC_X   ,  KC_C  ,   KC_V ,   KC_B ,       KC_N  ,   KC_M  ,  KC_COMM, KC_DOT, KC_SLSH, KC_RSFT,
                                 KC_LGUI,  KC_LALT, NAV_SPC,     GM_TOGGLE,KC_MPLY, SYM_ENT, KC_SPC, KC_RALT, KC_RGUI
    ),

    // Game Mode: no qmk magic
    [_G_MODE] = LAYOUT(
     KC_ESC  , KC_1 ,  KC_2   ,  KC_3  ,   KC_4 ,   KC_5 ,       KC_6  ,   KC_7  ,  KC_8 ,   KC_9 ,  KC_0  ,  KC_DEL,
     KC_TAB  , KC_Q ,  KC_W   ,  KC_E  ,   KC_R ,   KC_T ,       KC_Y  ,   KC_U  ,  KC_I ,   KC_O ,  KC_P  ,  KC_BSPC,
     KC_LCTL , KC_A ,  KC_S   ,  KC_D  ,   KC_F ,   KC_G ,       KC_H  ,   KC_J  ,  KC_K ,   KC_L ,  KC_SCLN, KC_QUOT,
     KC_LSFT , KC_Z ,  KC_X   ,  KC_C  ,   KC_V ,   KC_B ,       KC_N  ,   KC_M  ,  KC_COMM, KC_DOT, KC_SLSH, KC_RSFT,
                                 KC_LGUI,  KC_LALT, KC_SPC,     GM_TOGGLE, KC_MPLY, KC_ENT,  KC_SPC, KC_RALT, KC_RGUI
    ),

    //Nav Layer: Media, navigation
    [_NAV] = LAYOUT(
     _______, _______, _______, _______, _______, _______,       _______, _______, _______, _______, _______, _______,
     _______, _______, _______, _______, _______, _______,       KC_PGUP, KC_HOME, KC_UP,   KC_END,  KC_VOLU, KC_DEL,
     _______, KC_LGUI, KC_LALT, KC_LCTL, KC_LSFT, _______,       KC_PGDN, KC_LEFT, KC_DOWN, KC_RGHT, KC_VOLD, KC_INS,
     _______, _______, _______, _______, _______, _______,       KC_PAUSE, KC_MPRV, KC_MPLY, KC_MNXT, KC_MUTE, KC_PSCR,
                                _______, _______, _______,       _______, _______, _______, _______, _______, _______
    ),

    // Sym Layer: Numbers and symbols
    [_SYM] = LAYOUT(
     _______, _______, _______, _______, _______, _______,       _______, _______, _______, _______, _______, _______,
     KC_GRV ,   KC_1 ,   KC_2 ,   KC_3 ,   KC_4 ,   KC_5 ,       KC_6 ,   KC_7 ,   KC_8 ,   KC_9 ,   KC_0 , KC_EQL ,
     KC_TILD , KC_EXLM,  KC_AT , KC_HASH,  KC_DLR, KC_PERC,      KC_CIRC, KC_AMPR, KC_ASTR, KC_LPRN, KC_RPRN, KC_PLUS,
     KC_PIPE , KC_BSLS, KC_COLN, KC_SCLN, KC_MINS, KC_LBRC,      KC_RBRC, KC_UNDS, KC_COMM,  KC_DOT, KC_SLSH, _______,
                                _______, _______, _______,       _______, _______, _______, _______, _______, _______
    ),


    // Function Layer: Function keys
    [_FUNCTION] = LAYOUT(
     _______, _______, _______, _______, _______, _______,       _______, _______, _______, _______, _______, _______,
     _______, _______, _______, _______, _______, _______,       _______, KC_F9  ,  KC_F10,  KC_F11,  KC_F12, _______,
     _______, _______, _______, _______, _______, _______,       _______, KC_F5  ,  KC_F6 ,  KC_F7 ,  KC_F8 , _______,
     _______, _______, _______, _______, _______, _______,       _______, KC_F1  ,  KC_F2 ,  KC_F3 ,  KC_F4 , _______,
                                _______, _______, _______,       _______, _______, _______, _______, _______, _______
    ),
    // Adjust Layer: Default layer settings, RGB
    [_ADJUST] = LAYOUT(
     _______, _______, _______, _______, _______, _______,       _______, _______, _______, _______, _______, _______,
     _______, _______, _______, _______, _______, _______,       _______, _______, _______, _______, _______, _______,
     _______, _______, _______, _______, _______, _______,       RM_TOGG, RM_SATU, RM_HUEU, RM_VALU, RM_NEXT, _______,
     _______, _______, _______, _______, _______, _______,       _______, RM_SATD, RM_HUED, RM_VALD, RM_PREV, _______,
                                _______, _______, _______,       _______, _______, _______, _______, _______, _______
    ),
// /*
//  * Layer template
//     [_LAYERINDEX] = LAYOUT(
//     _______, _______, _______, _______, _______, _______,       _______, _______, _______, _______, _______, _______,
//     _______, _______, _______, _______, _______, _______,       _______, _______, _______, _______, _______, _______,
//     _______, _______, _______, _______, _______, _______,       _______, _______, _______, _______, _______, _______,
//     _______, _______, _______, _______, _______, _______,       _______, _______, _______, _______, _______, _______,
//                                _______, _______, _______,       _______, _______, _______, _______, _______, _______,
//     ),
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

// @todo: get rid of tapdacne, just cycle through qwerty, game mode 1 & game mode 2
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

