/*
    Module for controlling an external LED matrix
*/
#ifndef LED_MATRIX_H
#define LED_MATRIX_H

#include <stdbool.h>

#define ROW_SIZE 8
#define COL_SIZE 8

enum extLED_flash {
    FLASH_NONE = 0,
    FLASH_SLOW,
    FLASH_MED,
    FLASH_FAST
};

enum extLED_brightness {
    BRIGHTNESS_OFF = 0,
    BRIGHTNESS_LOW,
    BRIGHTNESS_HIGH
};

// Turns on the external LED matrix
// Must be called before use
void extLED_init(void);

// Turns off the LED matrix
// Must be the last call to the module
void extLED_stop(void);

// Turns matrix lights on/off based on 2d array of boolean values
// Matrices with sizes other than ROW_SIZE*COL_SIZE will result in undefined behaviour 
void extLED_setLEDs(bool led_settings[ROW_SIZE][COL_SIZE]);

// 
void extLED_setFlash(enum extLED_flash speed);

void extLED_setBrightness(enum extLED_brightness brightness);


#endif