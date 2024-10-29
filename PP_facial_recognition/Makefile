# Makefile for ImageMatching project with Gramine

# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude

ifeq ($(DEBUG),1)
GRAMINE_LOG_LEVEL = debug
CXXFLAGS += -g
else
GRAMINE_LOG_LEVEL = error
CXXFLAGS += -O3
endif

# Output executable
TARGET = ImageMatching

# Source files
SRCS = src/main.cpp src/vector_utils.cpp

# Default dataset filepath (can be overridden)
DATASET_FILEPATH = ""

# Build directories
BUILD_DIR = build
BIN_DIR = $(BUILD_DIR)

# Default target
all: $(BIN_DIR)/$(TARGET) $(BIN_DIR)/$(TARGET).manifest

ifeq ($(SGX),1)
all: $(BIN_DIR)/$(TARGET).manifest.sgx $(BIN_DIR)/$(TARGET).sig
endif

# Rule to build the target executable
$(BIN_DIR)/$(TARGET): $(SRCS)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS)

# Rule to build the manifest file
$(BIN_DIR)/$(TARGET).manifest: ImageMatching.manifest.template
	gramine-manifest \
		-Dlog_level=$(GRAMINE_LOG_LEVEL) \
		-Ddataset_filepath=$(DATASET_FILEPATH) \
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

# Run the application with Gramine and the dataset filepath
.PHONY: run-with-dataset
run-with-dataset: all
	$(GRAMINE) $(BIN_DIR)/$(TARGET) "$(DATASET_FILEPATH)"

# Clean up build files
.PHONY: clean
clean:
	rm -f *.token *.sig *.manifest.sgx *.manifest
	rm -rf $(BUILD_DIR)

# Phony targets
.PHONY: all
