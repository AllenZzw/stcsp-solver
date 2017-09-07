#ifndef __UTIL_H
#define __UTIL_H

#ifdef __cplusplus
#include <cstdlib>
#include <cstdarg>
#endif

#ifndef __cplusplus
#include <stdio.h>
#include <stdarg.h>
#endif

#define MIN(a, b)       ((a) < (b) ? (a) : (b))
#define MAX(a, b)       ((a) > (b) ? (a) : (b))

#define MY_INT_MAX      (2147483647)
#define MY_INT_MIN      (-2147483647 - 1)

#define MEMORY 1

#if MEMORY
extern unsigned long mallocCount;
extern unsigned long freeCount;
#endif

void *myMalloc2(size_t size, const char *func);
void myFree2(void *ptr, const char *func);
#define myMalloc(size) myMalloc2(size, __func__)
#define myFree(ptr) myFree2(ptr, __func__)
void freeMemory();
void backup(int *addr);
void levelUp();
void levelDown();

double cpuTime();

#define LOG_ERROR   1
#define LOG_WARNING 2
#define LOG_INFO    3
#define LOG_CONFIG  4
#define LOG_DEBUG   5
#define LOG_TRACE   6
#define LOG_DETAILED_TRACE 7

extern int logLevel;

void myLog(int level, char *format, ...);

/*#ifdef DEBUG
#define myLog(level, ...) _myLog((level), ##__VA_ARGS__)
#else
#define myLog(...) ((void) 0)
#endif*/

#endif
