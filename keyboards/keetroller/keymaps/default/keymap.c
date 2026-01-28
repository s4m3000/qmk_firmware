// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Aliases
#define KC_F_RIGHT KC_L
#define KC_F_LEFT KC_J
#define KC_F_UP KC_I
#define KC_F_DOWN KC_K
#define KC_S_RIGHT RCTL(KC_F_RIGHT)
#define KC_S_LEFT RCTL(KC_F_LEFT)
#define KC_S_UP RCTL(KC_F_UP)
#define KC_S_DOWN RCTL(KC_F_DOWN)

#define REG_KC(direction, speed) direction##_##speed

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
    const uint8_t range = 8;

    int axis = abs(axis_value);
    
    if (axis < min_threshold) {
        return NO_MVT;
    }

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

enum movement_axis_t {
    X_AXIS,
    Y_AXIS,
    AXIS_CNT
};

enum registered_kc_t {
    NONE_REGISTERED,
    POSITIVE_SLOW,
    POSITIVE_FAST,
    NEGATIVE_SLOW,
    NEGATIVE_FAST
};

struct movement_t {
    enum movement_speed_t speed;
    enum movement_direction_t direction;
    enum movement_axis_t axis;
    enum registered_kc_t registered_code;
};

enum registered_x_kc_t {
    X_NONE,
    RIGHT_F_KC,
    LEFT_F_KC,
    RIGHT_S_KC,
    LEFT_S_KC,
    NO_OF_X_KC
};

enum registered_y_kc_t {
    Y_NONE,
    UP_F_KC,
    DOWN_F_KC,
    UP_S_KC,
    DOWN_S_KC,
    NO_OF_Y_KC
};

struct xy_movement_t {
    struct movement_t movement_axis[AXIS_CNT];
};

static uint8_t get_kc(const enum movement_axis_t axis, const enum movement_direction_t direction, const enum movement_speed_t speed) {
    switch (speed) {
        case FAST:
            if (axis == X_AXIS) {
                return direction == POSITIVE ? KC_F_RIGHT : KC_F_LEFT;
            } else {
                return direction == POSITIVE ? KC_F_UP : KC_F_DOWN;
            }
        case SLOW:
            if (axis == X_AXIS) {
                return direction == POSITIVE ? KC_S_RIGHT : KC_S_LEFT;
            } else {
                return direction == POSITIVE ? KC_S_UP : KC_S_DOWN;
            }
        default:
            return KC_NO;
    }
}

static struct movement_t calculate_movement(const enum movement_axis_t axis, const int axis_value, const enum registered_kc_t registered_code) {
    struct movement_t movement;
    
    movement.speed = calculate_movement_speed(axis_value);
    movement.direction = axis_value < 0 ? NEGATIVE : POSITIVE;
    movement.axis = axis;
    movement.registered_code = registered_code;

    return movement;
}

bool movement_changed(const struct movement_t old_movement, const struct movement_t current_movement) {
    return old_movement.speed != current_movement.speed && old_movement.direction != current_movement.direction;
}

static struct xy_movement_t compose_xy_movement(const report_mouse_t mouse_report, const enum registered_kc_t registered_x_code, const enum registered_kc_t registered_y_code) {
    struct xy_movement_t xy_movement;
    
    xy_movement.movement_axis[X_AXIS] = calculate_movement(X_AXIS, mouse_report.x, registered_x_code);
    xy_movement.movement_axis[Y_AXIS] = calculate_movement(Y_AXIS, mouse_report.y, registered_y_code);

    return xy_movement;
}

void unregister_axis_movement_kc(struct movement_t* const movement_axis) {
    const enum movement_axis_t axis = movement_axis->axis;
    const enum movement_direction_t direction = movement_axis->direction;
    const enum movement_speed_t speed = movement_axis->speed;
    const uint8_t kc = get_kc(axis, direction, speed);

    switch (movement_axis->registered_code) {
        case NONE_REGISTERED:
            return;
        case POSITIVE_SLOW:
        case NEGATIVE_SLOW:
            unregister_code16(kc);
            break;
        case POSITIVE_FAST:
        case NEGATIVE_FAST:
            unregister_code(kc);
            break;
    }

    movement_axis->registered_code = NONE_REGISTERED;
}

static void unregister_all_movement_kc(struct xy_movement_t* const movement) {
    if (movement->movement_axis[X_AXIS].registered_code == NONE_REGISTERED && 
        movement->movement_axis[Y_AXIS].registered_code == NONE_REGISTERED) {
        return;
    }

    for (int i = 0; i < AXIS_CNT; i++) {
        unregister_axis_movement_kc(&movement->movement_axis[i]);
    }
}

static void register_axis_movement(struct movement_t* const movement) {
    enum registered_kc_t registered_kc = movement->registered_code;
    static struct movement_t old_movement = {};

    if (!movement_changed(old_movement, *movement))
        return;

    const enum movement_axis_t axis = movement->axis;
    const enum movement_direction_t direction = movement->direction;

    switch (movement->speed) {
        case SLOW:
            // Unregister fast KCs if they where registered before.
            if(registered_kc == POSITIVE_FAST) {
                unregister_code(get_kc(axis, POSITIVE, FAST));
            } else if (registered_kc == NEGATIVE_FAST) {
                unregister_code(get_kc(axis, NEGATIVE, FAST));
            }

            // Move Right / Up
            if (direction == POSITIVE) {
                if (registered_kc == NEGATIVE_SLOW)
                    unregister_code16(get_kc(axis, NEGATIVE, SLOW));
                register_code16(get_kc(axis, POSITIVE, SLOW));
                registered_kc = POSITIVE_SLOW;
            }
            // Move Left / Down
            else {
                if (registered_kc == POSITIVE_SLOW)
                    unregister_code16(get_kc(axis, POSITIVE, SLOW));
                register_code16(get_kc(axis, NEGATIVE, SLOW));
                registered_kc = NEGATIVE_SLOW;
            }
            break;
        case FAST:
            // Unregister slow KCs if they where registered before.
            if(registered_kc == POSITIVE_SLOW) {
                unregister_code16(get_kc(axis, POSITIVE, SLOW));
            } else if (registered_kc == NEGATIVE_FAST) {
                unregister_code16(get_kc(axis, NEGATIVE, SLOW));
            }

            // Move Right / Up
            if (direction == POSITIVE) {
                if (registered_kc == NEGATIVE_FAST)
                    unregister_code(get_kc(axis, NEGATIVE, FAST));
                register_code(get_kc(axis, POSITIVE, FAST));
                registered_kc = POSITIVE_FAST;
            }
            // Move Left 
            else {
                if (registered_kc == POSITIVE_FAST)
                    unregister_code(get_kc(axis, POSITIVE, FAST));
                register_code(get_kc(axis, NEGATIVE, FAST));
                registered_kc = NEGATIVE_FAST;
            }
            break;
        // No X Movement
        case NO_MVT:
            unregister_axis_movement_kc(movement);
    }

    movement->registered_code = registered_kc;
    memcpy(&old_movement, &movement, sizeof(old_movement));
}

void register_movement_kc(const report_mouse_t mouse_report) {
    static struct xy_movement_t old_xy_movement = {};

    if (!_game_mode) {
        unregister_all_movement_kc(&old_xy_movement);
        return;
    }

    struct xy_movement_t current_xy_movement = compose_xy_movement(
        mouse_report, old_xy_movement.movement_axis[X_AXIS].registered_code, old_xy_movement.movement_axis[Y_AXIS].registered_code
    );

    for (int i = 0; i < AXIS_CNT; i++)
        register_axis_movement(&current_xy_movement.movement_axis[i]);

    memcpy(&old_xy_movement, &current_xy_movement, sizeof(old_xy_movement));
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