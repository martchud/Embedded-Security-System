// All actions associated ot the PIR Motion Sensor
#ifndef LED_H
#define LED_H

// Setup GPIO for wired LED indicator.
// This LED is a status indicator that represents motion being detected
// void LED_init(void);

// Directly control the LED output
void LED_turnOn(void);
void LED_turnOff(void);

// Different statuses will be represented by different rates of flashing.
// This function allows for easy access to control the blinking (and sleep) frequency
void LED_flashNTimesInXms(double N, double ms);

#endif