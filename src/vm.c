
#include <stdbool.h> 
#include <stdio.h>
#include <stdlib.h>
#include "vm.h"






int ip = 0;
int sp = -1;

int stack[STACK_SIZE];
int memory[MEMORY_SIZE];
bool running = true;
int registers[NUM_OF_REGISTERS];
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
    printf("Failed to init memory\n");
  }
}

int pop() {

	if (sp < 0) {
		printf("Stack underflow!\n");
		running = false;
		return 0;
	}

	return stack[sp--];
}

void push(int val) {
	
	if (sp >= STACK_SIZE - 1) {
		printf("Stack overflow!\n");
		running = false;
		return;
	}
	stack[++sp] = val;
	printf("pushed val %d and current sp: %d\n", val, sp);
}

bool push_memory(int index, int val) {
	if (index < 0 || index >= MEMORY_SIZE) {
		printf("SET: Invalid memory index %d\n", index);
		return false;
	}

	memory[index] = val;

	return true;
}


bool get_memory(int index, int* out_val) {
	if (index < 0 || index >= MEMORY_SIZE) {
		printf("GET: Invalid memory index %d\n", index);
		return false; // or some sentinel value
	}

	printf("get mem: val: %d \n", memory[index]);
	*out_val = memory[index];
	return true;
}






int fetch(){

	return program[ip];
}





void eval(int instr){
	switch (instr){

	case HLT:running = false; break;

	case PSH: { push(program[++ip]); break;}

	case POP: { printf("popped %d\n", pop()); break;}

	case ADD: { BINARY_OP(+); break; }

	case SUB:{ BINARY_OP(-); break;}

	case DIV:{ BINARY_OP(/); break;}

	case MUL:{ BINARY_OP(*); break; }

	case MOD:{ BINARY_OP(%); break;}

	case JMP:{ 
		ip++;
		ip = program[ip];
		 return;
		}

	case JNZ:{
		int val = pop();
		int addr = program[++ip];
		if (val != 0){
			ip = addr;
			return;
		}else{
			ip++;
		}
		break;
	}

	case JZ:{
		int val = pop();
		int addr = program[++ip];
		if (val == 0){
			ip = addr;
			return;
		}else{
			ip++;
		}
		break;
	}

	case JE:{
		CMP_OP(==);
		break;
	}
	case JNE:{
		CMP_OP(!=);
		break;
	}
	case JL:{
		CMP_OP(<);
		break;
	}
	case JG:{
		CMP_OP(>);
		break;
	}
	case JLE:{
		CMP_OP(<=);
		break;
	}
	case JGE:{
		CMP_OP(>=);
		break;
	}
	case CMP:{
		int val2 = pop();
		int val1 = pop();

		int cmp = (val1 > val2) - (val1 < val2);
		push(cmp);
		break;

	}

	case LOG: {
		printf("SP: %d \n", sp);
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
	case GET:{
		int index = pop();
		int out_val = 0;


		if(get_memory(index, &out_val)){
			push(out_val);
		}
		break;
	}
	case DUP: {

		int val = stack[sp];
		push(val);
		break;

	}
	case CALL: {
		int addr = program[++ip];
		push(ip+1);
		ip = addr;
		return;

	}

	case RET: {
		int return_addr = pop();
		printf("ret addr: %d\n", return_addr);
		ip = return_addr;
		return;

	}
	case PEEK:{
		printf("PEEK: %d \n", stack[sp]);
		break;
	}

	case LOG_AND:{

		LOG_BITWISE_OP(&&);
		break;

	}

	case LOG_OR:{

		LOG_BITWISE_OP(||);
		break;

	}

	case LOG_XOR:{

		int b = pop();
		int a= pop();

		push(!!a !=  !!b);


		break;

	}
	case LOG_NOT:{
		int val = pop();
		push(!val);

		break;

	}

	case AND:{

		LOG_BITWISE_OP(&);

		break;

	}

	case OR:{

		
		LOG_BITWISE_OP(|);


		break;

	}

	case XOR:{


		LOG_BITWISE_OP(^);
		break;

	}
	case NOT:{
		int val = pop();
		push(~val);

		break;

	}

	case SHL:{
		SHIFT_BITS(<<);
		break;
	}

	case SHR:{
		SHIFT_BITS(>>);
		break;

	}

	case ALLOC:{
		int size = pop();
		int base = -1;
		Block* current = head;


		while(current != NULL){

			if(!current->used && current->size >= size){
				
				if(current->size == size){
					current->used = true;
					base = current->start;
					break;
				}else{

						Block* leftover = malloc(sizeof(Block));
						if (!leftover){
							printf("ALLOC: Failed to allocate leftover block");
							return;
						}
						leftover->start = current->start + size+1; // What if start is 0?
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
	

		


		if (base == -1){
			printf("ALLOC: No free space \n");
		}

		push(base);

		current = head;
			printf("---- Current Blocks ----\n");
			while (current) {
			    printf("[start: %d, size: %d, used: %s]\n",
			           current->start, current->size, current->used ? "true" : "false");
			    current = current->next;
			}
			printf("------------------------\n");
	break;
	}

	case FREE:{

		
	}

	case STORE: {

		int offset = pop();
		int base = pop();
		int val = pop();

		int addr = base + offset;

		if (addr < 0 || addr >= MEMORY_SIZE){
			printf("STORE: Address out of bounds \n");
			return;
		}
		//memory[addr] = val;

		push_memory(addr,val);
	}

	case LOAD: {
		int offset = pop();
		int base = pop();

		int addr = base + offset;

		if (addr < 0 || addr >= MEMORY_SIZE){
			printf("LAOD: Address out of bounds \n");
			push(0); //think about error handling convention
			return;
		}
		push(memory[addr]);

	}


	default:{
		printf("Unknown instruction: %d\n", instr);
		running = false;
		return;
	}
	}
	ip++;
}








void run_vm(const int* prog, int prog_len) {
	sp = -1;
	ip = 0;
	running = true;
	program = prog;

	while (running && ip < prog_len) {
		eval(program[ip]);
	}
}


