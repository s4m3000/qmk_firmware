// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include <stdio.h>
#include <stdlib.h>

// Aliases
#define KC_F_RIGHT KC_L
#define KC_F_LEFT KC_J
#define KC_F_UP KC_I
#define KC_F_DOWN KC_K
#define KC_S_RIGHT RCTL(KC_F_RIGHT)
#define KC_S_LEFT RCTL(KC_F_LEFT)
#define KC_S_UP RCTL(KC_F_UP)
#define KC_S_DOWN RCTL(KC_F_DOWN)

enum custom_keycodes {
    GM_TOGGLE,
};

bool _game_mode = false;

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

// TEST:
static void print_mouse_report(void) {
    return;
    char buffer[12];
    report_mouse_t mouse_report = pointing_device_get_report();

    sprintf(buffer, "x val: %d", mouse_report.x);
    oled_write_ln_P(buffer, false);

    sprintf(buffer, "y val: %d", mouse_report.y);
    oled_write_ln_P(buffer, false);
}

bool oled_task_user(void) {
    render_logo();
    oled_write_P(PSTR("Game Mode "), false);
    oled_write_ln_P(_game_mode ? PSTR("ON") : PSTR("OFF"), false);

    print_mouse_report();

    oled_invert(_game_mode);

    return false;
}
#endif

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case GM_TOGGLE:
            if (record->event.pressed) _game_mode = !_game_mode;
            break;
        default:
            break;
    }

    return true;
}

enum movement_speed_t {
    NO_MVT,
    SLOW,
    FAST
};

static enum movement_speed_t calculate_movement_speed(const int axis_value) {
    const uint8_t min_threshold = 2;
    const uint8_t range = 15;

    int axis = abs(axis_value);
    
    if (axis < min_threshold) {
        return NO_MVT;
    }

    // TEST:
    return FAST;

    if (axis < min_threshold + range) {
        return SLOW;
    } else {
        return FAST;
    }
}

enum movement_direction_t {
    POSITIVE,
    NEGATIVE
};

struct movement_t {
    enum movement_speed_t speed;
    enum movement_direction_t direction;
};

static struct movement_t calculate_movement(const int axis_value) {
    struct movement_t movement;
    
    movement.speed = calculate_movement_speed(axis_value);
    movement.direction = axis_value < 0 ? NEGATIVE : POSITIVE;

    return movement;
}

enum registered_x_kc_t {
    X_NONE,
    RIGHT_F_KC = KC_F_RIGHT,
    LEFT_F_KC = KC_F_LEFT,
    RIGHT_S_KC,
    LEFT_S_KC
};

enum registered_y_kc_t {
    Y_NONE,
    UP_F_KC = KC_F_UP,
    DOWN_F_KC = KC_F_DOWN,
    UP_S_KC,
    DOWN_S_KC,
};

void register_x_movement_kc(const struct movement_t x_movement) {
    static enum registered_x_kc_t registered_x_kc = X_NONE;

    switch (x_movement.speed) {
        case SLOW:
            if(registered_x_kc == RIGHT_F_KC) {
                unregister_code(KC_F_RIGHT);
            } else if (registered_x_kc == LEFT_F_KC) {
                unregister_code(KC_F_LEFT);
            }

            // Move Right
            if (x_movement.direction == POSITIVE) {
                if (registered_x_kc == LEFT_S_KC)
                    unregister_code16(KC_S_LEFT);
                register_code16(KC_S_RIGHT);
                registered_x_kc = RIGHT_S_KC;
            }
            // Move Left
            else {
                if (registered_x_kc == RIGHT_S_KC)
                    unregister_code16(KC_S_RIGHT);
                register_code16(KC_S_LEFT);
                registered_x_kc = LEFT_S_KC;
            }

        case FAST:
            if(registered_x_kc == RIGHT_S_KC) {
                unregister_code16(KC_S_RIGHT);
            } else if (registered_x_kc == LEFT_S_KC) {
                unregister_code16(KC_S_LEFT);
            }

            // Move Right
            if (x_movement.direction == POSITIVE) {
                if (registered_x_kc == LEFT_F_KC)
                    unregister_code(KC_F_LEFT);
                register_code(KC_F_RIGHT);
                registered_x_kc = KC_F_RIGHT;
            }
            // Move Left 
            else {
                if (registered_x_kc == RIGHT_F_KC)
                    unregister_code(KC_F_RIGHT);
                register_code(KC_F_LEFT);
                registered_x_kc = KC_F_LEFT;
            }
            break;
        // No X Movement
        case NO_MVT:
            if (registered_x_kc == RIGHT_F_KC) {
                unregister_code(KC_F_RIGHT);
            } else if (registered_x_kc == LEFT_F_KC) {
                unregister_code(KC_F_LEFT);
            } else if (registered_x_kc == RIGHT_S_KC) {
                unregister_code16(KC_S_RIGHT);
            } else if (registered_x_kc == LEFT_S_KC) {
                unregister_code16(KC_S_LEFT);
            }

            registered_x_kc = X_NONE;
            break;
    }
}

void register_y_movement_kc(const struct movement_t y_movement) {
    static enum registered_y_kc_t registered_y_kc = Y_NONE;

    switch (y_movement.speed) {
        case SLOW: 
            if (registered_y_kc == UP_F_KC) {
                unregister_code(KC_F_UP);
            } else if (registered_y_kc == DOWN_F_KC) {
                unregister_code(KC_F_DOWN);
            }

            // Move Up
            if (y_movement.direction == POSITIVE) {
                if (registered_y_kc == DOWN_S_KC)
                    unregister_code16(KC_S_DOWN);
                register_code16(KC_S_UP);
                registered_y_kc = UP_S_KC;
            }
            // Move Down
            else {
                if (registered_y_kc == UP_S_KC)
                    unregister_code16(KC_S_UP);
                register_code16(KC_S_DOWN);
                registered_y_kc = DOWN_S_KC;
            }
        case FAST:
            if (registered_y_kc == UP_S_KC) {
                unregister_code16(KC_S_UP);
            } else if (registered_y_kc == DOWN_S_KC) {
                unregister_code16(KC_S_DOWN);
            }

            // Move Up
            if (y_movement.direction == POSITIVE) {
                if (registered_y_kc == DOWN_F_KC)
                    unregister_code(KC_F_DOWN);
                register_code(KC_F_UP);
                registered_y_kc = UP_F_KC;
            }
            // Move Down
            else {
                if (registered_y_kc == UP_F_KC)
                    unregister_code(KC_F_UP);
                register_code(KC_F_DOWN);
                registered_y_kc = DOWN_F_KC;
            }
            break;
        // No Y Movement
        case NO_MVT:
            if (registered_y_kc == UP_F_KC) {
                unregister_code(KC_F_UP);
            } else if (registered_y_kc == DOWN_F_KC) {
                unregister_code(KC_F_DOWN);
            } else if (registered_y_kc == UP_S_KC) {
                unregister_code16(KC_S_UP);
            } else if (registered_y_kc == DOWN_S_KC) {
                unregister_code16(KC_S_DOWN);
            }

            registered_y_kc = Y_NONE;
            break;
    }
}

void register_movement_kc(const report_mouse_t mouse_report) {
    if (!_game_mode) {
        return;
    }

    register_x_movement_kc(calculate_movement(mouse_report.x));
    register_y_movement_kc(calculate_movement(mouse_report.y));
}

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    register_movement_kc(mouse_report);

    return mouse_report;
}

bool pointing_device_send(void) {
    static report_mouse_t old_report         = {};
    report_mouse_t        current_report     = pointing_device_get_report();
    bool                  should_send_report = has_mouse_report_changed(&current_report, &old_report) && !_game_mode;

    if (should_send_report) {
        host_mouse_send(&current_report);
    }
    // send it and 0 it out except for buttons, so those stay until they are explicitly over-ridden using update_pointing_device
    uint8_t buttons = current_report.buttons;
    if(!_game_mode) {
        memset(&current_report, 0, sizeof(current_report));
        current_report.buttons = buttons;
        memcpy(&old_report, &current_report, sizeof(current_report));
        pointing_device_set_report(current_report);
    }

    return should_send_report || buttons;
}