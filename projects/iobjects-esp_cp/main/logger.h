/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

/* debug level:
 * 0 - disabled
 * 1 - only message
 * 2 - filename + function + line + message
 */
#define DEBUG_LEVEL                                                           1

/*****************************************************************************/

#undef info
#undef warning
#undef error

#define __FILENAME__ (strrchr(__FILE__, '/') + 1)
#define __RED__      "\x1b[31m"
#define __GREEN__    "\x1b[32m"
#define __YELLOW__   "\x1b[33m"
#define __DEFAULT__  "\x1b[0m"

#if (DEBUG_LEVEL == 0)
#define info(FORMAT,...)
#define warning(FORMAT,...)
#define error(FORMAT,...)
#elif (DEBUG_LEVEL == 1)
#define info(FORMAT,...)    printf((__GREEN__  "[I] " FORMAT __DEFAULT__ "\n"), ##__VA_ARGS__)
#define warning(FORMAT,...) printf((__YELLOW__ "[W] " FORMAT __DEFAULT__ "\n"), ##__VA_ARGS__)
#define error(FORMAT,...)   printf((__RED__    "[E] " FORMAT __DEFAULT__ "\n"), ##__VA_ARGS__)
#elif (DEBUG_LEVEL == 2)
#define info(FORMAT,...)    printf((__GREEN__  "[I][%s %s() %d] " FORMAT __DEFAULT__ "\n"), __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define warning(FORMAT,...) printf((__YELLOW__ "[W][%s %s() %d] " FORMAT __DEFAULT__ "\n"), __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define error(FORMAT,...)   printf((__RED__    "[E][%s %s() %d] " FORMAT __DEFAULT__ "\n"), __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

/*****************************************************************************/
