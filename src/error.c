// error.c
#include <stdio.h>
#include <stdbool.h>

#include "error.h"

// You’ll include this variable in your main VM file
extern bool running;
VMError last_error = ERR_NONE;

const char* vm_error_message(VMError err) {
    switch (err) {
        case ERR_NONE: return "No error";
        case ERR_STACK_OVERFLOW: return "Stack overflow";
        case ERR_STACK_UNDERFLOW: return "Stack underflow";
        case ERR_INVALID_MEMORY: return "Invalid memory access";
        case ERR_UNKNOWN_OPCODE: return "Unknown opcode";
        case ERR_ALLOC_FAIL: return "Memory allocation failed";
        case ERR_DIV_ZERO: return "Division by zero";
        case ERR_FREE_INVALID: return "Invalid free address";
        case ERR_BLOCK_ALREADY_FREE: return "Block already free";
        case ERR_NO_FREE_SPACE: return "No free space for allocation";
        case ERR_PANIC: return "Generic panic";
        default: return "Unknown error";
    }
}

void panic(VMError err) {
    last_error = err;
    printf("PANIC: %s\n", vm_error_message(err));
    running = false;
}
