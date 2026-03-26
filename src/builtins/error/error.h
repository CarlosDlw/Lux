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
} tollvm_error;

// Exception handler stack frame
typedef struct tollvm_eh_frame {
    jmp_buf                  buf;
    struct tollvm_eh_frame*  prev;
    tollvm_error             error;
    int                      active;  // 1 if an error was thrown
} tollvm_eh_frame;

// Push/pop exception handler frames
void  tollvm_eh_push(tollvm_eh_frame* frame);
void  tollvm_eh_pop(void);

// Throw an error — longjmps to the nearest handler
void  tollvm_eh_throw(const tollvm_error* err);

// Get the current exception handler (for setjmp)
tollvm_eh_frame* tollvm_eh_current(void);

// Get jmp_buf pointer from a frame (for setjmp call)
void* tollvm_eh_get_jmpbuf(tollvm_eh_frame* frame);

// Get error struct pointer from a frame (for reading caught error)
tollvm_error* tollvm_eh_get_error(tollvm_eh_frame* frame);

// Allocate a frame on the heap (platform-independent sizing)
tollvm_eh_frame* tollvm_eh_alloc(void);

// Free a heap-allocated frame
void tollvm_eh_free(tollvm_eh_frame* frame);
