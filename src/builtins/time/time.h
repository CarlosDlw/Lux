#ifndef LUX_TIME_H
#define LUX_TIME_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    const char* ptr;
    size_t len;
} lux_time_str_result;

// now() -> uint64  (milliseconds since epoch)
uint64_t lux_now(void);

// nowNanos() -> uint64
uint64_t lux_nowNanos(void);

// nowMicros() -> uint64
uint64_t lux_nowMicros(void);

// sleep(uint64)  — milliseconds
void lux_sleep(uint64_t ms);

// sleepMicros(uint64)
void lux_sleepMicros(uint64_t us);

// clock() -> uint64  — CPU clock ticks
uint64_t lux_clock(void);

// year() -> int32
int32_t lux_year(void);

// month() -> int32  (1-12)
int32_t lux_month(void);

// day() -> int32  (1-31)
int32_t lux_day(void);

// hour() -> int32  (0-23)
int32_t lux_hour(void);

// minute() -> int32  (0-59)
int32_t lux_minute(void);

// second() -> int32  (0-59)
int32_t lux_second(void);

// weekday() -> int32  (0=Sunday, 6=Saturday)
int32_t lux_weekday(void);

// timestamp() -> string  (ISO 8601)
lux_time_str_result lux_timestamp(void);

// elapsed(uint64) -> uint64  (ms elapsed since given timestamp)
uint64_t lux_elapsed(uint64_t since);

// formatTime(uint64, string) -> string
lux_time_str_result lux_formatTime(uint64_t ms,
                                         const char* fmt_ptr, size_t fmt_len);

// parseTime(string, string) -> uint64
uint64_t lux_parseTime(const char* str_ptr, size_t str_len,
                          const char* fmt_ptr, size_t fmt_len);

#endif // LUX_TIME_H
