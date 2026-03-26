#include "random/random.h"
#include <stdlib.h>
#include <time.h>

// xoshiro256** state
static uint64_t rng_state[4] = {0};
static int rng_initialized = 0;

static uint64_t rotl(uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

static uint64_t splitmix64(uint64_t* state) {
    uint64_t z = (*state += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

static void ensure_initialized(void) {
    if (!rng_initialized) {
        tollvm_seedTime();
    }
}

static uint64_t next_random(void) {
    ensure_initialized();
    uint64_t result = rotl(rng_state[1] * 5, 7) * 9;
    uint64_t t = rng_state[1] << 17;
    rng_state[2] ^= rng_state[0];
    rng_state[3] ^= rng_state[1];
    rng_state[1] ^= rng_state[2];
    rng_state[0] ^= rng_state[3];
    rng_state[2] ^= t;
    rng_state[3] = rotl(rng_state[3], 45);
    return result;
}

void tollvm_seed(uint64_t s) {
    uint64_t sm = s;
    rng_state[0] = splitmix64(&sm);
    rng_state[1] = splitmix64(&sm);
    rng_state[2] = splitmix64(&sm);
    rng_state[3] = splitmix64(&sm);
    rng_initialized = 1;
}

void tollvm_seedTime(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    uint64_t s = (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
    tollvm_seed(s);
}

int64_t tollvm_randInt(void) {
    return (int64_t)next_random();
}

int64_t tollvm_randIntRange(int64_t min, int64_t max) {
    if (min >= max) return min;
    uint64_t range = (uint64_t)(max - min) + 1;
    uint64_t r = next_random();
    return min + (int64_t)(r % range);
}

uint64_t tollvm_randUint(void) {
    return next_random();
}

double tollvm_randFloat(void) {
    return (double)(next_random() >> 11) * 0x1.0p-53;
}

double tollvm_randFloatRange(double min, double max) {
    if (min >= max) return min;
    return min + tollvm_randFloat() * (max - min);
}

int32_t tollvm_randBool(void) {
    return (next_random() & 1) ? 1 : 0;
}

uint8_t tollvm_randChar(void) {
    // Printable ASCII: 32 ('space') to 126 ('~') = 95 chars
    return (uint8_t)(32 + (next_random() % 95));
}
