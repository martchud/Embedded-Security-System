// Joystick related modes on the zen cape
// Including a backup to turn alarm on or off
#ifndef JOYSTICK_H
#define JOYSTICK_H

#define gpio_mid    "/sys/class/gpio/gpio27/value"
#define EXPORT_FILE "/sys/class/gpio/export"
#define mid_dir     "/sys/class/gpio/gpio27/direction"

// Start/Stop thread that actively listens to the input of the joystick. 
// if there is input it will respond to the established functionality
void Joystick_startListening(void);
void Joystick_stopListening(void);

#endif