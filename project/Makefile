BUILD_DIR = build
SRC_DIR   = src
GCC_FLAGS = -g -m32 -Wall -Wextra $(LOG) # LOG=EN_LOG

all: util cd_ls_pwd main
	mkdir -p $(BUILD_DIR)

log: $(SRC_DIR)/log.h

type: $(SRC_DIR)/type.h

util: $(SRC_DIR)/util.c $(SRC_DIR)/util.h log type
	gcc $(GCC_FLAGS) -c -o $(BUILD_DIR)/util.o $(SRC_DIR)/util.c

cd_ls_pwd: $(SRC_DIR)/cd_ls_pwd.c $(SRC_DIR)/cd_ls_pwd.h log type
	gcc $(GCC_FLAGS) -c -o $(BUILD_DIR)/cd_ls_pwd.o $(SRC_DIR)/cd_ls_pwd.c

main: $(SRC_DIR)/main.c util cd_ls_pwd
	gcc $(GCC_FLAGS) -o $(BUILD_DIR)/main.bin $(SRC_DIR)/main.c $(BUILD_DIR)/*.o

clean:
	rm -rf $(BUILD_DIR)/*
