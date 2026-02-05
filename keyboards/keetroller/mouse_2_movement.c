// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

// === Includes ===
#include QMK_KEYBOARD_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mouse_2_movement.h"

// === Defines ===
#ifdef _DEBUG
// Enable printing to qmk console
#    include "print.h"

#    define SPEED_STR(speed) speed == NO_MVT ? "no mvt" : (speed == FAST ? "fast" : "slow")
#    define DIRECTION_STR(dir) dir == POSITIVE ? "positive" : "negative"
#    define AXIS_STR(axis) axis == X_AXIS ? "x" : "y"
#    define REG_KC_CAT_STR(kc_cat) kc_cat == NONE_REGISTERED ? "none" : (kc_cat == POSITIVE_SLOW ? "pos slw" : (kc_cat == POSITIVE_FAST ? "pos fst" : (kc_cat == NEGATIVE_SLOW ? "neg slw" : "neg fst")))
#endif

// Helper macros
/**
 * @brief Concat token `a` with token `b`.
 * @param a leading token
 * @param b trailing token
 **/
#define CONCAT(a, b) a##b

#define IS_MODIFIED(kc) (kc & QK_RCTL)
#ifndef _DEBUG
/**
 * @brief Concat the QMK register or unregister function name for base keycodes.
 * If `reg_unreg` is `register`, the concated function name is `register_code`, if it is `unregister`, the name is `unregister_code`.
 * @param reg_unregister `register` to concat `register_code`, `unregister` to concat `unregister_code`
 * @example `CAT_FUN_NAME(register) (kc); // Results in register_code(kc)`
 **/
#    define CAT_FUN_NAME(reg_unreg) CONCAT(reg_unreg, _code)
/**
 * @brief Concat the QMK register or unregister function name for modified keycodes.
 * If `reg_unreg` is `register`, the concated function name is `register_code16`, if it is `unregister`, the name is `unregister_code16`.
 * @param reg_unreg `register` to concat `register_code16`, `unregister` to concat `unregister_code16`
 * @example `CAT_FUN16_NAME(unregister) (kc); // Results in unregister_code16(kc)`
 **/
#    define CAT_FUN16_NAME(reg_unreg) CONCAT(reg_unreg, _code16)
/**
 * @brief Call the corresponding QMK function to un- / register the keycode `kc`.
 * If `kc` is a modified keycode (e.g. `RCTL(KC_L)`) the un- / register function for modified keycodes is called (e.g. `register_cod16(kc)`).
 * If `kc` is a base keycode (e.g. `KC_L`) the un- / register function for base keycodes is called (e.g. `register_code(kc)`).
 * @param reg_unreg `register` to call `register_code` / `register_code16`, `unregister` to call `unregister_code` / `unregister_code16`
 * @example `QMK_REGISTER_UNREGISTER(register, KC_L); // Results in register_code(KC_L)`
 * @example `QMK_REGISTER_UNREGISTER(unregister, RCTL(KC_L)); // Results in unregister_code16(RCTL(KC_L))
 **/
#    define QMK_REGISTER_UNREGISTER(reg_unreg, kc) /*is_ctl_modified(kc)*/ IS_MODIFIED(kc) ? CAT_FUN16_NAME(reg_unreg)(kc) : CAT_FUN_NAME(reg_unreg)(kc)
#else
// #define CAT_FUN_NAME(reg_unreg, kc) uprintf("%s_code\t%s\n", #reg_unreg, get_keycode_string(kc))
// #define CAT_FUN16_NAME(reg_unreg, kc) uprintf("%s_code16\t%s\n", #reg_unreg, get_keycode_string(kc))
#    define CAT_FUN_NAME(reg_unreg, kc) uprintf("%s_code\t%04x\n", #reg_unreg, kc)
#    define CAT_FUN16_NAME(reg_unreg, kc) uprintf("%s_code16\t%04x\n", #reg_unreg, kc)
#    define QMK_REGISTER_UNREGISTER(reg_unreg, kc) is_ctl_modified(kc) ? CAT_FUN16_NAME(reg_unreg, kc) : CAT_FUN_NAME(reg_unreg, kc)
#endif
/**
 * @brief Call `QMK_REGISTER_UNREGISTER` to register the keycode `kc`.
 * @param kc The keycode to register.
 **/
#define QMK_REGISTER_KC(kc) QMK_REGISTER_UNREGISTER(register, kc)
/**
 * @brief Call `QMK_REGISTER_UNREGISTER` to unregister the keycode `kc`.
 * @param kc The keycode to unregister.
 **/
#define QMK_UNREGISTER_KC(kc) QMK_REGISTER_UNREGISTER(unregister, kc)

enum movement_kc_t {
    // Fast movement (replaces WASD)
    KC_F_RIGHT = KC_L,
    KC_F_LEFT  = KC_J,
    KC_F_UP    = KC_K, // On y axis, `POSITIVE`/`NEGATIVE` directions
    KC_F_DOWN  = KC_I, // are inverted.
    // Slow movement (replaces WASD with R_CTL modifier)
    // QK_RCTL is 0x1100, RCTL(kc) results in `QK_RCTL | kc`
    KC_S_RIGHT = RCTL(KC_F_RIGHT),
    KC_S_LEFT  = RCTL(KC_F_LEFT),
    KC_S_UP    = RCTL(KC_F_UP),
    KC_S_DOWN  = RCTL(KC_F_DOWN),
    NO_MVT_KC  = KC_NO,
};

/**
 * @enum movement_speed_t
 * @brief Represents possible movement speeds.
 **/
enum movement_speed_t {
    NO_MVT, /**< Indicates that no movement command should be sent. */
    SLOW,   /**< Slow movement should be sent (e.g. `RCTL(KC_I)`). */
    FAST    /**< Fast movement should be sent (e.g. `KC_I`). */
};

enum movement_direction_t { POSITIVE, NEGATIVE };

enum movement_axis_t { X_AXIS, Y_AXIS, AXIS_CNT };

/**
 * @enum registered_kc_cat_t
 * @brief Defines keycode categories for movement speeds and directions.
 **/
enum registered_kc_cat_t { NONE_REGISTERED, POSITIVE_SLOW, POSITIVE_FAST, NEGATIVE_SLOW, NEGATIVE_FAST };

/**
 * @struct movement_t
 * @brief Represents a movement on an axis with speed, direction, axis, registered keycode.
 **/
struct movement_t {
    enum movement_speed_t     speed;
    enum movement_direction_t direction;
    enum movement_axis_t      axis;
    enum movement_kc_t        registered_kc;
};

/**
 * @struct xy_movement_t
 * @brief Structures the x and y movement.
 **/
struct xy_movement_t {
    struct movement_t movement_axis[AXIS_CNT];
};

/**
 * @brief Factor to multiply the subtractand by when calculating the speed transitien.
 * @see The docs of `speed_transition_t`.
 **/
#define SPEED_DIFF_FAC 3

/**
 * @enum speed_transition_t
 * @brief Represents the movement speed transitions from one movement to another.
 * If the speeds are not equal, the calculation is done by subtracting the current speed from the previous one.
 * To ensure the values are distinctive from each other, the subtractand is multiplied by a factor.
 **/

// clang-format off
enum speed_transition_t { 
    NO_TRANSITION = 0, 
    SLOW_2_FAST = SLOW - SPEED_DIFF_FAC * FAST, // 7
    FAST_2_SLOW = FAST - SPEED_DIFF_FAC * SLOW, // 5
    SLOW_2_NO_MVT = SLOW - SPEED_DIFF_FAC * NO_MVT, // 1
    FAST_2_NO_MVT = FAST - SPEED_DIFF_FAC * NO_MVT, // 2
    NO_MVT_2_SLOW = NO_MVT - SPEED_DIFF_FAC * SLOW, // 3
    NO_MVT_2_FAST = NO_MVT - SPEED_DIFF_FAC * FAST  // 7
};
//  clang-format on
// === Function definitions ===
/**
 * @brief Determine if `kc` is a keycode modified with RCTL (e.g. `RCTL(KC_L)`).
 * @param kc The keycode to inspect.
 * @return `true` if `kc` is modified with `RCTL`.
 **/
static inline bool is_ctl_modified(const uint16_t kc) {
    return (kc & QK_RCTL);
    //return true;
}

/**
 * @brief Calculate the speed transition from the previous to the current movement.
 * If `prev_speed` is equal to `curr_speed`, `NO_TRANSITION` is returned. Otherwise,
 * the transition is calculated by `previous_speed - SPEED_DIFF_FAC * curr_speed`.
 * @param prev_speed The previous movement speed level.
 * @param curr_speed The current movement speed level.
 * @return The `speed_transition_t` value.
 * @see The docs of `speed_transition_t`.
 **/
static inline enum speed_transition_t calculate_speed_transition(const enum movement_speed_t prev_speed, const enum movement_speed_t curr_speed) {
    return prev_speed == curr_speed ? NO_TRANSITION : (prev_speed - SPEED_DIFF_FAC * curr_speed);
}

/**
 * @brief Get the movement direction based on the given keycode.
 * @param kc The keycode to get the corresponding movement direction from.
 * @return `POSITIVE` for up/right movements, `NEGATIVE` otherwise.
 **/
static inline enum movement_direction_t kc_2_dir(const enum movement_kc_t kc) {
    return ((kc & 0xff) == KC_F_RIGHT || (kc & 0xff) == KC_F_UP) ? POSITIVE : NEGATIVE;
}

/**
 * @brief Calculate the movement speed from mouse report's movement axis value.
 * @param axis_value The value of the x- / y-axis movement.
 * @return A `movement_speed_t` value, bassed on `axis_value`.
 **/
static enum movement_speed_t calculate_movement_speed(const int axis_value) {
    const uint8_t min_threshold = 2;
    const uint8_t range         = 2;

    int axis = abs(axis_value);

    if (axis < min_threshold) return NO_MVT;
    if (!slow_movement_enabled) return FAST;

    if (axis < min_threshold + range) {
        return SLOW;
    } else {
        return FAST;
    }
}
static inline enum movement_speed_t kc_2_speed(const enum movement_kc_t kc) {
    // TODO: return `NO_MVT` if `kc` is no movement keycode
    return kc == NO_MVT_KC ? NO_MVT : (is_ctl_modified(kc) ? SLOW : FAST);
}
/**
 * @brief Get the corresponding keycode based on axis, direction and speed.
 * @param axis The axis on which the movement happens (`X_AXIS` or `Y_AXIS`).
 * @param direction The direction on the axis (`POSITIVE` or `NEGATIVE`).
 * @param speed The movement speed (`NO_MVT`, `FAST` or `SLOW`).
 * @return The keycode corresponding to the movement.
 **/
static enum movement_kc_t axis_dir_spd_2_kc(const enum movement_axis_t axis, const enum movement_direction_t direction, const enum movement_speed_t speed) {
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

/**
 * @brief Generate the axis movement based on the axis, value from mouse report and registered_kc.
 * @param axis The axis for the movement (`X_AXIS` or `Y_AXIS`).
 * @param axis_value The value from the mouse report on the `axis`.
 * @param registered_kc A keycode which will be set as the movement's `registered_kc`.
 * @return The movement for one axis.
 **/
static struct movement_t calculate_movement(const enum movement_axis_t axis, const int axis_value, const enum movement_kc_t registered_kc) {
    struct movement_t movement;

    movement.speed         = calculate_movement_speed(axis_value);
    movement.direction     = axis_value < 0 ? NEGATIVE : POSITIVE;
    movement.axis          = axis;
    movement.registered_kc = registered_kc;

    return movement;
}

/**
 * @brief Check if movement speed or direction are different in `old_movement` and `current_movement`.
 * @param old_movement A `movement_t` to be compared to `current_movement`.
 * @param current_movement A `movement_t` to be compared to `old_movement`.
 * @return `true` if `speed` and/or `direction` of `old_movement` differ from the ones in `current_movement`.
 **/
static inline bool movement_changed(const struct movement_t old_movement, const struct movement_t current_movement) {
    return old_movement.speed != current_movement.speed || old_movement.direction != current_movement.direction;
}

/**
 * @brief Construct the movement on the x- and y-axis.
 * @param mouse_report The mouse report with the mouse values on x- and y-axis.
 * @param registered_x_kc A keycode to be attached to the x-axis.
 * @param registered_y_kc A keycode to be attached to the y-axis.
 * @return A `xy_movement_t` struct.
 **/
static struct xy_movement_t compose_xy_movement(const report_mouse_t mouse_report, const uint16_t registered_x_kc, const uint16_t registered_y_kc) {
    struct xy_movement_t xy_movement;

    xy_movement.movement_axis[X_AXIS] = calculate_movement(X_AXIS, mouse_report.x, registered_x_kc);
    xy_movement.movement_axis[Y_AXIS] = calculate_movement(Y_AXIS, mouse_report.y, registered_y_kc);

    return xy_movement;
}

/**
 * @brief Unregister the keycode on the `movement_axis` if any is registered.
 * If `NO_MVT_KC` is `registered_kc`, this function does nothing.
 * After unregistering the keycode, `registered_kc` of `movement_axis` will be set to `NO_MVT_KC`.
 * @param movement_axis Pointer to the `movement_t` axis on which the keycode will be unregistered.
 * @return void
 **/
void unregister_axis_movement_kc(struct movement_t *const movement_axis) {
    const enum movement_kc_t registered_kc = movement_axis->registered_kc;

    if (registered_kc == NO_MVT_KC) return;

    QMK_UNREGISTER_KC(registered_kc);

    movement_axis->registered_kc = NO_MVT_KC;
}

/**
 * @brief Unregister the keycodes of both axis if any is registered.
 * @param movement Pointer to the `xy_movement_t` struct variable.
 **/
static void unregister_all_movement_kc(struct xy_movement_t *const movement) {
    if (movement->movement_axis[X_AXIS].registered_kc == NO_MVT_KC && movement->movement_axis[Y_AXIS].registered_kc == NO_MVT_KC) {
        return;
    }

    for (int i = 0; i < AXIS_CNT; i++) {
        unregister_axis_movement_kc(&movement->movement_axis[i]);
    }
}

/**
 * @brief Move the player character on the axis specified in `movement`.
 * This function updates `movement`s `registered_kc`.
 * @note Call this function only when the movement has changed.
 * @param movement Pointer to the `movement_t` struct on the axis.
 **/
static void move_on_axis(struct movement_t *const movement) {
    // `registered_kc` is previously set and will be updated in this function. It represents the previous movement not the current one!
    enum movement_kc_t             *registered_kc = &movement->registered_kc;
    const enum movement_axis_t      axis          = movement->axis;
    const enum movement_direction_t direction     = movement->direction;
    const enum movement_speed_t     speed         = movement->speed;

    if (speed != NO_MVT) {
        const enum movement_kc_t       new_kc      = axis_dir_spd_2_kc(axis, direction, speed);
        const enum speed_transition_t speed_trans = calculate_speed_transition(kc_2_speed(*registered_kc), speed);
        const bool                     same_dir    = direction == kc_2_dir(*registered_kc);
        // If speed changes from `FAST` to `SLOW` in the same direction, only modifier has to be registered.
        if (speed_trans == FAST_2_SLOW && same_dir) {
            QMK_REGISTER_KC((uint16_t)QK_RCTL);
        }
        // If speed changes from `SLOW` to `FAST` in same direction, only modifier has to be unregistered.
        else if (speed_trans == SLOW_2_FAST && same_dir) {
            QMK_UNREGISTER_KC(QK_RCTL);
        }
        // If speed stays on `SLOW` but direction changes, unregister the old 'direction' kc and register the new one.
        else if (speed == SLOW && speed_trans == NO_TRANSITION && !same_dir) {
            QMK_UNREGISTER_KC(*registered_kc & 0xff);
            QMK_REGISTER_KC(new_kc & 0xff);
        } else {
            QMK_UNREGISTER_KC(*registered_kc);
            QMK_REGISTER_KC(new_kc);
        }

        *registered_kc = new_kc;
    } else {
        unregister_axis_movement_kc(movement);
    }
}

/**
 * @brief Translate the axis values of the `mouse_report` to character movement keycodes.
 * @param mouse_report The mouse report which will be translated to the player movement.
 * @param game_mode Indicate if game mode is active (`true`) or inactive (`false`).
 **/
void move_player_character(const report_mouse_t mouse_report, const bool game_mode) {
    static struct xy_movement_t old_xy_movement = {};

    if (!game_mode) {
        if (old_xy_movement.movement_axis[X_AXIS].registered_kc != NO_MVT_KC || old_xy_movement.movement_axis[Y_AXIS].registered_kc != NO_MVT_KC) {
            unregister_all_movement_kc(&old_xy_movement);
        }
        return;
    }

    struct xy_movement_t current_xy_movement = compose_xy_movement(mouse_report, old_xy_movement.movement_axis[X_AXIS].registered_kc, old_xy_movement.movement_axis[Y_AXIS].registered_kc);

    for (int i = 0; i < AXIS_CNT; i++) {
        if (movement_changed(old_xy_movement.movement_axis[i], current_xy_movement.movement_axis[i])) {
            move_on_axis(&current_xy_movement.movement_axis[i]);
        }
    }

    memcpy(&old_xy_movement, &current_xy_movement, sizeof(old_xy_movement));
}