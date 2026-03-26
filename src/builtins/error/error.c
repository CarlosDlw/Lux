#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Thread-local exception handler stack
static __thread tollvm_eh_frame* eh_stack = NULL;

void tollvm_eh_push(tollvm_eh_frame* frame) {
    frame->prev   = eh_stack;
    frame->active = 0;
    memset(&frame->error, 0, sizeof(tollvm_error));
    eh_stack      = frame;
}

void tollvm_eh_pop(void) {
    if (eh_stack) {
        eh_stack = eh_stack->prev;
    }
}

void tollvm_eh_throw(const tollvm_error* err) {
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

tollvm_eh_frame* tollvm_eh_current(void) {
    return eh_stack;
}

void* tollvm_eh_get_jmpbuf(tollvm_eh_frame* frame) {
    return (void*)frame->buf;
}

tollvm_error* tollvm_eh_get_error(tollvm_eh_frame* frame) {
    return &frame->error;
}

tollvm_eh_frame* tollvm_eh_alloc(void) {
    return (tollvm_eh_frame*)calloc(1, sizeof(tollvm_eh_frame));
}

void tollvm_eh_free(tollvm_eh_frame* frame) {
    free(frame);
}
