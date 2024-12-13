#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <math.h>

#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL
#define UPPER_MASK 0x80000000UL
#define LOWER_MASK 0x7fffffffUL

typedef struct {
    char binary[8];
    char ascii[2];
} LookupEntry;

LookupEntry lookupTable[] = {
    {"0000000", {'\0', '\0'}},      // NUL
    {"0000001", {'\x01', '\0'}},    // SOH
    {"0000010", {'\x02', '\0'}},    // STX
    {"0000011", {'\x03', '\0'}},    // ETX
    {"0000100", {'\x04', '\0'}},    // EOT
    {"0000101", {'\x05', '\0'}},    // ENQ
    {"0000110", {'\x06', '\0'}},    // ACK
    {"0000111", {'\a', '\0'}},      // BEL
    {"0001000", {'\b', '\0'}},      // BS
    {"0001001", {'\t', '\0'}},      // TAB
    {"0001010", {'\n', '\0'}},      // LF
    {"0001011", {'\v', '\0'}},      // VT
    {"0001100", {'\f', '\0'}},      // FF
    {"0001101", {'\r', '\0'}},      // CR
    {"0001110", {'\x0E', '\0'}},    // SO
    {"0001111", {'\x0F', '\0'}},    // SI
    {"0010000", {'\x10', '\0'}},    // DLE
    {"0010001", {'\x11', '\0'}},    // DC1
    {"0010010", {'\x12', '\0'}},    // DC2
    {"0010011", {'\x13', '\0'}},    // DC3
    {"0010100", {'\x14', '\0'}},    // DC4
    {"0010101", {'\x15', '\0'}},    // NAK
    {"0010110", {'\x16', '\0'}},    // SYN
    {"0010111", {'\x17', '\0'}},    // ETB
    {"0011000", {'\x18', '\0'}},    // CAN
    {"0011001", {'\x19', '\0'}},    // EM
    {"0011010", {'\x1A', '\0'}},    // SUB
    {"0011011", {'\x1B', '\0'}},    // ESC
    {"0011100", {'\x1C', '\0'}},    // FS
    {"0011101", {'\x1D', '\0'}},    // GS
    {"0011110", {'\x1E', '\0'}},    // RS
    {"0011111", {'\x1F', '\0'}},    // US
    {"0100000", {' ', '\0'}},       // (space)
    {"0100001", {'!', '\0'}},
    {"0100010", {'"', '\0'}},
    {"0100011", {'#', '\0'}},
    {"0100100", {'$', '\0'}},
    {"0100101", {'%', '\0'}},
    {"0100110", {'&', '\0'}},
    {"0100111", {'\'', '\0'}},
    {"0101000", {'(', '\0'}},
    {"0101001", {')', '\0'}},
    {"0101010", {'*', '\0'}},
    {"0101011", {'+', '\0'}},
    {"0101100", {',', '\0'}},
    {"0101101", {'-', '\0'}},
    {"0101110", {'.', '\0'}},
    {"0101111", {'/', '\0'}},
    {"0110000", {'0', '\0'}},
    {"0110001", {'1', '\0'}},
    {"0110010", {'2', '\0'}},
    {"0110011", {'3', '\0'}},
    {"0110100", {'4', '\0'}},
    {"0110101", {'5', '\0'}},
    {"0110110", {'6', '\0'}},
    {"0110111", {'7', '\0'}},
    {"0111000", {'8', '\0'}},
    {"0111001", {'9', '\0'}},
    {"0111010", {':', '\0'}},
    {"0111011", {';', '\0'}},
    {"0111100", {'<', '\0'}},
    {"0111101", {'=', '\0'}},
    {"0111110", {'>', '\0'}},
    {"0111111", {'?', '\0'}},
    {"1000000", {'@', '\0'}},
    {"1000001", {'A', '\0'}},
    {"1000010", {'B', '\0'}},
    {"1000011", {'C', '\0'}},
    {"1000100", {'D', '\0'}},
    {"1000101", {'E', '\0'}},
    {"1000110", {'F', '\0'}},
    {"1000111", {'G', '\0'}},
    {"1001000", {'H', '\0'}},
    {"1001001", {'I', '\0'}},
    {"1001010", {'J', '\0'}},
    {"1001011", {'K', '\0'}},
    {"1001100", {'L', '\0'}},
    {"1001101", {'M', '\0'}},
    {"1001110", {'N', '\0'}},
    {"1001111", {'O', '\0'}},
    {"1010000", {'P', '\0'}},
    {"1010001", {'Q', '\0'}},
    {"1010010", {'R', '\0'}},
    {"1010011", {'S', '\0'}},
    {"1010100", {'T', '\0'}},
    {"1010101", {'U', '\0'}},
    {"1010110", {'V', '\0'}},
    {"1010111", {'W', '\0'}},
    {"1011000", {'X', '\0'}},
    {"1011001", {'Y', '\0'}},
    {"1011010", {'Z', '\0'}},
    {"1011011", {'[', '\0'}},
    {"1011100", {'\\', '\0'}},
    {"1011101", {']', '\0'}},
    {"1011110", {'^', '\0'}},
    {"1011111", {'_', '\0'}},
    {"1100000", {'`', '\0'}},
    {"1100001", {'a', '\0'}},
    {"1100010", {'b', '\0'}},
    {"1100011", {'c', '\0'}},
    {"1100100", {'d', '\0'}},
    {"1100101", {'e', '\0'}},
    {"1100110", {'f', '\0'}},
    {"1100111", {'g', '\0'}},
    {"1101000", {'h', '\0'}},
    {"1101001", {'i', '\0'}},
    {"1101010", {'j', '\0'}},
    {"1101011", {'k', '\0'}},
    {"1101100", {'l', '\0'}},
    {"1101101", {'m', '\0'}},
    {"1101110", {'n', '\0'}},
    {"1101111", {'o', '\0'}},
    {"1110000", {'p', '\0'}},
    {"1110001", {'q', '\0'}},
    {"1110010", {'r', '\0'}},
    {"1110011", {'s', '\0'}},
    {"1110100", {'t', '\0'}},
    {"1110101", {'u', '\0'}},
    {"1110110", {'v', '\0'}},
    {"1110111", {'w', '\0'}},
    {"1111000", {'x', '\0'}},
    {"1111001", {'y', '\0'}},
    {"1111010", {'z', '\0'}},
    {"1111011", {'{', '\0'}},
    {"1111100", {'|', '\0'}},
    {"1111101", {'}', '\0'}},
    {"1111110", {'~', '\0'}},
    {"1111111", {'\x7F', '\0'}}  // DEL
};

char* asciiToBinary(const char* ascii) {
    for (size_t i = 0; i < sizeof(lookupTable) / sizeof(LookupEntry); i++) {
        if (strcmp(lookupTable[i].ascii, ascii) == 0) {
            return lookupTable[i].binary;
        }
    }
    return NULL;
}

char* binaryToAscii(const char* binary) {
    for (size_t i = 0; i < sizeof(lookupTable) / sizeof(LookupEntry); i++) {
        if (strcmp(lookupTable[i].binary, binary) == 0) {
            return lookupTable[i].ascii;
        }
    }
    return NULL;
}

typedef struct {
    int* rule;
    size_t size;
} Rule;

unsigned long mtState[N];
int mtIndex = N + 1;

void initMT(unsigned long seed) {
    mtState[0] = seed & 0xffffffffUL;
    for (int i = 1; i < N; i++) {
        mtState[i] = (1812433253UL * (mtState[i - 1] ^ (mtState[i - 1] >> 30)) + i);
        mtState[i] &= 0xffffffffUL;
    }
    mtIndex = N;
}

unsigned long genRandInt32() {
    unsigned long y;
    static unsigned long mag01[2] = {0x0UL, MATRIX_A};
    if (mtIndex >= N) {
        int kk;
        if (mtIndex == N + 1)
            initMT(5489UL);
        for (kk = 0; kk < N - M; kk++) {
            y = (mtState[kk] & UPPER_MASK) | (mtState[kk + 1] & LOWER_MASK);
            mtState[kk] = mtState[kk + M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (; kk < N - 1; kk++) {
            y = (mtState[kk] & UPPER_MASK) | (mtState[kk + 1] & LOWER_MASK);
            mtState[kk] = mtState[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mtState[N - 1] & UPPER_MASK) | (mtState[0] & LOWER_MASK);
        mtState[N - 1] = mtState[M - 1] ^ (y >> 1) ^ mag01[y & 0x1UL];
        mtIndex = 0;
    }
    y = mtState[mtIndex++];
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);
    return y;
}

Rule mutate(Rule rule, double stressLevel, gsl_rng *rng) {
    Rule mutatedRule = {malloc(rule.size * sizeof(int)), rule.size};
    for (size_t i = 0; i < rule.size; ++i) {
        mutatedRule.rule[i] = rule.rule[i];
        if (gsl_rng_uniform(rng) < stressLevel) {
            mutatedRule.rule[i] = 1 - mutatedRule.rule[i];  // flip bit
        }
    }
    return mutatedRule;
}

void crossover(Rule parent1, Rule parent2, Rule *child, double stressLevel, gsl_rng *rng) {
    size_t crossoverPoint = gsl_rng_uniform_int(rng, parent1.size);
    for (size_t i = 0; i < crossoverPoint; ++i) {
        child->rule[i] = parent1.rule[i];
    }
    for (size_t i = crossoverPoint; i < parent2.size; ++i) {
        child->rule[i] = parent2.rule[i];
    }
}
 
int main() {
    gsl_rng *rng = gsl_rng_alloc(gsl_rng_mt19937);
    gsl_rng_set(rng, (unsigned long)time(NULL));

    Rule parent1 = {malloc(8 * sizeof(int)), 8};
    Rule parent2 = {malloc(8 * sizeof(int)), 8};
    Rule child = {malloc(8 * sizeof(int)), 8};

    // Initialize rules
    for (size_t i = 0; i < parent1.size; ++i) {
        parent1.rule[i] = rand() % 2;
        parent2.rule[i] = rand() % 2;
    }

    // Print parent rules
    printf("Parent 1: ");
    for (size_t i = 0; i < parent1.size; ++i) {
        printf("%d", parent1.rule[i]);
    }
    printf("\n");

    printf("Parent 2: ");
    for (size_t i = 0; i < parent2.size; ++i) {
        printf("%d", parent2.rule[i]);
    }
    printf("\n");

    // Set a stress level (e.g., 1.0 means normal stress)
    double stressLevel = 1.0;

    // Apply mutation and crossover
    Rule mutatedParent1 = mutate(parent1, stressLevel, rng);
    Rule mutatedParent2 = mutate(parent2, stressLevel, rng);

    printf("Mutated Parent 1: ");
    for (size_t i = 0; i < mutatedParent1.size; ++i) {
        printf("%d", mutatedParent1.rule[i]);
    }
    printf("\n");

    printf("Mutated Parent 2: ");
    for (size_t i = 0; i < mutatedParent2.size; ++i) {
        printf("%d", mutatedParent2.rule[i]);
    }
    printf("\n");

    crossover(mutatedParent1, mutatedParent2, &child, stressLevel, rng);

    printf("Child: ");
    for (size_t i = 0; i < child.size; ++i) {
        printf("%d", child.rule[i]);
    }
    printf("\n");

    // Free allocated memory
    free(parent1.rule);
    free(parent2.rule);
    free(mutatedParent1.rule);
    free(mutatedParent2.rule);
    free(child.rule);
    gsl_rng_free(rng);

    return 0;
}
