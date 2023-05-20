#include "include/udp.h"
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdio.h>
#include <limits.h>
#include <netdb.h>
#include <unistd.h>	
#include <float.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "include/alarm_interface.h"
#include "include/camera.h"

#define BIND_PORT 12345      // RUN: netcat -u 192.168.7.2 12345    
#define MSG_MAX_LEN 1024

//keywords for each commandtype
#define CMD_HELP "help\n"    
#define CMD_GET_STATUS "getStatus"
#define CMD_SET_MODE "setMode"
#define CMD_STOP "stop"
#define CMD_ARM "arm"
#define CMD_DISARM "disarm"
#define CMD_RESET "reset"
#define CMD_QR "QR"
#define ENTER '\n'

// mode setting options
// #define MODE_ARM         "arm"
// #define MODE_DISARM      "disarm"

// Status options
// #define STATUS_ARMED "armed"
// #define STATUS_DISARMED "disarmed"
// #define STATUS_ALERTED "alerted"
// #define STATUS_SOUNDING "sounding"

static pthread_t udpThreadID;

static struct sockaddr_in sock;
static int socketDescriptor;
static bool isConnected;
static socklen_t sock_sz;

static Alarm* myAlarmSystem;

static void* udpCommandThread(void *vargp);
static void initServer(void);

static bool shutDown = false;

bool udp_shutDown(void){
    return shutDown;
}

void udp_startSampling(void)
{
    printf("UDP Thread Starting At Port %i\n", BIND_PORT);
    initServer();
    myAlarmSystem = getAlarm();
    isConnected = true;

    pthread_create(&udpThreadID, NULL, &udpCommandThread, NULL);
}

void udp_stopSampling(void)
{
    printf("UDP Thread Stopping!\n");

    isConnected = false;
    pthread_cancel(udpThreadID);
    pthread_join(udpThreadID, NULL);
    close(socketDescriptor);

}

static void initServer(void)
{
    // Socket Setup
    memset(&sock, 0, sizeof(sock));
    sock.sin_family = AF_INET;
    sock.sin_addr.s_addr = htonl(INADDR_ANY);
    sock.sin_port = htons(BIND_PORT);

    socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    if (socketDescriptor == -1)
    {
        perror("UDP SERVER ERROR: Can't create socket\n");
        exit(1);
    }

    if (bind(socketDescriptor, (struct sockaddr*) &sock, sizeof(sock)) == -1)
    {
        perror("UDP SERVER ERROR: Failed to bind socket\n");
        exit(1);
    }
    sock_sz = sizeof(sock);

    return;
}

static void* udpCommandThread(void *vargp)
{
    char recvBuffer[MSG_MAX_LEN];
    char cmdHistory[MSG_MAX_LEN];
    char sendBuffer[MSG_MAX_LEN];

    while(isConnected){ 
        // Read incoming message
        int bytesRx = recvfrom(socketDescriptor, recvBuffer, MSG_MAX_LEN, 0, (struct sockaddr*)&sock, &sock_sz);
        if (bytesRx < 0 ){
            printf("UDP SERVER ERROR: Could not read incoming message\n");
            exit(1);
        } 
        recvBuffer[bytesRx] = '\0';

        // call prev command if 'enter'
        if (recvBuffer[0] == ENTER) {
            strncpy(recvBuffer, cmdHistory, MSG_MAX_LEN);   
        }

        //store history for enter command
        strncpy(cmdHistory,recvBuffer,MSG_MAX_LEN);         

        // _________Message Handling_________
        // get alarm status
        if(strncmp(recvBuffer, CMD_GET_STATUS, strlen(CMD_GET_STATUS)) == 0){
            enum Alarmstate state = myAlarmSystem->current;
            snprintf(sendBuffer, MSG_MAX_LEN, "%i", state);
        }
        // else if(strncmp(recvBuffer, CMD_SET_MODE, strlen(CMD_SET_MODE)) == 0){
        //     int mode = atoi(recvBuffer + strlen(CMD_SET_MODE) + 1);
        //     printf(" setting alarm to mode %i\n", mode);
        //     // todo: set the alarm mode
        //     myAlarmSystem->current = (enum Alarmstate) mode;
        //     snprintf(sendBuffer, MSG_MAX_LEN,"success");
        // }
        // Arm system
        else if(strncmp(recvBuffer, CMD_RESET, strlen(CMD_RESET)) == 0){
            int password = atoi(recvBuffer + strlen(CMD_RESET) + 1);
            int new_password = atoi(recvBuffer + strlen(CMD_RESET) +1+4+1);
            if(password == myAlarmSystem->password){
                Password_update((short) new_password);
                snprintf(sendBuffer, MSG_MAX_LEN,"success");
            }else{
                snprintf(sendBuffer, MSG_MAX_LEN,"incorrect_password");
            }
        }
        else if(strncmp(recvBuffer, CMD_ARM, strlen(CMD_ARM)) == 0){
            int password = atoi(recvBuffer + strlen(CMD_ARM) + 1);
            if(password == myAlarmSystem->password){
                myAlarmSystem->current = off;
                snprintf(sendBuffer, MSG_MAX_LEN,"success");
            }else{
                snprintf(sendBuffer, MSG_MAX_LEN,"incorrect_password");
            }
        }
        else if(strncmp(recvBuffer, CMD_DISARM, strlen(CMD_DISARM)) == 0){
            int password = atoi(recvBuffer + strlen(CMD_DISARM) + 1);
            if(password == myAlarmSystem->password){
                myAlarmSystem->current = off;
                snprintf(sendBuffer, MSG_MAX_LEN,"success");
            }else{
                snprintf(sendBuffer, MSG_MAX_LEN,"incorrect_password");
            }
        }
        else if(strncmp(recvBuffer, CMD_QR, strlen(CMD_QR)) == 0){
            myAlarmSystem->current = off;
            snprintf(sendBuffer, MSG_MAX_LEN,"deactivated");
        }
        else if(strncmp(recvBuffer, CMD_STOP, strlen(CMD_STOP)) == 0){
            // todo: handle shutdown
            snprintf(sendBuffer, MSG_MAX_LEN,"success");
            isConnected = false;
            Camera_stop();
            // sendto(socketDescriptor,sendBuffer, strnlen(sendBuffer,MSG_MAX_LEN),0,(struct sockaddr *) &sock, sock_sz);
            Alarm_Shutdown();
            shutDown = true;

        }
        // help message
        else if(strncmp(recvBuffer, CMD_HELP, strlen(CMD_HELP)) == 0){
            sprintf(sendBuffer, "Accepted command examples:\n");
			strcat(sendBuffer, "getStatus   -- Get alarm state. {0 = off, 1 = triggered, 2 = alerted}.\n");
            // strcat(sendBuffer, "setMode ####        -- Change state of alarm system 0=disarm, 1=arm\n");
			strcat(sendBuffer, "arm ###     -- Allow motion detection to trigger the alarm states\n");
			strcat(sendBuffer, "disarm #### -- Disarm alarm system\n");
			strcat(sendBuffer, "stop        -- Cause the server program to end.\n");
            strcat(sendBuffer, "<enter>     -- Repeat last command.\n");
        }
        // Invalid message
        else{
            snprintf(sendBuffer, MSG_MAX_LEN,"invalid command");
        }

        //send message
        sendto(socketDescriptor,sendBuffer, strnlen(sendBuffer,MSG_MAX_LEN),0,(struct sockaddr *) &sock, sock_sz);

        // reset buffers
        memset(recvBuffer, 0, MSG_MAX_LEN);
        memset(sendBuffer, 0, MSG_MAX_LEN);
    }
    return 0;
}
