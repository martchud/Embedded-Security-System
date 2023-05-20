#include "include/led_status.h"
#include "include/helpers.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <unistd.h>

#define EXPORT_FOLDER "/sys/class/gpio/export"
#define LED_GPIO_DIR "/sys/class/gpio/gpio49/direction"
#define LED_GPIO_VAL "/sys/class/gpio/gpio49/value"
#define LED_GPIO_NUM 49


void LED_turnOn(void)
{
    editGPIOReading(LED_GPIO_VAL,"0");
}

void LED_turnOff(void)
{
    editGPIOReading(LED_GPIO_VAL,"1");
}

void LED_flashNTimesInXms(double N, double ms)
{
    double flashTime = ms/N/2; 

    for (int i = 0; i < N; i++){
        LED_turnOn();
        sleepForMs(flashTime);
        LED_turnOff();
        sleepForMs(flashTime);
    }

}

