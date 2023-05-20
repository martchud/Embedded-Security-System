#include "include/joystick.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#include "include/helpers.h"
#include "include/alarm_interface.h"
#include "include/audioOutput.h"

#define JOYSTICK_DEBOUNCE_MS 175
#define MIDDLE_DEBOUNCE_MS 500


static pthread_t joystickThreadID;
static void* joystickThread(void *vargp);
bool joystick_isRunning;

static Alarm* myAlarmSystem;


void Joystick_startListening()
{
    joystick_isRunning = true;
    pthread_create(&joystickThreadID,NULL,&joystickThread,NULL);
}

void Joystick_stopListening()
{
    joystick_isRunning = false;
    pthread_join(joystickThreadID,NULL);
}

//
// static members: 
//

//write to file (hardcoded to write "in" to the joystick direciton)
static void setStickToInput(char * joystickFile) 
{
    FILE *pFile = fopen(joystickFile, "w"); 
    if (pFile == NULL) { 
        printf("ERROR: Unable to open file (%s) for read\n", joystickFile); 
        exit(-1); 
    } 
    fprintf(pFile,"in");         //joystick pin set to input
    fclose(pFile);  
}

/////////////////

// exports the joystick and sets direction / value to running
static void init_joystick(void){
    runCommand("config-pin p8.17 gpio"); //middle 
    setStickToInput(mid_dir);
}

//determines if the given joystick path has been activated
static bool joystick_pressed(char* joystickFile)
{
    FILE *pFile = fopen( joystickFile, "r"); 
    if (pFile == NULL) { 
        printf("ERROR: Unable to open file (%s) for read\n", joystickFile); 
        exit(-1); 
    } 
    // Read string (line) 
    const int MAX_LENGTH = 1024; 
    char press_val[MAX_LENGTH]; 
    fgets(press_val, MAX_LENGTH, pFile);    //puts output into buff as a string
    fclose(pFile);

    if (atol(press_val) == 0LL)
        return true; 
        
    return false;
}

// Constantly listens for interaction with the joystick, and performs an action based on the action
static void* joystickThread(void *vargp)
{
    init_joystick();
    printf("Starting joystick listener thread!\n");
    joystick_isRunning = true;
    myAlarmSystem = getAlarm();

    while(joystick_isRunning)
    {
        if (joystick_pressed(gpio_mid))
        {
            playSound(1);
            // FOR DEMONSTRATION PURPOSES.... SHOULD REQUIRE PASSWORD TO DO THIS OTHERWISE.
            if (myAlarmSystem->current == unarmed){         //middle press turns on / off as a stopgap
                printf("Alarm turned on!\n");
                myAlarmSystem->current = off;
            }else{
                printf("Alarm turned off!\n");
                myAlarmSystem->current = unarmed;
            }

            sleepForMs(MIDDLE_DEBOUNCE_MS);
        }

    }
    printf("Ending joystick thread execution\n");
    return 0;
}
