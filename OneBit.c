#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

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

// Embedded Lookup Table for 7-bit ASCII to Binary mapping
typedef struct {
    char binary[8];  // 7-bit binary + 1 for null terminator
    char ascii;      // ASCII character
} LookupEntry;
// Define the lookup structure for 7-bit binary and ASCII
typedef struct {
    char binary[8];  // 7-bit binary string plus the null terminator
    char ascii;      // Corresponding ASCII character
} LookupEntry;

// Lookup table for 7-bit binary to ASCII conversion
LookupEntry lookupTable[] = {
    {"0000000", '\0'},      // NUL
    {"0000001", '\x01'},    // SOH
    {"0000010", '\x02'},    // STX
    {"0000011", '\x03'},    // ETX
    {"0000100", '\x04'},    // EOT
    {"0000101", '\x05'},    // ENQ
    {"0000110", '\x06'},    // ACK
    {"0000111", '\a'},      // BEL
    {"0001000", '\b'},      // BS
    {"0001001", '\t'},      // TAB
    {"0001010", '\n'},      // LF
    {"0001011", '\v'},      // VT
    {"0001100", '\f'},      // FF
    {"0001101", '\r'},      // CR
    {"0001110", '\x0E'},    // SO
    {"0001111", '\x0F'},    // SI
    {"0010000", '\x10'},    // DLE
    {"0010001", '\x11'},    // DC1
    {"0010010", '\x12'},    // DC2
    {"0010011", '\x13'},    // DC3
    {"0010100", '\x14'},    // DC4
    {"0010101", '\x15'},    // NAK
    {"0010110", '\x16'},    // SYN
    {"0010111", '\x17'},    // ETB
    {"0011000", '\x18'},    // CAN
    {"0011001", '\x19'},    // EM
    {"0011010", '\x1A'},    // SUB
    {"0011011", '\x1B'},    // ESC
    {"0011100", '\x1C'},    // FS
    {"0011101", '\x1D'},    // GS
    {"0011110", '\x1E'},    // RS
    {"0011111", '\x1F'},    // US
    {"0100000", ' '},       // Space
    {"0100001", '!'},       // Exclamation mark
    {"0100010", '"'},       // Double quote
    {"0100011", '#'},       // Hash
    {"0100100", '$'},       // Dollar sign
    {"0100101", '%'},       // Percent sign
    {"0100110", '&'},       // Ampersand
    {"0100111", '\''},      // Single quote
    {"0101000", '('},       // Left parenthesis
    {"0101001", ')'},       // Right parenthesis
    {"0101010", '*'},       // Asterisk
    {"0101011", '+'},       // Plus
    {"0101100", ','},       // Comma
    {"0101101", '-'},       // Minus
    {"0101110", '.'},       // Period
    {"0101111", '/'},       // Slash
    {"0110000", '0'},       // Zero
    {"0110001", '1'},       // One
    {"0110010", '2'},       // Two
    {"0110011", '3'},       // Three
    {"0110100", '4'},       // Four
    {"0110101", '5'},       // Five
    {"0110110", '6'},       // Six
    {"0110111", '7'},       // Seven
    {"0111000", '8'},       // Eight
    {"0111001", '9'},       // Nine
    {"0111010", ':'},       // Colon
    {"0111011", ';'},       // Semicolon
    {"0111100", '<'},       // Less than
    {"0111101", '='},       // Equal sign
    {"0111110", '>'},       // Greater than
    {"0111111", '?'},       // Question mark
    {"1000000", '@'},       // At symbol
    {"1000001", 'A'},       // Uppercase A
    {"1000010", 'B'},       // Uppercase B
    {"1000011", 'C'},       // Uppercase C
    {"1000100", 'D'},       // Uppercase D
    {"1000101", 'E'},       // Uppercase E
    {"1000110", 'F'},       // Uppercase F
    {"1000111", 'G'},       // Uppercase G
    {"1001000", 'H'},       // Uppercase H
    {"1001001", 'I'},       // Uppercase I
    {"1001010", 'J'},       // Uppercase J
    {"1001011", 'K'},       // Uppercase K
    {"1001100", 'L'},       // Uppercase L
    {"1001101", 'M'},       // Uppercase M
    {"1001110", 'N'},       // Uppercase N
    {"1001111", 'O'},       // Uppercase O
    {"1010000", 'P'},       // Uppercase P
    {"1010001", 'Q'},       // Uppercase Q
    {"1010010", 'R'},       // Uppercase R
    {"1010011", 'S'},       // Uppercase S
    {"1010100", 'T'},       // Uppercase T
    {"1010101", 'U'},       // Uppercase U
    {"1010110", 'V'},       // Uppercase V
    {"1010111", 'W'},       // Uppercase W
    {"1011000", 'X'},       // Uppercase X
    {"1011001", 'Y'},       // Uppercase Y
    {"1011010", 'Z'},       // Uppercase Z
    {"1011011", '['},       // Left bracket
    {"1011100", '\\'},      // Backslash
    {"1011101", ']'},       // Right bracket
    {"1011110", '^'},       // Caret
    {"1011111", '_'},       // Underscore
    {"1100000", '`'},       // Backtick
    {"1100001", 'a'},       // Lowercase a
    {"1100010", 'b'},       // Lowercase b
    {"1100011", 'c'},       // Lowercase c
    {"1100100", 'd'},       // Lowercase d
    {"1100101", 'e'},       // Lowercase e
    {"1100110", 'f'},       // Lowercase f
    {"1100111", 'g'},       // Lowercase g
    {"1101000", 'h'},       // Lowercase h
    {"1101001", 'i'},       // Lowercase i
    {"1101010", 'j'},       // Lowercase j
    {"1101011", 'k'},       // Lowercase k
    {"1101100", 'l'},       // Lowercase l
    {"1101101", 'm'},       // Lowercase m
    {"1101110", 'n'},       // Lowercase n
    {"1101111", 'o'},       // Lowercase o
    {"1110000", 'p'},       // Lowercase p
    {"1110001", 'q'},       // Lowercase q
    {"1110010", 'r'},       // Lowercase r
    {"1110011", 's'},       // Lowercase s
    {"1110100", 't'},       // Lowercase t
    {"1110101", 'u'},       // Lowercase u
    {"1110110", 'v'},       // Lowercase v
    {"1110111", 'w'},       // Lowercase w
    {"1111000", 'x'},       // Lowercase x
    {"1111001", 'y'},       // Lowercase y
    {"1111010", 'z'},       // Lowercase z
    {"1111011", '{'},       // Left brace
    {"1111100", '|'},       // Vertical bar
    {"1111101", '}'},       // Right brace
    {"1111110", '~'},       // Tilde
    {"1111111", '\x7F'}     // DEL
};

// Function to convert binary string to ASCII character
char binaryToAscii(const char* binary) {
    int ascii = 0;
    for (int i = 0; i < 7; ++i) {
        ascii = ascii * 2 + (binary[i] - '0');  // Convert binary string to integer
    }
    return (char)ascii;  // Return the corresponding ASCII character
}

// Function to convert ASCII character to binary string
void asciiToBinary(char ascii, char* binary) {
    for (int i = 6; i >= 0; --i) {
        binary[6 - i] = (ascii & (1 << i)) ? '1' : '0';  // Extract each bit
    }
    binary[7] = '\0';  // Null terminate the string
}

int main() {
    // Test converting binary to ASCII
    const char* binaryStr = "0100000";  // Space character in binary
    char asciiChar = binaryToAscii(binaryStr);
    printf("Binary %s -> ASCII: '%c'\n", binaryStr, asciiChar);

    // Test converting ASCII to binary
    char inputChar = 'A';
    char binaryStrOut[8];
    asciiToBinary(inputChar, binaryStrOut);
    printf("ASCII '%c' -> Binary: %s\n", inputChar, binaryStrOut);

    return 0;
}


// Function prototypes
void initMT(MTState* state, unsigned long seed);
unsigned long genRandInt32(MTState* state);
const char* asciiToBinary(const char ascii);
const char* binaryToAscii(const char* binary);
void logMessage(const char* message);
void generateInquiry(MTState* state, const char* previousResponse);
void* inquiryWorker(void* arg);
void handleError(const char* message);

// Initialize Mersenne Twister state
void initMT(MTState* state, unsigned long seed) {
    state->mt[0] = seed & 0xffffffffUL;
    for (state->mti = 1; state->mti < N; state->mti++) {
        state->mt[state->mti] = (1812433253UL * (state->mt[state->mti - 1] ^ (state->mt[state->mti - 1] >> 30)) + state->mti);
        state->mt[state->mti] &= 0xffffffffUL;
    }
}

// Generate random number
unsigned long genRandInt32(MTState* state) {
    unsigned long y;
    static unsigned long mag01[2] = {0x0UL, MATRIX_A};
    if (state->mti >= N) {
        int kk;
        for (kk = 0; kk < N - M; kk++) {
            y = (state->mt[kk] & UPPER_MASK) | (state->mt[kk + 1] & LOWER_MASK);
            state->mt[kk] = state->mt[kk + M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (; kk < N - 1; kk++) {
            y = (state->mt[kk] & UPPER_MASK) | (state->mt[kk + 1] & LOWER_MASK);
            state->mt[kk] = state->mt[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (state->mt[N - 1] & UPPER_MASK) | (state->mt[0] & LOWER_MASK);
        state->mt[N - 1] = state->mt[M - 1] ^ (y >> 1) ^ mag01[y & 0x1UL];
        state->mti = 0;
    }
    y = state->mt[state->mti++];
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);
    return y & 0xffffffffUL;
}

// Convert ASCII to 7-bit Binary
const char* asciiToBinary(const char ascii) {
    for (size_t i = 0; i < sizeof(lookupTable) / sizeof(LookupEntry); i++) {
        if (lookupTable[i].ascii == ascii) {
            return lookupTable[i].binary;
        }
    }
    return NULL;  // Return NULL on failure
}

// Convert 7-bit Binary to ASCII
const char* binaryToAscii(const char* binary) {
    for (size_t i = 0; i < sizeof(lookupTable) / sizeof(LookupEntry); i++) {
        if (strcmp(lookupTable[i].binary, binary) == 0) {
            return &lookupTable[i].ascii;
        }
    }
    return NULL;  // Return NULL on failure
}

// Log messages to a file with a timestamp
void logMessage(const char* message) {
    FILE* logFile = fopen(LOG_FILE, "a");
    if (!logFile) {
        fprintf(stderr, "Error opening log file!\n");
        exit(1);
    }
    
    time_t now;
    time(&now);
    struct tm* localTime = localtime(&now);

    fprintf(logFile, "[%02d-%02d-%04d %02d:%02d:%02d] %s\n",
            localTime->tm_mday, localTime->tm_mon + 1, localTime->tm_year + 1900,
            localTime->tm_hour, localTime->tm_min, localTime->tm_sec, message);
    
    fclose(logFile);
}

// Handle errors and log them
void handleError(const char* message) {
    logMessage(message);
    fprintf(stderr, "ERROR: %s\n", message);
    exit(1);
}

// Function to dynamically generate a line of inquiry
void generateInquiry(MTState* state, const char* previousResponse) {
    // Generate a random number to drive the next question
    unsigned long randomNum = genRandInt32(state);
    char nextChar = (char)(randomNum % 26 + 'A');  // Random letter from 'A' to 'Z'
    
    // Convert random number to binary
    const char* binary = asciiToBinary(nextChar);
    if (!binary) {
        handleError("Error converting character to binary.");
    }

    // Generate a question based on randomness and the previous response
    char inquiryMessage[256];
    snprintf(inquiryMessage, sizeof(inquiryMessage), "Question: Based on your response '%s', what do you think about '%c' (Binary: %s)?\n",
             previousResponse, nextChar, binary);
    logMessage(inquiryMessage);
    printf("%s", inquiryMessage);
}

// Worker thread for inquiry generation
void* inquiryWorker(void* arg) {
    MTState* state = (MTState*)arg;
    char previousResponse[] = "Initial Response";  // Simulating the initial response

    // Simulate 5 iterations of spontaneous inquiry generation
    for (int i = 0; i < 5; i++) {
        generateInquiry(state, previousResponse);
        usleep(1000000);  // Pause for a second before generating next inquiry
    }

    return NULL;
}

int main() {
    unsigned long seed = time(NULL);
    MTState mt;
    initMT(&mt, seed);

    printf("Kernel Initialized. Random seed: %lu\n", seed);
    logMessage("Kernel Initialized.");

    // Swarming Threads: Multiple threads simulate parallel inquiry generation
    int numThreads = 2;
    if (numThreads > MAX_THREADS) {
        handleError("Too many threads requested. Max allowed is 16.");
    }

    pthread_t threads[numThreads];

    // Create threads to simulate inquiry generation
    for (int i = 0; i < numThreads; i++) {
        if (pthread_create(&threads[i], NULL, inquiryWorker, (void*)&mt) != 0) {
            handleError("Failed to create thread.");
        }
    }

    // Join all threads
    for (int i = 0; i < numThreads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            handleError("Failed to join thread.");
        }
    }

    logMessage("Program completed successfully.");
    return 0;
}
