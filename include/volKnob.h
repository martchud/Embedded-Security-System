/*
volKnob.h
Module for functionality of reading the integrated potentiometer (POT) on the zen cape 
The position of the knob determines the loudness of the output sounds
*/
#ifndef _VOLKNOB_H_
#define _VOLKNOB_H_

#define POT_VOLTAGE_PATH "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"

// // Returns a the raw voltage value from the POT from 0-4095 inclusive. 
// // If it is unable to read the value, it returns -1.
// int Vol_getValue(void);

// // Converts the raw value reading from the POT to an actual voltage value.
// // Returns -1 if there is an error reading the raw value of the POT.
// double Vol_getVoltage(void);


//Uses the potentiometer knob to obtain a value, which is translated into a
// number between 1 and 100 to be used as a volume for the alarm
// int Vol_convertPotToVolume();

//triggers udp quit
void Vol_quit(void);

// Controls the potentiometer thread, which directly converts to a volume of the alarm output. 
void Vol_startSampling(void);

void Vol_stopSampling(void);

#endif