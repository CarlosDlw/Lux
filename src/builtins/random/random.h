#ifndef TOLLVM_RANDOM_H
#define TOLLVM_RANDOM_H

#include <stdint.h>

// seed(uint64)
void tollvm_seed(uint64_t s);

// seedTime()
void tollvm_seedTime(void);

// randInt() -> int64
int64_t tollvm_randInt(void);

// randIntRange(int64, int64) -> int64
int64_t tollvm_randIntRange(int64_t min, int64_t max);

// randUint() -> uint64
uint64_t tollvm_randUint(void);

// randFloat() -> float64
double tollvm_randFloat(void);

// randFloatRange(float64, float64) -> float64
double tollvm_randFloatRange(double min, double max);

// randBool() -> bool
int32_t tollvm_randBool(void);

// randChar() -> char
uint8_t tollvm_randChar(void);

#endif // TOLLVM_RANDOM_H
