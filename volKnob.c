#include "include/volKnob.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#include "include/audioBuffer.h"
#include "include/alarm_interface.h"

//code given in class

#define MAX_BUF_LENGTH 1024
#define MAX_POT_VAL 4095

#define MAX_VOLUME 100


static pthread_t potThreadID;
static bool isSampling;
static Alarm* myAlarmSystem; 

// Function for the thread to sample the POT and update the buffer size.
static void* Vol_Thread(void *vargp);

void Vol_quit(void)
{
    isSampling = false;
}

void Vol_startSampling(void)
{
    isSampling = true;
    pthread_create(&potThreadID, NULL, &Vol_Thread, NULL);
}

void Vol_stopSampling(void)
{
    isSampling = false;
    pthread_join(potThreadID,NULL);
}

// Returns a the raw voltage value from the POT from 0-4095 inclusive. 
// If it is unable to read the value, it returns -1.
static int Vol_getValue(void)
{
    FILE *potVoltageFile = fopen(POT_VOLTAGE_PATH, "r");
    bool fileStatus = true;

    if (!potVoltageFile)
    {
        fprintf(stderr, "Error reading the voltage for the POT.\n");
        fileStatus = false;
    }

    char buf[MAX_BUF_LENGTH];
    fgets(buf, MAX_BUF_LENGTH, potVoltageFile); //get voltage reading from file, place in buffer
    fclose(potVoltageFile);
    int voltageVal = atoi(buf);

    if (fileStatus == false)
    {
        voltageVal = -1;
    }
    return voltageVal;                      
}


// Uses the potentiometer knob to obtain a value, which is translated into a
// number between 1 and 100 to be used as a volume for the alarm
static int Vol_convertPotToVolume()
{
    double volt = Vol_getValue(); // 0-4095 
    
    double convert_volume = (volt * MAX_VOLUME) / MAX_POT_VAL;  //converts from range of 0-4095 to range of 0-100
    int volume = convert_volume;

    return volume;
}

static void* Vol_Thread(void *vargp)
{
    myAlarmSystem = getAlarm();
    isSampling = true;
    while (isSampling)
    {
        int vol = Vol_convertPotToVolume();
        if (vol <= 25) { vol = 25; }     // alarm shouldn't ever be inaudiable

        myAlarmSystem->volume = vol;

        AudioMixer_setVolume(myAlarmSystem->volume);      // count potentiometer value
        sleep(1);                       // check for changes every 3 seconds
    }
    printf("Sampling of POT has now stopped.\n");
    return 0;
}