// HELPER FUNCTIONS FOR USE ACROSS ALL MODULES
// Time support
#ifndef HELPERS_H
#define HELPERS_H

#define SEC_PER_MIN 60
#define MS_PER_SEC 1000

// sleep program execution for a given number of ms
void sleepForMs(long long delayInMs);

// run a console command
void runCommand(char* command); 

// edit the file with the associated value, to be used
// with GPIO files only
void editGPIOReading(char* fileName, char* val);

void writeI2cReg(int i2cFileDescr, unsigned char regAddr, unsigned char value);

#endif