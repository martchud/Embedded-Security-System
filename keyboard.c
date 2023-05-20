#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include "include/keyboard.h"
#include "include/alarm_interface.h"

//Input event path that maps to USB keyboard numpad (other parts of keyboard not used)
#define INPUT_EVENT "/dev/input/event1"

//Keycode to ASCII mapping and other boundary locations
#define KEY_MAP ((int[]){7,8,9,-1,4,5,6,-1,1,2,3,0})
#define KEY_OFFSET 71
#define ENTERKEY 96
#define IGNOREKEY_A 74
#define IGNOREKEY_B 78
#define KEY_BOUNDARY 82

static int code = 0;
static int pos = 0;
static int fd;

Alarm* myAlarm;

static pthread_t scanthread;
static bool killthread = false;

void *scanner(void *args)
{
    //Create a struct to store input data
    struct input_event ev;

    while (!killthread) {
        ssize_t n = read(fd, &ev, sizeof(ev));
        if (n < sizeof(ev)) {
            perror("Error reading input event");
            exit(1);
        }

        //Keypress deteced
        if (ev.type == EV_KEY && ev.value == 1) {
            if(ev.code == IGNOREKEY_A || ev.code == IGNOREKEY_B || ev.code > KEY_BOUNDARY || ev.code < KEY_OFFSET) continue;
            printf("Key %d pressed, Key code:%d\n", KEY_MAP[ev.code-KEY_OFFSET],ev.code);
        //Key released
        } else if (ev.type == EV_KEY && ev.value == 0) {
            if(ev.code == ENTERKEY){
                //send the code for validation and wipe stored password
                if (myAlarm->password == code){
                    if (myAlarm->current != unarmed) myAlarm->current = unarmed;
                    else myAlarm->current =off;
                } 
                    //if already off, turn on
                printf("Code: %d\n",code);
                pos = code = 0;
            }else{
                if(ev.code == IGNOREKEY_A || ev.code == IGNOREKEY_B || ev.code > KEY_BOUNDARY || ev.code < KEY_OFFSET) continue;
                printf("Key %d released, Key code:%d\n", KEY_MAP[ev.code-KEY_OFFSET],ev.code);
                if(pos < 4){
                    //not allowed 0 for first digit of password
                    if(pos == 0 && KEY_MAP[ev.code-KEY_OFFSET] == 0){
                        continue;
                    }
                    code*=10;
                    code+=KEY_MAP[ev.code-KEY_OFFSET];
                    pos++;
                }
            }
        }
    }
    return NULL;
}

void init(){
    myAlarm = getAlarm();
    fd = open(INPUT_EVENT, O_RDONLY);
    if (fd < 0) {
        perror("Error opening input device");
        exit(1);
    }
}

void Keyboard_scan(void)
{   
    init();
    pthread_create(&scanthread, NULL, &scanner, NULL);
}

void Keyboard_stop(void){
    printf("Keyboard scanning stopped\n");
    killthread = true;
    pthread_cancel(scanthread);
    close(fd);
}

//Comment out main function when merged with project
// int main() {    
//     Keyboard_scan();
//     pthread_join(scanthread,NULL);
//     return 0;
// }
