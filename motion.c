// Code/threads associated with the motion sensor
#include "include/motion.h"
#include "include/helpers.h"
#include "include/led_status.h"
#include "include/14segCountdown.h"
#include "include/stopwatch.h"
#include "include/alarm_interface.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <unistd.h>

#define EXPORT_FOLDER "/sys/class/gpio/export"
#define MOTION_GPIO_DIR "/sys/class/gpio/gpio48/direction"
#define MOTION_GPIO_VAL "/sys/class/gpio/gpio48/value"

#define LED_GPIO_DIR "/sys/class/gpio/gpio49/direction"
#define LED_GPIO_VAL "/sys/class/gpio/gpio49/value"
#define MOTION_GPIO_NUM 48
#define LED_GPIO_NUM 49

static Alarm* myAlarmSystem;

// ------------- Local Variables -------------

static pthread_t motionSensor_threadID;
static void* motionSensorThread(void *vargp);
static bool motionSensor_isRunning;

// ------------- Local Functions -------------


static void initPIRSensor() {     

    // runCommand("config-pin p9.15 gpio"); // pin 48
    // runCommand("config-pin p9.23 gpio"); // pin 49

    FILE *pFileExp = fopen(EXPORT_FOLDER, "w");     
    if (pFileExp == NULL) {         
        printf("ERROR: Unable to open export file.\n");         
        exit(1);     
    }  
      
    int written = fprintf(pFileExp, "%d", MOTION_GPIO_NUM);     
    if (written <= 0) {         
        printf("ERROR: Unable to write export file.\n");     
    }     
    written = fprintf(pFileExp, "%d", LED_GPIO_NUM);     
    if (written <= 0) {         
        printf("ERROR: Unable to write export file.\n");     
    }     

    fclose(pFileExp);     
        
    sleep(1);

    FILE* pFileDir = fopen(MOTION_GPIO_DIR, "w");     
    // if (pFileDir == NULL) {                                  //this should be here, but for whatever reason it doesn't work
    //     printf("ERROR: Unable to open direction file.\n");         
    //     exit(1);     
    // }    

    written = fprintf(pFileDir, "in");    

    if (written <= 0) {         
        printf("ERROR: Unable to write MOTION direction file.\n");     
    }     

    fclose(pFileDir); 

    pFileDir = fopen(LED_GPIO_DIR, "w");   
    written = fprintf(pFileDir, "out");    

    if (written <= 0) {         
        printf("ERROR: Unable to write LED direction file.\n");     
    }     

    fclose(pFileDir); 
} 

// reads the GPIO value to determine if any motion has been detected.
// extracts and uses this value for other events
bool Motion_readPIRSensor(){
    FILE *pFile = fopen(MOTION_GPIO_VAL, "r");     
    if (pFile == NULL) {         
        printf("ERROR: Unable to open MOTION GPIO value file.\n");         
        exit(1);     
    }     
    int buff_size = 1024;     
    char buff[buff_size];     
    fgets(buff, (buff_size-1), pFile);     
    int val;     
    sscanf(buff, "%d", &val);     
    fclose(pFile);     
    
    if(val == 1){
        return true;     
    } else {         
        return false;    
    }
}

// ------------- External Functions -------------

void Motion_startSensorListener(){
    initPIRSensor();        //set up the sensor for GPIO
    motionSensor_isRunning = true;
    pthread_create(&motionSensor_threadID, NULL, &motionSensorThread, NULL);

}

void Motion_stopSensorListener(){
    motionSensor_isRunning = false;
    pthread_join(motionSensor_threadID, NULL);
}


static void* motionSensorThread(void *vargp){
    myAlarmSystem = getAlarm();
    motionSensor_isRunning = true;
    printf("Starting motion sensor thread execution\n");
    LED_turnOff();
    while (motionSensor_isRunning){
        if (myAlarmSystem->current!=0){     // only detect motion when the system is armed
            // Motion_readPIRSensor();
            if (Motion_readPIRSensor())
            {
                myAlarmSystem->motionEvents++;
            }
            else
            {
                if (myAlarmSystem->motionEvents > 0)
                    myAlarmSystem->motionEvents--;
            }
        }
        sleep(1);

    }
    printf("Ending motion sensor thread execution\n");
    return NULL;
}