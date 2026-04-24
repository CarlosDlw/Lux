#ifndef LUX_RANDOM_H
#define LUX_RANDOM_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
	const char* ptr;
	size_t len;
} lux_random_str_result;

// seed(uint64)
void lux_seed(uint64_t s);

// seedTime()
void lux_seedTime(void);

// randInt() -> int64
int64_t lux_randInt(void);

// randIntRange(int64, int64) -> int64
int64_t lux_randIntRange(int64_t min, int64_t max);

// randUint() -> uint64
uint64_t lux_randUint(void);

// randFloat() -> float64
double lux_randFloat(void);

// randFloatRange(float64, float64) -> float64
double lux_randFloatRange(double min, double max);

// randBool() -> bool
int32_t lux_randBool(void);

// randChar() -> char
uint8_t lux_randChar(void);

// uuid_v4() -> string
lux_random_str_result lux_uuid_v4(void);

#endif // LUX_RANDOM_H
