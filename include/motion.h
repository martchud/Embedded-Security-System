#include <stdbool.h>
// All actions associated ot the PIR Motion Sensor
#ifndef MOTION_H
#define MOTION_H

// Define the starting and ending calls to the thread that controls the 
// initialization, configuration and extraction of information from the
// PIR motion sensor. 
void Motion_startSensorListener(void);
void Motion_stopSensorListener(void);

// uses the motion sensor to determine if there is motion being detected
bool Motion_readPIRSensor(void);


#endif