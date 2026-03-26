#include "thread.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ── Task implementation ───────────────────────────────────────────────── */

struct lux_Task {
    pthread_t thread;
    void*     result;
    int       completed;
};

lux_Task* lux_taskCreate(void* (*fn)(void*), void* arg) {
    lux_Task* t = (lux_Task*)calloc(1, sizeof(lux_Task));
    pthread_create(&t->thread, NULL, fn, arg);
    return t;
}

void* lux_taskAwait(lux_Task* task) {
    if (!task) return NULL;
    if (!task->completed) {
        void* retval;
        pthread_join(task->thread, &retval);
        task->result    = retval;
        task->completed = 1;
    }
    return task->result;
}

void lux_taskFree(lux_Task* task) {
    if (task) free(task);
}

/* ── Mutex implementation ──────────────────────────────────────────────── */

struct lux_Mutex {
    pthread_mutex_t mtx;
};

lux_Mutex* lux_mutexCreate(void) {
    lux_Mutex* m = (lux_Mutex*)malloc(sizeof(lux_Mutex));
    pthread_mutex_init(&m->mtx, NULL);
    return m;
}

void lux_mutexLock(lux_Mutex* m) {
    if (m) pthread_mutex_lock(&m->mtx);
}

void lux_mutexUnlock(lux_Mutex* m) {
    if (m) pthread_mutex_unlock(&m->mtx);
}

int32_t lux_mutexTryLock(lux_Mutex* m) {
    if (!m) return 0;
    return pthread_mutex_trylock(&m->mtx) == 0 ? 1 : 0;
}

void lux_mutexFree(lux_Mutex* m) {
    if (m) {
        pthread_mutex_destroy(&m->mtx);
        free(m);
    }
}

/* ── Utilities ─────────────────────────────────────────────────────────── */

uint32_t lux_cpuCount(void) {
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    return n > 0 ? (uint32_t)n : 1;
}

uint64_t lux_threadId(void) {
    return (uint64_t)pthread_self();
}

void lux_yield(void) {
    sched_yield();
}
