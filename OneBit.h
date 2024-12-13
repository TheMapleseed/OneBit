#ifndef KERNEL_H
#define KERNEL_H

#include <pthread.h>
#include <stdlib.h>

// Mersenne Twister (MT19937) parameters
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL
#define UPPER_MASK 0x80000000UL
#define LOWER_MASK 0x7fffffffUL

// Maximum number of threads (swarm size)
#define MAX_THREADS 16

// Log file for debug and production information
#define LOG_FILE "kernel_log.txt"

// Structure for representing the Mersenne Twister state
typedef struct {
    unsigned long mt[N];
    int mti;
} MTState;

// Structure for binary-to-ASCII conversion lookup
typedef struct {
    char binary[8];  // 7-bit binary string + null terminator
    char ascii;      // Corresponding ASCII character
} LookupEntry;

// Lookup table for 7-bit binary to ASCII conversion
extern LookupEntry lookupTable[];

// Function prototypes
void initMT(MTState* state, unsigned long seed);
unsigned long genRandInt32(MTState* state);
const char* asciiToBinary(const char ascii);
const char* binaryToAscii(const char* binary);
void logMessage(const char* message);
void handleError(const char* message);
void generateInquiry(MTState* state, const char* previousResponse);
void* inquiryWorker(void* arg);

#endif // KERNEL_H
