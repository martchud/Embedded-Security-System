#ifndef _SEGEMENT_DISP_H_
#define _SEGEMENT_DISP_H_

// Turns on display
// Must be called before display can be used
void Display_initialize(void);

// Displays any integer between 0 and 99
// if input is larger than 99, displays 99
// if input is less than 0, displays 0
void Display_integer(int value);

// Turns off display
// Must be called after use of display
void Display_stop(void);

#endif