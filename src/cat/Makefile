CC = gcc
C_FLAGS = -Wall -Werror -Wextra -std=c11
BUILD_DIR = .

all: s21_cat

s21_cat: clean_all s21_cat_bin clean_o
	
s21_cat_bin: s21_string.o s21_args_parser.o s21_shared_func.o ./s21_cat.c
	${CC} ${C_FLAGS} $(BUILD_DIR)/s21_string.o $(BUILD_DIR)/s21_args_parser.o $(BUILD_DIR)/s21_shared_func.o ./s21_cat.c -o $(BUILD_DIR)/s21_cat

s21_shared_func.o: s21_string.o s21_args_parser.o ../common/s21_shared_func.c
	${CC} ${C_FLAGS} -c -o $(BUILD_DIR)/s21_shared_func.o ../common/s21_shared_func.c

s21_args_parser.o: s21_string.o ../common/s21_args_parser.c
	${CC} ${C_FLAGS} -c -o $(BUILD_DIR)/s21_args_parser.o ../common/s21_args_parser.c

s21_string.o: ../common/s21_string.c
	${CC} ${C_FLAGS} -c -o $(BUILD_DIR)/s21_string.o ../common/s21_string.c

clean_all:
	[ -d $(BUILD_DIR) ] || mkdir -p $(BUILD_DIR)
	rm -rf $(BUILD_DIR)/*.o
	rm -rf $(BUILD_DIR)/s21_cat

clean_o:
	[ -d $(BUILD_DIR) ] || mkdir -p $(BUILD_DIR)
	rm -rf $(BUILD_DIR)/*.o