/**
Module for capture frames with camera
Sends frame data to a node server to be displayed as a livestream
Detects motion enter from left side of camera
**/
#ifndef _CAMERA_H_
#define _CAMERA_H_
#include <stdbool.h>
void Camera_start(bool);
void Camera_stop(void);
#endif
