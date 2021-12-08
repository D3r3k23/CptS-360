SRC_DIR   := src
BUILD_DIR := build
EXE 	  := $(BUILD_DIR)/a.out
GCC_FLAGS := -g -m32 -MMD -Wall #-DEN_LOG

SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(subst $(SRC_DIR), $(BUILD_DIR), $(SRCS:.c=.o))

all : $(OBJS) $(EXE)

$(EXE) : $(OBJS) | $(BUILD_DIR)
	@echo "------ Make $(EXE) ------"
	rm -f $(EXE)
	gcc $(GCC_FLAGS) -o $(EXE) $(OBJS)

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "------ Make $(@) ------"
	rm -f $@
	gcc $(GCC_FLAGS) -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

-include $(BUILD_DIR)/*.d

clean:
	rm -rf $(BUILD_DIR)/*
