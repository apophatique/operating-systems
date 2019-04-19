#ifndef INCLUDE_UTIL_H_
#define INCLUDE_UTIL_H_

#include <asm/ioctls.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>

short get_height_of_term();

long get_random(long min, long max);

#endif  // INCLUDE_UTIL_H_
