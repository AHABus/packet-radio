SHELL			:= bash
STD				:= c99

TARGET			:= encoder

SOURCE_DIR		:= ./src
BUILD_DIR		:= ./build
TEST_DIR		:= ./test
OBJECTS_DIR		:= $(BUILD_DIR)/intermediate
PRODUCT_DIR		:= $(BUILD_DIR)/product

SOURCES			:= $(wildcard $(SOURCE_DIR)/*.c)
OBJECTS			:= $(patsubst $(SOURCE_DIR)/%.c, $(OBJECTS_DIR)/%.o, $(SOURCES))

CFLAGS			:= -Wall -std=$(STD)
LDFLAGS			:= 

.PHONY: clean

all: $(TARGET)

$(TARGET): CFLAGS += -I$(SOURCE_DIR)
$(TARGET): $(PRODUCT_DIR) $(TEST_DIR)/$(TARGET).c $(OBJECTS)
	@$(CC) $(TEST_DIR)/$(TARGET).c $(OBJECTS) $(CFLAGS) $(LDFLAGS) -o $(PRODUCT_DIR)/$(TARGET)
	@$(PRODUCT_DIR)/$(TARGET)

$(OBJECTS_DIR)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "compiling $(notdir $<)"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJECTS_DIR):
	@mkdir -p $(OBJECTS_DIR)
	
$(PRODUCT_DIR):
	@mkdir -p $(PRODUCT_DIR)

clean:
	@echo "cleaning project"
	@rm -rf $(BUILD_DIR)
	
