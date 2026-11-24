# clang-format off
SRC += mouse_2_movement.c
#Bootloader selection
BOOTLOADER = rp2040
# enable debugging
# CONSOLE_ENABLE = yes
# KEYCODE_STRING_ENABLE = yes

POINTING_DEVICE_ENABLE = yes 
POINTING_DEVICE_DRIVER = analog_joystick

OLED_ENABLE = yes 
OLED_DRIVER = ssd1306 
OLED_TRANSPORT = i2c
# clang-format on