/* License header
 *
 */

#pragma once

// Analog Stick defines (https://docs.qmk.fm/features/pointing_device#analog-joystick)
#define ANALOG_JOYSTICK_X_AXIS_PIN GP28
#define ANALOG_JOYSTICK_Y_AXIS_PIN GP27
#define ANALOG_JOYSTICK_AUTO_AXIS
#define POINTING_DEVICE_INVERT_X
#define POINTING_DEVICE_INVERT_Y
// https://blog.ffff.lt/posts/keyboard-with-joysticks-5/
// #define ANALOG_JOYSTICK_WEIGHTS {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,28,29,29,30,31,33,34,35,36,37,40,41,43,44,48,49,51,56,58,60,65,68,70,73,79,82,85,89,96,100}

#define TAPPING_TERM 300
#define TAPPING_TERM_PER_KEY

#define I2C_DRIVER I2CD0
#define I2C1_SDA_PIN GP0
#define I2C1_SCL_PIN GP1
