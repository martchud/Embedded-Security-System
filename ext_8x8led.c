#include "include/ext_8x8led.h"
#include "include/helpers.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <time.h>
#include <unistd.h>

#include <string.h>

#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"
#define I2C_ADDRESS 0x70 // HT16K33 device address


// HT16K33 Register Addresses
#define HT16K33_DISPLAY_SETUP_REG 0x80
#define HT16K33_OCSCILLATOR_REG 0x20
#define HT16K33_DISPLAY_RAM_REG 0x00
#define HT16K33_BRIGHTNESS_REG 0xE0

// brightness is set by the lower 4 bits
//  e.g.) 0xE7 sets brightness to max
#define MAX_BRIGHTNESS 0x07
#define MED_BRIGHTNESS 0x03
#define MIN_BRIGHTNESS 0x00

// Flash Speed Options
#define NO_FLASH 0x00
#define SLOW_FLASH 0x06   // 0.5HZ
#define MEDIUM_FLASH 0x04 // 1HZ
#define FAST_FLASH 0x02   // 2HZ


#define ON 0x01
#define OFF 0x00

// led register values, bottom to top
#define L1 0x80
#define L2 0x01
#define L3 0x02
#define L4 0x04
#define L5 0x08
#define L6 0x10
#define L7 0x20
#define L8 0x40

static unsigned char LED_VALS[COL_SIZE] = {L8, L7, L6, L5, L4, L3, L2, L1};

// Used by init and exit functions
static unsigned char LEDS_OFF[16] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static int initI2cBus(char* bus, int address);
static void setLEDs(int fileDesc, unsigned char regAddr, unsigned char* values);

static int i2c_fd = 0; // file descriptor

void extLED_init()
{
    // configure pins 
    // runCommand("config-pin p9_18 i2c");
    // runCommand("config-pin p9_17 i2c");

    // open file and get descriptor
    i2c_fd = initI2cBus(I2CDRV_LINUX_BUS1, I2C_ADDRESS);

    // Turn on oscillator
    writeI2cReg(i2c_fd, HT16K33_OCSCILLATOR_REG | ON, 0x00);

    // Turn on display
    writeI2cReg(i2c_fd, HT16K33_DISPLAY_SETUP_REG | ON, 0x00);

    // Turn off leds by default
    setLEDs(i2c_fd, HT16K33_DISPLAY_RAM_REG, LEDS_OFF);

    // Set max brightness
    writeI2cReg(i2c_fd, HT16K33_BRIGHTNESS_REG | MAX_BRIGHTNESS, 0x00);

    return;
}

void extLED_stop()
{
    // Turn off LEDs
    setLEDs(i2c_fd, HT16K33_DISPLAY_RAM_REG, LEDS_OFF);
    // Turn off display
    writeI2cReg(i2c_fd, HT16K33_DISPLAY_SETUP_REG | OFF, 0x00);
    // Turn of oscillator
    writeI2cReg(i2c_fd, HT16K33_OCSCILLATOR_REG | OFF, 0x00);
    // close file
    close(i2c_fd);

    return;
}

void extLED_setLEDs(bool led_settings[ROW_SIZE][COL_SIZE])
{
    unsigned char led_data[16];
    memset(led_data, 0x00, 16);
    
    for(int i=0; i < ROW_SIZE; i++){
        for(int j=0; j < COL_SIZE; j++){
            if(led_settings[i][j] == true){
                // add the register value of the led on row i to column j
                led_data[j*2] |= LED_VALS[i];
            }
        }
    }

    setLEDs(i2c_fd, HT16K33_DISPLAY_RAM_REG, led_data);
}

void extLED_setFlash(enum extLED_flash speed)
{
    if(speed > 3 || speed < 0){
        printf("extLED_setFlash: Invalid flash speed defaulting to NONE (must be between 0-4, you set it to %i)\n", speed);
        speed = NO_FLASH;
    }

    unsigned char option[4] = {NO_FLASH, SLOW_FLASH, MEDIUM_FLASH, FAST_FLASH};
    writeI2cReg(i2c_fd, HT16K33_DISPLAY_SETUP_REG | ON | option[speed], 0x00);
    return;
}

void extLED_setBrightness(enum extLED_brightness brightness)
{
    if(brightness > 2 || brightness < 0){
        printf("extLED_setBrightness: Invalid brightness setting, defaulting to NONE (must be between 0-2, you set it to %i)\n", brightness);
        brightness = MED_BRIGHTNESS;
    }

    unsigned char option[3] = {MIN_BRIGHTNESS, MED_BRIGHTNESS, MAX_BRIGHTNESS};
    writeI2cReg(i2c_fd, HT16K33_BRIGHTNESS_REG | option[brightness], 0x00);
    return;
}

static void setLEDs(int fileDesc, unsigned char regAddr, unsigned char* values)
{
    unsigned char buff[17];
    buff[0] = regAddr;
    for(int i = 1; i <= 16; i++){
        buff[i] = values[i-1];
    }

    int res = write(fileDesc, buff, 17);
    if(res != 17){
        perror("I2C: Unable to write i2c register.\n");
        exit(1);
    }
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
