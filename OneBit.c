#include <stdio.h>
#include <stdlib.h>
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
    unsigned long mt[N];  // The state vector
    int mti;              // Index into mt[] for the next random value
} gsl_rng;

gsl_rng* gsl_rng_alloc() {
    gsl_rng *rng = (gsl_rng*)malloc(sizeof(gsl_rng));
    if (rng != NULL) {
        rng->mt[0] = time(NULL);  // Seed with the current time
        for (rng->mti = 1; rng->mti < N; rng->mti++) {
            rng->mt[rng->mti] = (1812433253UL * (rng->mt[rng->mti-1] ^ (rng->mt[rng->mti-1] >> 30)) + rng->mti);
            rng->mt[rng->mti] &= 0xffffffffUL;  // 32-bit integer
        }
    }
    return rng;
}

void gsl_rng_free(gsl_rng *rng) {
    if (rng != NULL) {
        free(rng);
    }
}

// Tempering function to extract a random value
unsigned long gsl_rng_extract(gsl_rng *rng) {
    unsigned long y;
    unsigned long mag01[2] = {0x0UL, MATRIX_A};
    if (rng->mti >= N) {
        int kk;
        for (kk = 0; kk < N - M; kk++) {
            y = (rng->mt[kk] & UPPER_MASK) | (rng->mt[kk+1] & LOWER_MASK);
            rng->mt[kk] = rng->mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (; kk < N - 1; kk++) {
            y = (rng->mt[kk] & UPPER_MASK) | (rng->mt[kk+1] & LOWER_MASK);
            rng->mt[kk] = rng->mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (rng->mt[N-1] & UPPER_MASK) | (rng->mt[0] & LOWER_MASK);
        rng->mt[N-1] = rng->mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];
        rng->mti = 0;
    }

    y = rng->mt[rng->mti++];
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);
    
    return y;
}

// Generate a random number between 0 and 1
double gsl_rng_uniform(gsl_rng *rng) {
    return (double)(gsl_rng_extract(rng) >> 5) * (1.0 / 9007199254740992.0);
}

// Define a simple rule struct
typedef struct {
    int *rule;
    size_t size;
} Rule;

// Mutation function
Rule mutate(Rule rule, double mutationRate, gsl_rng *rng) {
    Rule mutated = { .size = rule.size, .rule = (int *)malloc(rule.size * sizeof(int)) };
    for (size_t i = 0; i < rule.size; ++i) {
        mutated.rule[i] = rule.rule[i];
        double randomValue = gsl_rng_uniform(rng);  // Use inline RNG
        if (randomValue < mutationRate) {
            mutated.rule[i] = 1 - mutated.rule[i];  // Flip bit if mutation occurs
        }
    }
    return mutated;
}

// Function to print the rule
void printRule(Rule rule) {
    printf("[ ");
    for (size_t i = 0; i < rule.size; ++i) {
        printf("%d ", rule.rule[i]);
    }
    printf("]\n");
}

// Example of fitness evaluation (random)
int evaluateFitness(Rule rule) {
    return rand() % 2 == 0 ? 1 : -1; // Random fitness evaluation (placeholder)
}

// Main function demonstrating mutation
int main() {
    srand(time(NULL));  // Initialize the random number generator

    // Allocate and initialize RNG
    gsl_rng *rng = gsl_rng_alloc();
    if (rng == NULL) {
        fprintf(stderr, "Failed to allocate RNG\n");
        return -1;
    }

    // Create a sample rule (6 bits)
    Rule rule = { .size = 6, .rule = (int *)malloc(6 * sizeof(int)) };
    for (size_t i = 0; i < 6; ++i) {
        rule.rule[i] = rand() % 2;  // Initialize with random 0 or 1
    }

    printf("Original Rule: ");
    printRule(rule);  // Print original rule

    // Perform mutation with a rate of 0.05 (5% mutation chance)
    double mutationRate = 0.05;
    Rule mutatedRule = mutate(rule, mutationRate, rng);

    printf("Mutated Rule: ");
    printRule(mutatedRule);  // Print mutated rule

    // Evaluate fitness of the mutated rule
    int fitness = evaluateFitness(mutatedRule);
    printf("Fitness of mutated rule: %d\n", fitness);

    // Free memory
    free(rule.rule);
    free(mutatedRule.rule);
    gsl_rng_free(rng);

    return 0;
}
