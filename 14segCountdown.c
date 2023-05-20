#include "include/14segCountdown.h"
#include "include/alarm_interface.h"
#include "include/helpers.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/ioctl.h>      // ioctl()
#include <linux/i2c.h>
#include <linux/i2c-dev.h>  // I2C_SLAVE macro
#include <time.h>
#include <pthread.h>
#include <unistd.h>

// I2C Configuration Settings/Options
#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"
#define I2C_DEVICE_ADDRESS 0x20

// Pin Configuration Settings/Options
#define UPPER_DIGIT_VALUE "/sys/class/gpio/gpio61/value"
#define LOWER_DIGIT_VALUE "/sys/class/gpio/gpio44/value"
#define UPPER_DIGIT_DIRECTION "/sys/class/gpio/gpio61/direction"
#define LOWER_DIGIT_DIRECTION "/sys/class/gpio/gpio44/direction"
#define VALUE_OFF "0"
#define VALUE_ON "1"

#define OFF 0x00

#ifdef CAPE_GREEN
	#define REG_DIRA 0x00
	#define REG_DIRB 0x01
	#define REG_OUTA 0x14
	#define REG_OUTB 0x15

	#define ZERO 0xA186
	#define ONE 0x8002
	#define TWO 0x310E
	#define THREE 0xB00E
	#define FOUR 0x908A
	#define FIVE 0xB08C
	#define SIX 0xB18C
	#define SEVEN 0x8006
	#define EIGHT 0xB18E
	#define NINE 0x908E

#else

	#define REG_DIRA 0x02
	#define REG_DIRB 0x03
	#define REG_OUTA 0x00
	#define REG_OUTB 0x01

	// Segement values
	#define A 0x0001
	#define B 0x8000
	#define C 0x4000
	#define D 0x1000
	#define E 0x0080
	#define F 0x0020
	#define G1 0x0002
	#define G2 0x0800
	#define H 0x0010
	#define I 0x0008
	#define J 0x0004
	#define K 0x0100
	#define L 0x0200
	#define M 0x0400

	// Values for diplaying integers

	#define ZERO (A|B|C|D|E|F)
	#define ONE (J|B|C)
	#define TWO (A|B|G1|G2|E|D)
	#define THREE (A|B|G1|G2|C|D)
	#define FOUR (F|G1|G2|B|C)
	#define FIVE (A|F|G1|G2|C|D)
	#define SIX (A|F|G1|G2|C|D|E)
	#define SEVEN (A|J|L)
	#define EIGHT (A|B|C|D|E|F|G1|G2)
	#define NINE (A|B|C|F|G1|G2)

#endif


// Delay for displaying different numbers
#define DELAY_MS 5

const int INT_DISPLAY_VALUE[10] = {ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE};

pthread_t display_tid;

static int high_digit;
static int low_digit;
static bool display_on;
static int i2cFileDesc;

// thread function
static void *runDisplay(void *args);

// functions for pin/i2c configuration
static int initI2cBus(char* bus, int address);
static void setPinValue(char* pin_value_file, char* value);


static void *runDisplay(void *args)
{
	Alarm* myAlarmSystem = getAlarm();
    while(display_on){

		// Code for alaram system -----
		int num_to_display = myAlarmSystem->countdown_timer;
        // printf("Stopwatch currently at: %d\n", num_to_display);
		
        int first_digit = num_to_display / 10;  	// moves the decimal place one to the left
        int second_digit = num_to_display % 10; 	// extracts the first num

        if (num_to_display >= 99) 					// display 01-99
		{ 
			first_digit = 9; 						// dip count over 100 rounds to 99
			second_digit= 9;
		}

		high_digit = first_digit;
		low_digit = second_digit;
		// ----------------------------

        // Turn off both digits
        setPinValue(UPPER_DIGIT_VALUE, VALUE_OFF);
        setPinValue(LOWER_DIGIT_VALUE, VALUE_OFF);

        // Set and display upper digit
        writeI2cReg(i2cFileDesc, REG_OUTA, INT_DISPLAY_VALUE[high_digit]>>8);
        writeI2cReg(i2cFileDesc, REG_OUTB, INT_DISPLAY_VALUE[high_digit]&0xFF);
        setPinValue(UPPER_DIGIT_VALUE, VALUE_ON);
        sleepForMs(DELAY_MS);

        // Turn off both digits
        setPinValue(UPPER_DIGIT_VALUE, VALUE_OFF);
        setPinValue(LOWER_DIGIT_VALUE, VALUE_OFF);

        // Set and display lower digit
        writeI2cReg(i2cFileDesc, REG_OUTA, INT_DISPLAY_VALUE[low_digit]>>8);
        writeI2cReg(i2cFileDesc, REG_OUTB, INT_DISPLAY_VALUE[low_digit]&0xFF);
        setPinValue(LOWER_DIGIT_VALUE, VALUE_ON);
        sleepForMs(DELAY_MS);
    }

    pthread_exit(0);
}

void Display_initialize(void)
{   
    // configure pins 
    runCommand("config-pin p9_18 i2c");
    runCommand("config-pin p9_17 i2c");

    // turn on 
    setPinValue(UPPER_DIGIT_VALUE, VALUE_OFF);
    setPinValue(LOWER_DIGIT_VALUE, VALUE_OFF);

    setPinValue(UPPER_DIGIT_DIRECTION, "out");
    setPinValue(LOWER_DIGIT_DIRECTION, "out");

    i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);

    // set GPIO extender to be outputs
    writeI2cReg(i2cFileDesc, REG_DIRA, 0x00);
    writeI2cReg(i2cFileDesc, REG_DIRB, 0x00);

    writeI2cReg(i2cFileDesc, REG_OUTA, OFF);
    writeI2cReg(i2cFileDesc, REG_OUTB, OFF);
    
    high_digit = OFF;
    low_digit = OFF;
    display_on = true;

    // create display thread
    pthread_create(&display_tid, NULL, &runDisplay, NULL);

}

void Display_integer(int value)
{
    if(value > 99){ value = 99; }
    if(value < 0){ value = 0; }

    low_digit = value%10;
    value /= 10;
    high_digit = value%10;
}

void Display_stop(void)
{
    display_on = false;
    pthread_join(display_tid, NULL);

    writeI2cReg(i2cFileDesc, REG_OUTA, OFF);
    writeI2cReg(i2cFileDesc, REG_OUTB, OFF);
    setPinValue(UPPER_DIGIT_VALUE, VALUE_OFF);
    setPinValue(LOWER_DIGIT_VALUE, VALUE_OFF);

    close(i2cFileDesc);
}

static int initI2cBus(char* bus, int address)
{
    int fileDesc = open(bus, O_RDWR);
    int result = ioctl(fileDesc, I2C_SLAVE, address);
    if(result < 0) {
        perror("I2C: Unable to set I2C device to slave address");
        exit(1);
    }
    return fileDesc;
}

static void setPinValue(char* pin_value_file, char* value)
{
    // open file
    FILE *pPinValueFile = fopen(pin_value_file, "w");
    if(pPinValueFile == NULL){
        printf("ERROR OPENING \'%s\'.\n", pin_value_file);
        exit(1);
    }
    // write value to file
    int charWritten = fprintf(pPinValueFile, value);
    if(charWritten <= 0){
        printf("ERROR WRITING DATA TO \'%s\'.\n", pin_value_file);
        exit(1);
    }
    // close file
    fclose(pPinValueFile);
}
