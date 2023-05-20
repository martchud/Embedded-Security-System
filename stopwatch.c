#include "include/stopwatch.h"
#include "include/helpers.h"
#include "include/led_status.h"

#include "include/alarm_interface.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

static void* Stopwatch_updateTimeWhenTriggered(void *vargp);

static pthread_t timerManagerThread;
static bool isUpdating;

static Alarm* myAlarmSystem;

// Thread Functions:
void Stopwatch_quit()
{
    isUpdating = false;
}

void Stopwatch_startProcess(void)
{
    isUpdating = true;
    pthread_create(&timerManagerThread, NULL, &Stopwatch_updateTimeWhenTriggered, NULL);
}

void Stopwatch_stopProcess(void)
{
    isUpdating = false;
    pthread_join(timerManagerThread, NULL);
}


// Functionality:

// used to bring down the stopwatch count every second.
// only accessed internally
static void Stopwatch_decrement(){
    if (myAlarmSystem->countdown_timer > 0){
        myAlarmSystem->countdown_timer--;
    }
}


static int Stopwatch_getBlinkFrequency(int currTime)
{
    if (currTime > 15){
        return 1;
    }else if(currTime > 5 && currTime >= 15){
        return 2;
    }else{
        return 4;
    }
}
// Main Thread

// Simply begins to count down if the stopwatch is active, blinking the LED
// as a visual warning. 
static void* Stopwatch_updateTimeWhenTriggered(void *vargp)
{
    myAlarmSystem = getAlarm();
	printf("Timer Update Thread Started!\n");
	isUpdating = true;
	while (isUpdating)
	{
        int currTime = myAlarmSystem->countdown_timer;
		if (currTime>0){		    // only perform the action if the timer is active
			LED_flashNTimesInXms(Stopwatch_getBlinkFrequency(currTime),1000);	//flash and sleep
			Stopwatch_decrement();
		}
        if (currTime==0)
        {
            LED_turnOff();
        }
	}
	printf("Timer Update Thread stopped!\n");
	return 0;
}