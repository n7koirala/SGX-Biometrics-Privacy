# Makefile for running the Receiver-Sender code with Gramine and SGX

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
TARGET = ReceiverSender

# Source files
SRCS = src/main.cpp src/aes_crypt.cpp  # Ensure correct path
DEPS = include/aes_crypt.h             # Include header dependency

# Default target
all: $(TARGET) $(TARGET).manifest

ifeq ($(SGX),1)
all: $(TARGET).manifest.sgx $(TARGET).sig
endif

# Rule to build the target executable
$(TARGET): $(SRCS) $(DEPS)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lssl -lcrypto

# Rule to build the manifest file
$(TARGET).manifest: $(TARGET).manifest.template
	gramine-manifest \
		-Dlog_level=$(GRAMINE_LOG_LEVEL) \
		-Dnum_senders="$(NUM_SENDERS)" \
		$< $@
	gramine-manifest-check $@

# Rule to sign the manifest for SGX
$(TARGET).sig $(TARGET).manifest.sgx: sgx_sign
	@:

.INTERMEDIATE: sgx_sign
sgx_sign: $(TARGET).manifest $(TARGET)
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
	$(GRAMINE) ./$(TARGET)

# Run the application with Gramine and specify number of senders
.PHONY: run-with-senders
run-with-senders: all
	$(GRAMINE) ./$(TARGET) "$(NUM_SENDERS)"

# Clean up build files
.PHONY: clean
clean:
	rm -f *.token *.sig *.manifest.sgx *.manifest
	rm -f $(TARGET)

# Phony targets
.PHONY: all
