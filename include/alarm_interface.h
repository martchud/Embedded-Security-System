// alarm_interface.h
// Contains the core structure of the alarm interface
// 
#include <stdbool.h>


#ifndef _INTERFACE_H_
#define _INTERFACE_H_

enum Alarmstate{
    unarmed,
    off,
    alerted,
    sounding
};

//primary structure defining the alarm functionality

typedef struct{

    bool isArmed;
    enum Alarmstate current;

    int countdown_timer;
    int volume;
    int motionEvents; 

    int password;

} Alarm;

/**
 * 
 * 
 * 
*/
Alarm* getAlarm(void);
/**
 * 
 * 
 * 
 * 
*/

// Program Controls
void Alarm_Initialize(void);
void Alarm_Shutdown(void);

// Use to update the alarm's password, which is used to control arming and disarming
// the system interface.
void Password_update(short newPW);


#endif