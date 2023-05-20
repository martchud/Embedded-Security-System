#include "include/alarm_interface.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "include/motion.h"
#include "include/14segCountdown.h"
#include "include/led_status.h"
#include "include/stopwatch.h"
#include "include/terminal.h"
#include "include/audioOutput.h"
#include "include/volKnob.h"
#include "include/joystick.h"
#include "include/helpers.h"
#include "include/ext_8x8led.h"
#include "include/keyboard.h"

#define PASSWORD_PATH "/mnt/remote/myApps/password.txt"

// For 8x8 matrix
static bool STOP_SYMBOL[ROW_SIZE][COL_SIZE] = {
        {0,0,1,1,1,1,0,0},
        {0,1,0,0,0,0,1,0},
        {1,0,1,0,0,1,0,1},
        {1,0,0,1,1,0,0,1},
        {1,0,0,1,1,0,0,1},
        {1,0,1,0,0,1,0,1},
        {0,1,0,0,0,0,1,0},
        {0,0,1,1,1,1,0,0},
};

static bool LEDS_OFF[ROW_SIZE][COL_SIZE] = {
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0}
};

static pthread_t Alarm_Logic_threadID;
static void Password_getFromMemory(void);
static void* Alarm_LogicThread(void *vargp);
// static bool Alarm_isRunning;
/**
 * 
 * 
*/
static Alarm myAlarmSystem;
Alarm* getAlarm(){
    return &myAlarmSystem;
}

void Alarm_Initialize(void)
{
    Motion_startSensorListener();
    // Analog_startDisplaying();
    Display_initialize();
    Stopwatch_startProcess();
    Terminal_startPrinting();
    Vol_startSampling();
    Audio_player_start();
    Joystick_startListening();
    Keyboard_scan();
    extLED_init();

    // Stopwatch_setTime(30);
    myAlarmSystem.countdown_timer = 0;  //starts 0
    myAlarmSystem.current = 0;
    myAlarmSystem.isArmed = true;
    myAlarmSystem.volume = 50;          //controlled by the potentiometer otherwise
    Password_getFromMemory();

    LED_turnOff();
    
    pthread_create(&Alarm_Logic_threadID,NULL,&Alarm_LogicThread,NULL);
}

void Alarm_Shutdown()
{   
    
    Audio_player_shutDown();
    Joystick_stopListening();
    Vol_stopSampling();
    Terminal_stopPrinting();
    Stopwatch_stopProcess();
    // Analog_stopDisplaying();
    Display_stop();
    Motion_stopSensorListener();
    Keyboard_stop();
    myAlarmSystem.isArmed = false;
    extLED_stop();
    printf("Shutting down alarm\n");

    pthread_join(Alarm_Logic_threadID,NULL);
    myAlarmSystem.current = 0;
}
/**
 * 
 * 
*/

// Internal: used to apply a password to the struct on startup.
// grabs the four digits from the file. to be used in comparing file content 
static void Password_getFromMemory(void)
{
    
    FILE *passwordfile = fopen(PASSWORD_PATH,"r");
    if (passwordfile==NULL){
        printf("ERROR: File not found: %s\n", PASSWORD_PATH);
        exit(1);
    }
    int buff_size = 1024;     
    char buff[buff_size];     
    fgets(buff, (buff_size-1), passwordfile);     
    int pword;     
    sscanf(buff, "%d", &pword);     
    
    myAlarmSystem.password = pword;
}

// updates the contents as triggered by udp
void Password_update(short newPW)
{
    
    if (newPW < 0 || newPW > 9999){
        printf("Invalid password! Must be a 4 digit integer.\n");
        return;
    }

    FILE *passwordfile = fopen(PASSWORD_PATH,"w");
    if (passwordfile==NULL){
        printf("ERROR: File not found: %s\n", PASSWORD_PATH);
        exit(1);
    }

    myAlarmSystem.password = newPW;            // update local struct password
    fprintf(passwordfile, "%d", newPW);         // update changed password in file
	fclose(passwordfile);
}

// MAIN CODE FOR THIS PROJECT
// ALL functionality is cycled through these four modes

static void* Alarm_LogicThread(void *vargp)
{
    myAlarmSystem.isArmed = true;
    printf("Starting logic algorithm for alarm\n");
    while (myAlarmSystem.isArmed)
    {

        switch(myAlarmSystem.current){

            case unarmed:           // Case where alarm isn't on for action.
                myAlarmSystem.countdown_timer = 0;
                myAlarmSystem.motionEvents = 0;
                break;

            case off:               // Case where armed but not triggered                              
                if (myAlarmSystem.motionEvents > 4){            // Consistent Motion changes the state to alerted. 
                        myAlarmSystem.current = alerted;
                        myAlarmSystem.countdown_timer = 20;     // Set alarm timer
                }
                break;

            case alerted:
            
                if (myAlarmSystem.countdown_timer == 1){
                    myAlarmSystem.current = sounding;
                }
                playSound(1);

                break;

            case sounding:
                // printf("Alarm sounding!\n");
                playSound(2);

                break;
        }

        if(myAlarmSystem.current == sounding){
            extLED_setLEDs(STOP_SYMBOL);
            extLED_setFlash(FLASH_MED);
        }else{
            extLED_setFlash(FLASH_NONE);
            extLED_setLEDs(LEDS_OFF);
        }

        sleep(1);
    }
    printf("Ending main running logic thread for Alarm\n");
    return 0;
}