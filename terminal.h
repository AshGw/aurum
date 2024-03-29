#include <errno.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#ifndef TERMINAL_H
#define TERMINAL_H 1

int getCursorPosition(unsigned int *, unsigned int *);
void getWindowSize(unsigned int *, unsigned int *);
void enableRAWMode(void);
void disableRAWMode(void);
#endif // For TERMINAL_H