#include <alsa/asoundlib.h>
#include "include/audioBuffer.h"
#include "include/helpers.h"
#include <pthread.h>
#include <stdbool.h>
#include "include/audioControl.h"
#include "include/motion.h"
// #include "include/alarm_interface.h"

// #define ALARM "wave-files/ALARM_SHORT.WAV"
// #define alarm2 "wave-files/alarm2.wav"

#define BEEP_WARN "/mnt/remote/myApps/wave-files/beep.wav"
#define ALARM "/mnt/remote/myApps/wave-files/mainAlarm2.wav"

// static Alarm* myAlarmSystem;

static wavedata_t alarmFile;
static wavedata_t alarm2File;

static bool shutdown = false;

// int currentMode = 1;

// void *playBeats(void *args){
// 	printf("Starting audio thread!\n");
// 	myAlarmSystem = getAlarm();
// 	while(!shutdown){

// 		if (myAlarmSystem->current == 0 || myAlarmSystem->current == 1){
// 			// play no sound
// 		}
// 		else if (myAlarmSystem->current == 2 && currentMode == 1){
// 			//beep
// 			AudioMixer_queueSound(&alarm2File);
// 			sleepForMs(3000);		//beeps every 3 seconds
// 		}
// 		else if (myAlarmSystem->current == 3 && currentMode == 1){
// 			AudioMixer_queueSound(&alarmFile);
// 			sleepForMs(32050);
// 		}

// 		//...
// 	}
// 	return NULL;
// }

//temp!
void playSound(int type){

	if (type == 1){
		AudioMixer_queueSound(&alarm2File);
	}
	else if (type == 2){
		AudioMixer_queueSound(&alarmFile);
	}
}

// void setCurrentMode(int num){
// 	currentMode = num;
// }

static pthread_t alarmthread;
void Audio_player_start(void)
{
	setOn();
	AudioMixer_init();
	AudioMixer_readWaveFileIntoMemory(ALARM, &alarmFile);
	AudioMixer_readWaveFileIntoMemory(BEEP_WARN, &alarm2File);
	// pthread_create(&alarmthread, NULL, &playBeats, NULL);
}

void Audio_player_shutDown(){
	shutdown = true;
	pthread_join(alarmthread,NULL);
	AudioMixer_cleanup();
	AudioMixer_freeWaveFileData(&alarmFile);
}
