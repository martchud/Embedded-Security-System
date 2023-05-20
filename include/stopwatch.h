// stopwatch.h
// Module to run the clock associated to the security system's pre-state
// Works closely with the LED_Status module
// 
#ifndef _STOPWATCH_H_
#define _STOPWATCH_H_

//stop looping the thread for stopwatch
void Stopwatch_quit();

// Begin/end the background thread which displays the # of dips on the I2C display
void Stopwatch_startProcess(void);
void Stopwatch_stopProcess(void);

#endif