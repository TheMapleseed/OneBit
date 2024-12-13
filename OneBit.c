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

// Function to apply mutation with a dynamic rate based on stress level
Rule mutate(Rule rule, double stressLevel, gsl_rng *rng) {
    double mutationRate = fmin(0.05 * stressLevel, 0.2);  // Higher stress, higher mutation
    Rule mutated = { .size = rule.size, .rule = (int *)malloc(rule.size * sizeof(int)) };
    if (mutated.rule == NULL) {
        fprintf(stderr, "Memory allocation failed for mutated rule\n");
        exit(1);  // Exit if memory allocation fails
    }
    for (size_t i = 0; i < rule.size; ++i) {
        mutated.rule[i] = rule.rule[i];
        double randomValue = gsl_rng_uniform(rng);
        if (randomValue < mutationRate) {
            mutated.rule[i] = 1 - mutated.rule[i];  // Flip bit if mutation occurs
        }
    }
    return mutated;
}

// Crossover with higher exploration when stress level is high
void crossover(Rule parent1, Rule parent2, Rule *child, double stressLevel, gsl_rng *rng) {
    size_t crossoverPoint = rand() % parent1.size;
    double crossoverRate = fmin(0.5 * stressLevel, 1.0);  // Higher stress encourages more crossover
    for (size_t i = 0; i < parent1.size; ++i) {
        if (i < crossoverPoint) {
            child->rule[i] = parent1.rule[i];
        } else {
            child->rule[i] = parent2.rule[i];
        }
        // Additional randomization based on stress level
        if (gsl_rng_uniform(rng) < crossoverRate) {
            child->rule[i] = 1 - child->rule[i];  // Flip bit for exploration
        }
    }
}

int main() {
    srand(time(NULL));  // Initialize the random number generator

    // Allocate and initialize RNG
    gsl_rng *rng = gsl_rng_alloc();
    if (rng == NULL) {
        fprintf(stderr, "Failed to allocate RNG\n");
        return -1;
    }

    // Create two parent rules (6 bits)
    Rule parent1 = { .size = 6, .rule = (int *)malloc(6 * sizeof(int)) };
    Rule parent2 = { .size = 6, .rule = (int *)malloc(6 * sizeof(int)) };
    Rule child = { .size = 6, .rule = (int *)malloc(6 * sizeof(int)) };
    if (parent1.rule == NULL || parent2.rule == NULL || child.rule == NULL) {
        fprintf(stderr, "Memory allocation failed for rules\n");
        gsl_rng_free(rng);
        return -1;
    }

    // Initialize parent rules randomly
    for (size_t i = 0; i < 6; ++i) {
        parent1.rule[i] = rand() % 2;
        parent2.rule[i] = rand() % 2;
    }

    printf("Parent 1: ");
    for (size_t i = 0; i < 6; ++i) {
        printf("%d ", parent1.rule[i]);
    }
    printf("\n");

    printf("Parent 2: ");
    for (size_t i = 0; i < 6; ++i) {
        printf("%d ", parent2.rule[i]);
    }
    printf("\n");

    // Perform crossover and mutation based on stress level
    double stressLevel = 1.0;  // Can be adjusted dynamically
    crossover(parent1, parent2, &child, stressLevel, rng);

    printf("Child (after crossover): ");
    for (size_t i = 0; i < 6; ++i) {
        printf("%d ", child.rule[i]);
    }
    printf("\n");

    Rule mutatedChild = mutate(child, stressLevel, rng);
    printf("Mutated Child: ");
    for (size_t i = 0; i < 6; ++i) {
        printf("%d ", mutatedChild.rule[i]);
    }
    printf("\n");

    gsl_rng_free(rng);
    free(parent1.rule);
    free(parent2.rule);
    free(child.rule);
    free(mutatedChild.rule);

    return 0;
}

