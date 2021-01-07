#ifndef TERMINAL_H
#define TERMINAL_H 1
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <errno.h>
#include <termios.h>

int getCursorPosition(unsigned int *, unsigned int *);
void getWindowSize(unsigned int *, unsigned int *);
void enableRAWMode(void);
void disableRAWMode(void);
#endif //For TERMINAL_H
