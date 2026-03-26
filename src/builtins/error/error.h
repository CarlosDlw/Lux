#pragma once

#include <stdint.h>
#include <stddef.h>
#include <setjmp.h>

// Error struct layout — must match LLVM struct { {ptr,usize}, {ptr,usize}, i32, i32 }
typedef struct {
    const char* message_ptr;
    size_t      message_len;
    const char* file_ptr;
    size_t      file_len;
    int32_t     line;
    int32_t     column;
} lux_error;

// Exception handler stack frame
typedef struct lux_eh_frame {
    jmp_buf                  buf;
    struct lux_eh_frame*  prev;
    lux_error             error;
    int                      active;  // 1 if an error was thrown
} lux_eh_frame;

// Push/pop exception handler frames
void  lux_eh_push(lux_eh_frame* frame);
void  lux_eh_pop(void);

// Throw an error — longjmps to the nearest handler
void  lux_eh_throw(const lux_error* err);

// Get the current exception handler (for setjmp)
lux_eh_frame* lux_eh_current(void);

// Get jmp_buf pointer from a frame (for setjmp call)
void* lux_eh_get_jmpbuf(lux_eh_frame* frame);

// Get error struct pointer from a frame (for reading caught error)
lux_error* lux_eh_get_error(lux_eh_frame* frame);

// Allocate a frame on the heap (platform-independent sizing)
lux_eh_frame* lux_eh_alloc(void);

// Free a heap-allocated frame
void lux_eh_free(lux_eh_frame* frame);
