#include <alsa/asoundlib.h>
#include "include/audioBuffer.h"
#include "include/audioOutput.h"
#include <stdbool.h>

bool AudioControl = false;

void setOn(){
AudioControl = true;
}

void setOff(){
AudioControl = false;
}

bool getStatus(){
return AudioControl;
}
