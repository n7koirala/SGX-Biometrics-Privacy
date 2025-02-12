#------------------------------------------------------------------------------
# Makefile for running ReceiverSender with Gramine and SGX
#------------------------------------------------------------------------------

# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude

# Debug vs Release
ifeq ($(DEBUG),1)
  GRAMINE_LOG_LEVEL = debug
  CXXFLAGS += -g
else
  GRAMINE_LOG_LEVEL = error
  CXXFLAGS += -O3
endif

# Target application
TARGET = ReceiverSender

# Source files and includes
SRCS = src/main.cpp src/aes_crypt.cpp
DEPS = include/aes_crypt.h include/file_paths.h  # Adjust as needed

# Default build targets
all: $(TARGET) $(TARGET).manifest

# If SGX=1, also generate the .sig and .manifest.sgx
ifeq ($(SGX),1)
all: $(TARGET).manifest.sgx $(TARGET).sig
endif

#------------------------------------------------------------------------------
# Build the application
#------------------------------------------------------------------------------

$(TARGET): $(SRCS) $(DEPS)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lssl -lcrypto

#------------------------------------------------------------------------------
# Manifest generation
#------------------------------------------------------------------------------

# Create .manifest from .manifest.template, substituting log level only
$(TARGET).manifest: $(TARGET).manifest.template
	gramine-manifest \
		-Dlog_level=$(GRAMINE_LOG_LEVEL) \
		$< $@
	gramine-manifest-check $@

#------------------------------------------------------------------------------
# SGX Signing
#------------------------------------------------------------------------------

$(TARGET).sig $(TARGET).manifest.sgx: sgx_sign
	@:

.INTERMEDIATE: sgx_sign
sgx_sign: $(TARGET).manifest $(TARGET)
	gramine-sgx-sign \
		--manifest $< \
		--output $<.sgx
	gramine-manifest-check $<.sgx

#------------------------------------------------------------------------------
# Run targets
#------------------------------------------------------------------------------

# Determine whether to run in SGX or non-SGX mode
ifeq ($(SGX),)
GRAMINE = gramine-direct
else
GRAMINE = gramine-sgx
endif

.PHONY: run
run: all
	$(GRAMINE) ./$(TARGET)

#------------------------------------------------------------------------------
# Clean
#------------------------------------------------------------------------------

.PHONY: clean
clean:
	rm -f *.token *.sig *.manifest.sgx *.manifest
	rm -f $(TARGET)

.PHONY: all
