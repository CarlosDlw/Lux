#ifndef TOLLVM_THREAD_H
#define TOLLVM_THREAD_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Task ──────────────────────────────────────────────────────────────── */
typedef struct tollvm_Task tollvm_Task;

/* Create a task that runs `fn(arg)` on a new thread.
   `fn` must return a heap-allocated result pointer (or NULL for void). */
tollvm_Task* tollvm_taskCreate(void* (*fn)(void*), void* arg);

/* Block until the task finishes, return the result pointer.
   The caller owns the returned pointer and must free() it. */
void* tollvm_taskAwait(tollvm_Task* task);

/* Free the task handle (call after await). */
void tollvm_taskFree(tollvm_Task* task);

/* ── Mutex ─────────────────────────────────────────────────────────────── */
typedef struct tollvm_Mutex tollvm_Mutex;

tollvm_Mutex* tollvm_mutexCreate(void);
void          tollvm_mutexLock(tollvm_Mutex* m);
void          tollvm_mutexUnlock(tollvm_Mutex* m);
int32_t       tollvm_mutexTryLock(tollvm_Mutex* m);
void          tollvm_mutexFree(tollvm_Mutex* m);

/* ── Utilities ─────────────────────────────────────────────────────────── */
uint32_t tollvm_cpuCount(void);
uint64_t tollvm_threadId(void);
void     tollvm_yield(void);

#ifdef __cplusplus
}
#endif

#endif /* TOLLVM_THREAD_H */
