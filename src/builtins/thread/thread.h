#ifndef LUX_THREAD_H
#define LUX_THREAD_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Task ──────────────────────────────────────────────────────────────── */
typedef struct lux_Task lux_Task;

/* Create a task that runs `fn(arg)` on a new thread.
   `fn` must return a heap-allocated result pointer (or NULL for void). */
lux_Task* lux_taskCreate(void* (*fn)(void*), void* arg);

/* Block until the task finishes, return the result pointer.
   The caller owns the returned pointer and must free() it. */
void* lux_taskAwait(lux_Task* task);

/* Free the task handle (call after await). */
void lux_taskFree(lux_Task* task);

/* ── Mutex ─────────────────────────────────────────────────────────────── */
typedef struct lux_Mutex lux_Mutex;

lux_Mutex* lux_mutexCreate(void);
void          lux_mutexLock(lux_Mutex* m);
void          lux_mutexUnlock(lux_Mutex* m);
int32_t       lux_mutexTryLock(lux_Mutex* m);
void          lux_mutexFree(lux_Mutex* m);

/* ── Utilities ─────────────────────────────────────────────────────────── */
uint32_t lux_cpuCount(void);
uint64_t lux_threadId(void);
void     lux_yield(void);

#ifdef __cplusplus
}
#endif

#endif /* LUX_THREAD_H */
