/* License header
 *
 */

#pragma once

#include QMK_KEYBOARD_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Indicate if game mode is active or inactive.
 * When game mode is active, reports from the keeb's pointing device will be translated into keycodes.
 * E.g. moving the pointing device all the way to the left will then result in `KC_J`, moving it only half way will send `RCTL(KC_J)` if `enable_slow_movement` is `true`.
 * The mouse report it self will not be sent to the computer.
 * @note We don't use WASD for the translated movement commands but IJKL (same pattern but on the right hand side).
 **/
extern bool game_mode;
/**
 * @brief Indicate if slow movement is enabled.
 * When slow movement is enabled, moving the analog stick only half way will send send the movement keycodes modified with `RCTL`. */
extern bool slow_movement_enabled;

extern void move_player_character(const report_mouse_t mouse_report, const bool game_mode);