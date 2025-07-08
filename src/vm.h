#ifndef VM_H
#define VM_H

#include <stdbool.h>

#define STACK_SIZE 256
#define MEMORY_SIZE 1024


#define BINARY_OP(op) \
	do { \
		int b = pop(); \
		int a = pop(); \
		push(a op b);\
	} while(0) 

#define CMP_OP(op) \
	do { \
		int val2 = pop(); \
		int val1 = pop(); \
		int addr = program[++ip]; \
		if(val1 op val2){ ip = addr; return;} \
		else{ip++;} \
	} while(0)

#define LOG_BITWISE_OP(op) \
	do { \
		int b= pop(); \
		int a = pop(); \
		push(a op b); \
	} while(0)

#define SHIFT_BITS(op)\
	do{\
		int shift = program[ip]; \
		int val = pop(); \
		push(val >> shift); \
	}while(0)




// Opcodes
typedef enum{
	PSH,
	POP,
	MOV,
	HLT,

	JMP,
	JZ,
	JE,
	JNE,
	JNZ,
	JL,
	JG,
	JLE, 
	JGE,
	CMP,
	SET,
	GET,
	ADD,
	SUB,
	MUL,
	DIV,
	MOD,
	LOG,
	PEEK,
	

	DUP,
	CALL,
	RET,

	AND, 
	OR, 
	NOT,
	XOR, 

	SHL, 
	SHR,
	
	LOG_AND, 
	LOG_OR, 
	LOG_NOT,
	LOG_XOR,

	STORE, 
	LOAD, 
	ALLOC, 
	FREE,
	


	
} InstructionSet;

typedef enum{

	A,B,C,D,E,F, IP, SP, NUM_OF_REGISTERS

} Registers;

typedef struct Block{
	int start;
	int size;
	bool used;
	struct Block* next;
} Block;



// External state
extern int stack[STACK_SIZE];
extern int sp;
extern int ip;
extern bool running;
extern const int* program;



// VM API
void eval(int instr);
void run_vm(const int* program, int program_len);
int pop();
void push(int val);
void init_memory();

#endif
