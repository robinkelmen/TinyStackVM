#include <stdio.h>
#include "vm.h"
#include "error.h"



#define ASSERT_TOP(expected) do { \
	if (sp < 0) { \
		printf("ASSERTION FAILED: stack is empty (expected %d)\n", expected); \
	} else if (stack[sp] != expected) { \
		printf("ASSERTION FAILED: expected top %d, got %d\n", expected, stack[sp]); \
	} else { \
		printf("ASSERT PASSED: top == %d\n", expected); \
	} \
} while (0)
 

void run_test(const char* name, const int* prog, int len) {
	printf("=== Running %s ===\n", name);
	init_memory();
	run_vm(prog, len);
	//ASSERT_TOP(expected);
	printf("\n");
}

int main() {

	const int test_jmp[] = {
		PSH, 1,
		JMP, 6,
		PSH, 999, POP, HLT,
		PSH, 42, LOG, HLT
	};

	const int test_jnz[] = {
		PSH, 7,
		JNZ, 6,
		PSH, 999, PSH, 69, POP, HLT,
		PSH, 42, LOG, HLT
	};

	const int test_jz[] = {
		PSH, 0,
		JZ, 6,
		PSH, 999, PSH, 420, POP, HLT,
		PSH, 42, LOG, HLT
	};

	const int test_je[] = {
		PSH, 5, PSH, 5,
		JE, 6,
		PSH, 999, LOG, HLT,
		PSH, 42, LOG, HLT
	};

	const int test_set_get[] = {
		PSH, 99,  // value
		PSH, 10,  // address
		SET,      // memory[10] = 99
		PSH, 10,  // address
		GET,      // push memory[10]
		LOG,      // should log 99
		HLT
	};

	const int test_cmp[] = {
		PSH, 5,
		PSH, 10,
		CMP,      // pushes -1
		LOG,      // expect -1
		PSH, 10,
		PSH, 10,
		CMP,      // pushes 0
		LOG,      // expect 0
		PSH, 15,
		PSH, 10,
		CMP,      // pushes 1
		LOG,      // expect 1
		HLT
	};

	const int test_call_ret[] = {
	// Main code
	CALL, 6,         // Call function at instruction index 6
	PSH, 99,         // Push 99 after return
	LOG,             // Should log 99
	HLT,

	// Subroutine (at index 6)
	PSH, 42,         // Push 42
	LOG,             // Should log 42
	RET              // Return to after CALL (i.e., PSH 99)
};

// Test for LOG_AND: both true
const int test_log_and[] = {
    PSH, 1,
    PSH, 1,
    LOG_AND,
    LOG,       // Should log 1
    HLT
};

// Test for LOG_AND: one false
const int test_log_and_false[] = {
    PSH, 1,
    PSH, 0,
    LOG_AND,
    LOG,       // Should log 0
    HLT
};

// Test for LOG_OR: one true
const int test_log_or[] = {
    PSH, 0,
    PSH, 42,
    LOG_OR,
    LOG,       // Should log 1
    HLT
};

// Test for LOG_XOR: true XOR false
const int test_log_xor[] = {
    PSH, 1,
    PSH, 0,
    LOG_XOR,
    LOG,       // Should log 1
    HLT
};

// Test for LOG_XOR: true XOR true
const int test_log_xor_false[] = {
    PSH, 1,
    PSH, 1,
    LOG_XOR,
    LOG,       // Should log 0
    HLT
};

// Test for LOG_NOT
const int test_log_not[] = {
    PSH, 0,
    LOG_NOT,
    LOG,       // Should log 1
    PSH, 5,
    LOG_NOT,
    LOG,       // Should log 0
    HLT
};

// Test for bitwise AND
const int test_and[] = {
    PSH, 6,   // 0110
    PSH, 3,   // 0011
    AND,
    LOG,     // Should log 2 (0010)
    HLT
};

// Test for bitwise OR
const int test_or[] = {
    PSH, 6,   // 0110
    PSH, 3,   // 0011
    OR,
    LOG,     // Should log 7 (0111)
    HLT
};

// Test for bitwise XOR
const int test_xor[] = {
    PSH, 6,   // 0110
    PSH, 3,   // 0011
    XOR,
    LOG,     // Should log 5 (0101)
    HLT
};

// Test for bitwise NOT
const int test_not[] = {
    PSH, 0,
    NOT,
    LOG,     // Should log ~0 (depends on int size)
    HLT
};

const int test_alloc_basic[] = {
    PSH, 100,    // Push size 100
    ALLOC,       // Allocate 100 bytes → pushes base
    LOG,         // Should log 0 (first base should be 0)
    HLT
};

const int test_alloc_twice[] = {
    PSH, 100,    // First alloc
    ALLOC,
    LOG,         // Should log 0

    PSH, 200,    // Second alloc
    ALLOC,
    LOG,         // Should log 100 (next block starts at 100)
    HLT
};

const int test_alloc_fail[] = {
    PSH, 1024,
    ALLOC,
    LOG,    // Should log 0 (alloc whole memory)

    PSH, 1,
    ALLOC,
    LOG,    // Should log -1 (fail — no space left)
    HLT
};

const int test_alloc_store_load_free[] = {
    // === alloc A ===
    PSH, 100,      // size = 100
    ALLOC,         // alloc → base0
    DUP,           // keep base0 for later
    PEEK,          // peek base0 (should be 0)

    // === alloc B ===
    PSH, 200,
    ALLOC,         // alloc → base1
    DUP,           // keep base1
    PEEK,          // peek base1 (should be 100)

    // === store value 42 at base1 + 0 ===
    DUP,           // keep base1
    PSH, 0,        // offset
    PSH, 42,       // value
    STORE,         // memory[base1 + 0] = 42

    // === load it back ===
    DUP,           // keep base1
    PSH, 0,
    LOAD,
    PEEK,          // should be 42

    // === free B ===
    POP,           // pop loaded value (tidy)
    FREE,          // free base1

    // === alloc C, should reuse freed space ===
    PSH, 200,
    ALLOC,         // should get base1 spot again
    PEEK,          // should be same as old base1

    // === free A ===
    POP,           // pop baseC (tidy)
    POP,           // pop old base1 copy (tidy)
    FREE,          // free base0

    // === alloc D (big) should merge A+B and reuse whole ===
    PSH, 300,
    ALLOC,
    PEEK,          // should be base0 if merge works

    HLT
};

const int test_alloc_store_load_free_merge[] = {
    // alloc A (100)
    PSH, 100,
    ALLOC,
    DUP,

    // alloc B (200)
    PSH, 200,
    ALLOC,
    DUP,

    // store at B
    DUP,
    PSH, 0,
    PSH, 42,
    STORE,

    // load back
    DUP,
    PSH, 0,
    LOAD,
    PEEK,

    // clean stack
    POP,

    // free B
    FREE,

    // free A
    FREE,

    // now alloc 300 — should merge A+B
    PSH, 300,
    ALLOC,
    PEEK,

    HLT
};






	run_test("TEST JMP", test_jmp, sizeof(test_jmp)/sizeof(int));
	run_test("TEST JNZ", test_jnz, sizeof(test_jnz)/sizeof(int));
	run_test("TEST JZ",  test_jz,  sizeof(test_jz)/sizeof(int));
	run_test("TEST JE",  test_je,  sizeof(test_je)/sizeof(int));
	run_test("TEST SET/GET", test_set_get, sizeof(test_set_get)/sizeof(int));
	run_test("TEST CMP", test_cmp, sizeof(test_cmp)/sizeof(int));
	run_test("TEST CALL/RET", test_call_ret, sizeof(test_call_ret)/sizeof(int));
	run_test("TEST LOG_AND", test_log_and, sizeof(test_log_and)/sizeof(int));
run_test("TEST LOG_AND FALSE", test_log_and_false, sizeof(test_log_and_false)/sizeof(int));
run_test("TEST LOG_OR", test_log_or, sizeof(test_log_or)/sizeof(int));
run_test("TEST LOG_XOR", test_log_xor, sizeof(test_log_xor)/sizeof(int));
run_test("TEST LOG_XOR FALSE", test_log_xor_false, sizeof(test_log_xor_false)/sizeof(int));
run_test("TEST LOG_NOT", test_log_not, sizeof(test_log_not)/sizeof(int));
run_test("TEST AND", test_and, sizeof(test_and)/sizeof(int));
run_test("TEST OR", test_or, sizeof(test_or)/sizeof(int));
run_test("TEST XOR", test_xor, sizeof(test_xor)/sizeof(int));
run_test("TEST NOT", test_not, sizeof(test_not)/sizeof(int));
run_test("TEST ALLOC BASIC", test_alloc_basic, sizeof(test_alloc_basic)/sizeof(int));
run_test("TEST ALLOC TWICE", test_alloc_twice, sizeof(test_alloc_twice)/sizeof(int));
run_test("TEST ALLOC FAIL", test_alloc_fail, sizeof(test_alloc_fail)/sizeof(int));
run_test("TEST_ALLOC_STORE_LOAD_FREE", test_alloc_store_load_free, sizeof(test_alloc_store_load_free)/sizeof(int));
run_test("TEST_ALLOC_STORE_LOAD_FREE_MERGE", test_alloc_store_load_free_merge, sizeof(test_alloc_store_load_free_merge)/sizeof(int));





	return 0;
}
