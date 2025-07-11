

CC_FLAGS = -Wall -Wextra -g -std=c11
CC = clang

SRC_DIR = src
BUILD_DIR = build


SRC_FILES = $(SRC_DIR)/vm.c $(SRC_DIR)/error.c 
TEST_FILES = $(SRC_DIR)/test.c


all: VirtMach

VirtMach: $(SRC_FILES)
	${CC} ${CC_FLAGS} $^ -o $@

test: $(TEST_FILES) $(SRC_FILES)
	mkdir -p $(BUILD_DIR)
	${CC} ${CC_FLAGS} $^ -o $(BUILD_DIR)/test
	./$(BUILD_DIR)/test

clean:
	rm -f VirtMach $(BUILD_DIR)/test
	rm -rf $(BUILD_DIR)
	rm -rf *.dSYM

