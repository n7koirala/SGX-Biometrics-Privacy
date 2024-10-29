# Makefile for running the Receiver-Sender code with Gramine and SGX

# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++17

ifeq ($(DEBUG),1)
GRAMINE_LOG_LEVEL = debug
CXXFLAGS += -g
else
GRAMINE_LOG_LEVEL = error
CXXFLAGS += -O3
endif

# Output executable
TARGET = ReceiverSender

# Source files
SRCS = main.cpp

# Build directories
BUILD_DIR = build
BIN_DIR = $(BUILD_DIR)
LIB_DIR = lib

# Default target
all: $(BIN_DIR)/$(TARGET) $(BIN_DIR)/$(TARGET).manifest

ifeq ($(SGX),1)
all: $(BIN_DIR)/$(TARGET).manifest.sgx $(BIN_DIR)/$(TARGET).sig
endif

# Rule to build the target executable
$(BIN_DIR)/$(TARGET): $(SRCS)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Rule to build the manifest file
$(BIN_DIR)/$(TARGET).manifest: $(TARGET).manifest.template
	gramine-manifest \
		-Dlog_level=$(GRAMINE_LOG_LEVEL) \
		-Dnum_senders="$(NUM_SENDERS)" \
		$< $@
	gramine-manifest-check $@

# Rule to sign the manifest for SGX
$(BIN_DIR)/$(TARGET).sig $(BIN_DIR)/$(TARGET).manifest.sgx: sgx_sign
	@:

.INTERMEDIATE: sgx_sign
sgx_sign: $(BIN_DIR)/$(TARGET).manifest $(BIN_DIR)/$(TARGET)
	gramine-sgx-sign \
		--manifest $< \
		--output $<.sgx
	gramine-manifest-check $<.sgx

# Determine whether to run in SGX or non-SGX mode
ifeq ($(SGX),)
GRAMINE = gramine-direct
else
GRAMINE = gramine-sgx
endif

# Run the application with Gramine
.PHONY: run
run: all
	$(GRAMINE) $(BIN_DIR)/$(TARGET)

# Run the application with Gramine and specify number of senders
.PHONY: run-with-senders
run-with-senders: all
	$(GRAMINE) $(BIN_DIR)/$(TARGET) "$(NUM_SENDERS)"

# Clean up build files
.PHONY: clean
clean:
	rm -f *.token *.sig *.manifest.sgx *.manifest
	rm -rf $(BUILD_DIR) $(LIB_DIR)

# Phony targets
.PHONY: all
