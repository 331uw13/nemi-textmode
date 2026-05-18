TARGET=textmode.so



LIBNEMI_DIR=$(shell (cd ../nemi && pwd))
SRC_DIR=./src
OBJ_DIR=./obj


CC_FLAGS = \
		   -ggdb \
		   -fPIC \
		   -Wall \
		   -Wextra \
		   -I$(LIBNEMI_DIR)/src


LD_FLAGS = \
		   -shared \
		   -Wl,-rpath=$(LIBNEMI_DIR) \
		   -L$(LIBNEMI_DIR) \
		   -lnemi

all: $(TARGET) 

SRC_FILES = $(shell find $(SRC_DIR) -type f -name '*.c')
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))


$(OBJ_FILES): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(shell dirname $@)
	@echo "($(TARGET)): $<"
	@$(CC) $(CC_FLAGS) -c $< -o $@


$(TARGET): $(OBJ_FILES)
	@echo "link"
	$(CC) $(LD_FLAGS) $(OBJ_FILES) -o $@
	@echo -e "\033[1;32mBuild complete!\033[0m"
	@cp -v $(TARGET) $(LIBNEMI_DIR)/modules/


clean:
	rm -rfv $(OBJ_DIR)
	rm -v $(TARGET)

.PHONY: all clean
