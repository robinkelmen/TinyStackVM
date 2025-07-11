// error.h
#ifndef ERROR_H
#define ERROR_H

typedef enum {
    ERR_NONE,
    ERR_STACK_OVERFLOW,
    ERR_STACK_UNDERFLOW,
    ERR_INVALID_MEMORY,
    ERR_UNKNOWN_OPCODE,
    ERR_ALLOC_FAIL,
    ERR_DIV_ZERO,
    ERR_FREE_INVALID,
    ERR_BLOCK_ALREADY_FREE,
    ERR_NO_FREE_SPACE,
    ERR_PANIC
} VMError;

const char* vm_error_message(VMError err);
void panic(VMError err);

#endif
