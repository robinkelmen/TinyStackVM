#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "vm.h"
#include "error.h"   

int ip = 0;
int sp = -1;

int stack[STACK_SIZE];
int memory[MEMORY_SIZE];
bool running = true;


const int* program;

Block* head = NULL;

void init_memory() {
    head = malloc(sizeof(Block));
    if (head) {
        head->start = 0;
        head->size = MEMORY_SIZE;
        head->used = false;
        head->next = NULL;
    } else {
        panic(ERR_ALLOC_FAIL);
    }
}

int pop() {
    if (sp < 0) {
        panic(ERR_STACK_UNDERFLOW);
        return 0;
    }
    return stack[sp--];
}

void push(int val) {
    if (sp >= STACK_SIZE - 1) {
        panic(ERR_STACK_OVERFLOW);
        return;
    }
    stack[++sp] = val;
}

bool push_memory(int index, int val) {
    if (index < 0 || index >= MEMORY_SIZE) {
        panic(ERR_INVALID_MEMORY);
        return false;
    }
    memory[index] = val;
    return true;
}

bool get_memory(int index, int* out_val) {
    if (index < 0 || index >= MEMORY_SIZE) {
        panic(ERR_INVALID_MEMORY);
        return false;
    }
    *out_val = memory[index];
    return true;
}

void print_blocks() {
    Block* current = head;
    printf("=== Blocks ===\n");
    while (current) {
        printf("[start=%d size=%d used=%d] -> \n",
            current->start, current->size, current->used);
        current = current->next;
    }
    printf("NULL\n");
}

int fetch() {
    return program[ip];
}

void eval(int instr) {
	switch (instr) {

	case HLT: { running = false; print_blocks(); break; }

	case PSH: { push(program[++ip]); break; }

	case POP: { printf("popped %d\n", pop()); break; }

	case ADD: { BINARY_OP(+); break; }

	case SUB: { BINARY_OP(-); break; }

	case DIV: { 
		int b = pop();
		if (b == 0) {
			panic(ERR_DIV_ZERO);
			return;
		}
		int a = pop();
		push(a / b);
		break; 
	}

	case MUL: { BINARY_OP(*); break; }

	case MOD: { BINARY_OP(%); break; }

	case SWAP: {
		if (sp < 1) {
			panic(ERR_STACK_UNDERFLOW);
			return;
		}
		int tmp = stack[sp - 1];
		stack[sp - 1] = stack[sp];
		stack[sp] = tmp;
		break;
	}

	case JMP: {
		ip++;
		ip = program[ip];
		return;
	}

	case JNZ: {
		int val = pop();
		int addr = program[++ip];
		if (val != 0) {
			ip = addr;
			return;
		} else {
			ip++;
		}
		break;
	}

	case JZ: {
		int val = pop();
		int addr = program[++ip];
		if (val == 0) {
			ip = addr;
			return;
		} else {
			ip++;
		}
		break;
	}

	case JE: { CMP_OP(==); break; }
	case JNE: { CMP_OP(!=); break; }
	case JL: { CMP_OP(<); break; }
	case JG: { CMP_OP(>); break; }
	case JLE: { CMP_OP(<=); break; }
	case JGE: { CMP_OP(>=); break; }

	case CMP: {
		int val2 = pop();
		int val1 = pop();
		int cmp = (val1 > val2) - (val1 < val2);
		push(cmp);
		break;
	}

	case LOG: {
		int val = pop();
		printf("LOG: %d\n", val);
		break;
	}

	case SET: {
		int index = pop();
		int val = pop();
		push_memory(index, val);
		break;
	}

	case GET: {
		int index = pop();
		int out_val = 0;
		if (get_memory(index, &out_val)) {
			push(out_val);
		}
		break;
	}

	case DUP: {
		if (sp < 0) {
			panic(ERR_STACK_UNDERFLOW);
			return;
		}
		int val = stack[sp];
		push(val);
		break;
	}

	case CALL: {
		int addr = program[++ip];
		push(ip + 1);
		ip = addr;
		return;
	}

	case RET: {
		int return_addr = pop();
		printf("ret addr: %d\n", return_addr);
		ip = return_addr;
		return;
	}

	case PEEK: {
		printf("PEEK: %d \n", stack[sp]);
		break;
	}

	case LOG_AND: { LOG_BITWISE_OP(&&); break; }
	case LOG_OR:  { LOG_BITWISE_OP(||); break; }
	case LOG_XOR: {
		int b = pop();
		int a = pop();
		push(!!a != !!b);
		break;
	}
	case LOG_NOT: {
		int val = pop();
		push(!val);
		break;
	}

	case AND: { LOG_BITWISE_OP(&); break; }
	case OR:  { LOG_BITWISE_OP(|); break; }
	case XOR: { LOG_BITWISE_OP(^); break; }
	case NOT: {
		int val = pop();
		push(~val);
		break;
	}

	case SHL: { SHIFT_BITS(<<); break; }
	case SHR: { SHIFT_BITS(>>); break; }

	case ALLOC: {
		int size = pop();
		int base = -1;
		Block* current = head;

		if (size <= 0) {
			panic(ERR_ALLOC_FAIL);
			push(-1);
			return;
		}

		while (current != NULL) {
			if (!current->used && current->size >= size) {
				if (current->size == size) {
					current->used = true;
					base = current->start;
					break;
				} else {
					Block* leftover = malloc(sizeof(Block));
					if (!leftover) {
						panic(ERR_ALLOC_FAIL);
						return;
					}
					leftover->start = current->start + size;
					leftover->size = current->size - size;
					leftover->used = false;
					leftover->next = current->next;

					current->size = size;
					current->used = true;
					current->next = leftover;

					base = current->start;
					break;
				}
			}
			current = current->next;
		}

		if (base == -1) {
			panic(ERR_NO_FREE_SPACE);
			push(-1);
		} else {
			push(base);
		}

		print_blocks();
		break;
	}

	case FREE: {
		int base = pop();
		Block* current = head;

		while (current && current->start != base) {
			current = current->next;
		}

		if (!current) {
			panic(ERR_FREE_INVALID);
			return;
		}
		if (!current->used) {
			panic(ERR_BLOCK_ALREADY_FREE);
			return;
		}

		current->used = false;

		if (current->next && !current->next->used) {
			Block* next = current->next;
			current->size += next->size;
			current->next = next->next;
			free(next);
		}

		Block* prev = NULL;
		Block* tmp = head;

		while (tmp && tmp != current) {
			prev = tmp;
			tmp = tmp->next;
		}

		if (prev && !prev->used) {
			prev->size += current->size;
			prev->next = current->next;
			free(current);
		}

		break;
	}

	case STORE: {
		int val = pop();
		int offset = pop();
		int base = pop();

		int addr = base + offset;

		if (addr < 0 || addr >= MEMORY_SIZE) {
			panic(ERR_INVALID_MEMORY);
			return;
		}
		push_memory(addr, val);
		break;
	}

	case LOAD: {
		int offset = pop();
		int base = pop();

		int addr = base + offset;

		if (addr < 0 || addr >= MEMORY_SIZE) {
			panic(ERR_INVALID_MEMORY);
			push(0);
			return;
		}
		push(memory[addr]);
		break;
	}

	default: {
		panic(ERR_UNKNOWN_OPCODE);
		return;
	}
	}

	ip++;
}


void run_vm(const int* prog, int prog_len) {
    sp = -1;
    ip = 0;
    running = true;
    last_error = ERR_NONE;
    program = prog;

    while (running && ip < prog_len) {
        eval(program[ip]);
    }

    if (last_error != ERR_NONE) {
        printf("❌ VM halted with error: %s \n", vm_error_message(last_error));
    } else {
         printf("✅ SUccess!\n");
    }
}
