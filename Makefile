# Makefile

CXX = gcc
CXXFLAGS = 
SRC_DIR = src
BIN_DIR = bin

CHAT_SRC = $(SRC_DIR)/translate.c

CHAT_BIN = $(BIN_DIR)/translate

TARGETS = $(CHAT_BIN) 

all: $(TARGETS)

# Ensure bin directory exists
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Build translate
$(CHAT_BIN): $(CHAT_SRC) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean
