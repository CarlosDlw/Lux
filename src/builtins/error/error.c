#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Thread-local exception handler stack
static __thread lux_eh_frame* eh_stack = NULL;

void lux_eh_push(lux_eh_frame* frame) {
    frame->prev   = eh_stack;
    frame->active = 0;
    memset(&frame->error, 0, sizeof(lux_error));
    eh_stack      = frame;
}

void lux_eh_pop(void) {
    if (eh_stack) {
        eh_stack = eh_stack->prev;
    }
}

void lux_eh_throw(const lux_error* err) {
    if (!eh_stack) {
        // No handler — print and abort
        fprintf(stderr, "unhandled error: %.*s\n  at %.*s:%d:%d\n",
                (int)err->message_len, err->message_ptr,
                (int)err->file_len,    err->file_ptr,
                err->line, err->column);
        exit(1);
    }

    eh_stack->error  = *err;
    eh_stack->active = 1;
    longjmp(eh_stack->buf, 1);
}

lux_eh_frame* lux_eh_current(void) {
    return eh_stack;
}

void* lux_eh_get_jmpbuf(lux_eh_frame* frame) {
    return (void*)frame->buf;
}

lux_error* lux_eh_get_error(lux_eh_frame* frame) {
    return &frame->error;
}

lux_eh_frame* lux_eh_alloc(void) {
    return (lux_eh_frame*)calloc(1, sizeof(lux_eh_frame));
}

void lux_eh_free(lux_eh_frame* frame) {
    free(frame);
}
