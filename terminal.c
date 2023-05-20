// Code/threads associated with the system interface
#include "include/terminal.h"
#include "include/motion.h"
#include "include/14segCountdown.h"
#include "include/led_status.h"
#include "include/stopwatch.h"
#include "include/audioOutput.h"
#include "include/volKnob.h"
#include "include/audioBuffer.h"

#include "include/alarm_interface.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

static pthread_t termThreadID;
static bool isPrinting;
bool toggleAlarm = false;

static Alarm* myAlarmSystem;

static void* print_to_terminal(void *vargp);

void Terminal_startPrinting(void)
{
    isPrinting = true;
    pthread_create(&termThreadID, NULL, &print_to_terminal, NULL);
}

void Terminal_stopPrinting(void)
{
    isPrinting = false;
    pthread_join(termThreadID, NULL);
}

void Terminal_quit()
{
    isPrinting = false;
}

static void printWhenOff()
{
    printf("[] SYSTEM DISARMED []\t"
           "Type 4 digit code to arm the system!\n");


}

// State when system is armed but not triggered
static void printWhenPassive()
{
    bool motionDetected = Motion_readPIRSensor();

    if (motionDetected){    // motion may be detected but that doesn't immediately set off the system
        printf("[][][] SYSTEM ARMED [][][] \t"
               "State: [Passive]\t" 
               "Motion Detected [True] \n");
    }else{
        printf("[][][] SYSTEM ARMED [][][] \t"
               "State: [Passive]\t" 
               "Motion Detected [False] \n");
    }    
}

// Print information when system is prepared to sound alarm.
// Gives a breif warning before triggering.
static void printWhenTriggered()
{
    bool motionDetected = Motion_readPIRSensor();
    
    if (motionDetected){ 
        printf("[][][] SYSTEM ARMED [][][]\t"
               "State: [ALERTED]\t" 
               "Motion Detected [True]\t"
               "Countdown [%d]\t"
               "Volume [%d]\t"
               "Enter 4 digit code to disarm!\n",
                    myAlarmSystem->countdown_timer,
                    myAlarmSystem->volume 
            );
    }
    else{
        printf("[][][] SYSTEM ARMED [][][]\t"
               "State: [ALERTED]\t" 
               "Motion Detected [False]\t"
               "Countdown [%d]\t"
               "Volume [%d]\t"
               "Enter 4 digit code to disarm!\n",
                    myAlarmSystem->countdown_timer,
                    myAlarmSystem->volume 
            );
    }
}

static void printWhenAlarmed()
{
    bool motionDetected = Motion_readPIRSensor();
    
    if (motionDetected){ 
        printf("[][][] SYSTEM ARMED [][][]\t"
               "State: [ALARMED]\t" 
               "Motion Detected [True] \t"
               "Volume [%d]\t"
               "Enter 4 digit code to disarm!\n",
                    myAlarmSystem->volume 
            );
    }
    else{
        printf("[][][] SYSTEM ARMED [][][]\t"
               "State: [ALARMED]\t" 
               "Motion Detected [False] \t"
               "Volume [%d]\t"
               "Enter 4 digit code to disarm!\n",
                    myAlarmSystem->volume 
            );
    }
}


//thread repeatedly outputs specified data to screen at a 1 second rate
static void* print_to_terminal(void *vargp)
{
    isPrinting = true;
    myAlarmSystem = getAlarm();
    while(isPrinting){  //collect relevant information from other modules for reporting.       

        if (myAlarmSystem->current == 0){
            printWhenOff();
        }

        else if (myAlarmSystem->current == 1){
            printWhenPassive();
        }

        else if (myAlarmSystem->current == 2){
            printWhenTriggered();
        }

        else if (myAlarmSystem->current == 3){
            printWhenAlarmed();
        }
        
        sleep(1);       //write every 3 seconds
    }
    return 0;
}
