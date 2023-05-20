//main running code for all thread calls
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include<unistd.h>
#include "include/alarm_interface.h"
#include "include/camera.h"
#include "include/udp.h"

#define CAMERA_MOTION "motion"

int main(int argc, char* argv[])
{
    Alarm_Initialize();
    udp_startSampling();
    bool camera_detection = false;
    if(argc > 1){
        if (strncmp(argv[1], CAMERA_MOTION, strlen(CAMERA_MOTION)) == 0){
            camera_detection = true;
        }
    }
    Camera_start(camera_detection);
    // sleep(5);
    // Alarm_Shutdown();
    // udp_stopSampling();
    while(!udp_shutDown()){
        //busy wait
    }
    udp_stopSampling();
    printf("Stopping program execution!\n");
    return 0;
}
