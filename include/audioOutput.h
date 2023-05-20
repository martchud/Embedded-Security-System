#ifndef _AUDIOOUTPUT_H_
#define _AUDIOOUTPUT_H_
//
void Audio_player_start(void);
void *playBeats(void *args);
void Audio_player_shutDown();
void setCurrentMode(int num);
void playSound(int type);
#endif
