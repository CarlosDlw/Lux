#include "thread.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ── Task implementation ───────────────────────────────────────────────── */

struct tollvm_Task {
    pthread_t thread;
    void*     result;
    int       completed;
};

tollvm_Task* tollvm_taskCreate(void* (*fn)(void*), void* arg) {
    tollvm_Task* t = (tollvm_Task*)calloc(1, sizeof(tollvm_Task));
    pthread_create(&t->thread, NULL, fn, arg);
    return t;
}

void* tollvm_taskAwait(tollvm_Task* task) {
    if (!task) return NULL;
    if (!task->completed) {
        void* retval;
        pthread_join(task->thread, &retval);
        task->result    = retval;
        task->completed = 1;
    }
    return task->result;
}

void tollvm_taskFree(tollvm_Task* task) {
    if (task) free(task);
}

/* ── Mutex implementation ──────────────────────────────────────────────── */

struct tollvm_Mutex {
    pthread_mutex_t mtx;
};

tollvm_Mutex* tollvm_mutexCreate(void) {
    tollvm_Mutex* m = (tollvm_Mutex*)malloc(sizeof(tollvm_Mutex));
    pthread_mutex_init(&m->mtx, NULL);
    return m;
}

void tollvm_mutexLock(tollvm_Mutex* m) {
    if (m) pthread_mutex_lock(&m->mtx);
}

void tollvm_mutexUnlock(tollvm_Mutex* m) {
    if (m) pthread_mutex_unlock(&m->mtx);
}

int32_t tollvm_mutexTryLock(tollvm_Mutex* m) {
    if (!m) return 0;
    return pthread_mutex_trylock(&m->mtx) == 0 ? 1 : 0;
}

void tollvm_mutexFree(tollvm_Mutex* m) {
    if (m) {
        pthread_mutex_destroy(&m->mtx);
        free(m);
    }
}

/* ── Utilities ─────────────────────────────────────────────────────────── */

uint32_t tollvm_cpuCount(void) {
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    return n > 0 ? (uint32_t)n : 1;
}

uint64_t tollvm_threadId(void) {
    return (uint64_t)pthread_self();
}

void tollvm_yield(void) {
    sched_yield();
}
