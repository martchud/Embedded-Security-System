// Module for UDP:
// Supports running commands over netcat     
// and communication to server by nodejs
#ifndef _UDP_H_
#define _UDP_H_
#include <stdbool.h>
// Begin/end the thread controlling communication via UDP
void udp_startSampling(void);
void udp_stopSampling(void);
bool udp_shutDown(void);

#endif