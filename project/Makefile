SRC_DIR   := src
BUILD_DIR := build
EXE 	  := $(BUILD_DIR)/a.out
GCC_FLAGS := -g -m32 -Wall -Wextra -DEN_LOG

SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(subst $(SRC_DIR), $(BUILD_DIR), $(SRCS:.c=.o))

all : $(OBJS) $(EXE)

$(EXE) : $(OBJS)
	@echo "make" $(EXE)
	gcc $(GCC_FLAGS) -MMD -o $(EXE) $(OBJS)

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "make $(@)"
	gcc $(GCC_FLAGS) -MMD -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

-include $(BUILD_DIR)/*.d

clean:
	rm -rf $(BUILD_DIR)/*
