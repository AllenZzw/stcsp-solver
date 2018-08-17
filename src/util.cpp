#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <unistd.h>
#include <sys/time.h>
#include <sys/times.h>
#include "util.h"

#if MEMORY
unsigned long mallocCount = 0;
unsigned long freeCount = 0;
#endif

/* exported */
void *myMalloc2(size_t size, const char *func) {
    void *ptr;

    ptr = malloc(size);
    if (ptr == NULL) {
        myLog(LOG_ERROR, "Error: not enough memory! Malloc-ed in %s\n", func);
        exit(1);
    }
    myLog(LOG_DETAILED_TRACE, "Malloc-ed in %s\n", func);
#if MEMORY
    mallocCount++;
#endif

    return ptr;
}

/* exported */
void myFree2(void *ptr, const char *func) {
    if (ptr != NULL) {
#if MEMORY
        freeCount++;
#endif
        free(ptr);
        myLog(LOG_DETAILED_TRACE, "Free-ed in %s\n", func);
    }
}

/*****
 * Backup and Restore
 */

#define CHUNK_SIZE 1024

typedef struct _Chunk {
    int **addr;
    int *data;
    int ptr;
    struct _Chunk *prev;
    struct _Chunk *next;
} Chunk;

Chunk *memory = NULL;
int level = 0;

Chunk *newChunk() {
    Chunk *chunk;

    chunk = (Chunk *)myMalloc(sizeof(Chunk));
    chunk->addr = (int **)myMalloc(sizeof(int *) * CHUNK_SIZE);
    chunk->data = (int *)myMalloc(sizeof(int) * CHUNK_SIZE);
    chunk->ptr = 0;
    chunk->prev = NULL;
    chunk->next = NULL;

    return chunk;
}

void freeMemory() {
    Chunk *chunk, *temp;

    chunk = memory;
    if(chunk != NULL){
        while (chunk->prev != NULL) {
            chunk = chunk->prev;
        }

        while (chunk != NULL) {
            myFree(chunk->addr);
            myFree(chunk->data);
            temp = chunk->next;
            myFree(chunk);
            chunk = temp;
        }
    }
}

/* exported */
//backup? backup the address of data ?
void backup(int *addr) {
    if (memory == NULL) {
        memory = newChunk();
    }

    if (addr == NULL) {
        memory->addr[memory->ptr] = NULL;
        memory->data[memory->ptr] = 0;
    } else {
        memory->addr[memory->ptr] = addr;
        memory->data[memory->ptr] = *addr;
    }
    memory->ptr++;

    if (memory->ptr == CHUNK_SIZE) { /* chunk full, get another chunk */
        if (memory->next == NULL) {
            memory->next = newChunk();
            memory->next->prev = memory;
        }
        memory = memory->next;
    }
}

void printLevel() {
    int i;

    myLog(LOG_TRACE, "%02d:", level);
    for (i = 0; i < level; i++) {
        myLog(LOG_TRACE, "        ");
    }
}

/* exported */
void levelUp() {
    level++;
    backup(NULL);
}

/* exported */
void levelDown() {
    level--;
    if (memory->ptr == 0) {
        memory = memory->prev;
    }
    memory->ptr--;
    while (memory->addr[memory->ptr] != NULL) {
        *memory->addr[memory->ptr] = memory->data[memory->ptr];
        if (memory->ptr == 0) {
            memory = memory->prev;
        }
        memory->ptr--;
    }
}

/* Time */
double cpuTime() {
    static struct tms buf;

    times(&buf);

    return ((double)(buf.tms_utime + buf.tms_stime + buf.tms_cutime + buf.tms_cstime)) / ((double)sysconf(_SC_CLK_TCK));
}

// int logLevel = LOG_DEBUG;
int logLevel = LOG_ERROR;
int color[] = { 0, 31, 33, 37, 35, 32, 36 };

void myLog(int level, char *format, ...) {
    va_list args;
    FILE *stream;
    if (level <= logLevel) {
        stream = fopen("error.txt", "a+"); //(level == LOG_ERROR ? stderr : stdout);
        va_start(args, format);
        //if (isatty(1)) {
        //    fprintf(stream, "%c[%d;%d;%dm", 0x1b, 0, color[level], 40);
        //}
        vfprintf(stream, format, args);
        //if (isatty(1)) {
        //    fprintf(stream, "%c[%dm", 0x1b, 0);
        //}
        va_end(args);
        fflush(stream);
        fclose(stream);
    }
}
