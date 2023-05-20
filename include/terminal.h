// terminal.h
// Module to print important information to the screen.
// 
#ifndef _TERMINAL_H_
#define _TERMINAL_H_

//stop looping the thread for terminal (udp quit)
void Terminal_quit();

// Begin/end the background thread which prints program information to screen
void Terminal_startPrinting(void);
void Terminal_stopPrinting(void);

#endif