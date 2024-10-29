# Makefile for FingerprintMatching project with Gramine

CMAKE_FLAGS = -DCMAKE_BUILD_TYPE=Release
CFLAGS = -Wall -Wextra

ifeq ($(DEBUG),1)
GRAMINE_LOG_LEVEL = debug
CMAKE_FLAGS += -DCMAKE_BUILD_TYPE=Debug
else
GRAMINE_LOG_LEVEL = error
CFLAGS += -O3
endif

# Define default args for two fingerprint files (can be overridden)
PROBE = ""

GALLERY = ""

.PHONY: all
all: build/FingerprintMatching build/FingerprintMatching.manifest
ifeq ($(SGX),1)
all: build/FingerprintMatching.manifest.sgx build/FingerprintMatching.sig
endif

build/FingerprintMatching: CMakeLists.txt
	mkdir -p build
	cd build && cmake .. $(CMAKE_FLAGS) && make

build/FingerprintMatching.manifest: FingerprintMatching.manifest.template
	gramine-manifest \
		-Dlog_level=$(GRAMINE_LOG_LEVEL) \
		-Dprobe="$(PROBE)" \
		-Dgallery="$(GALLERY)" \
		$< $@
	gramine-manifest-check $@

build/FingerprintMatching.sig build/FingerprintMatching.manifest.sgx: sgx_sign
	@:

.INTERMEDIATE: sgx_sign
sgx_sign: build/FingerprintMatching.manifest build/FingerprintMatching
	gramine-sgx-sign \
		--manifest $< \
		--output $<.sgx
	gramine-manifest-check $<.sgx

ifeq ($(SGX),)
GRAMINE = gramine-direct
else
GRAMINE = gramine-sgx
endif

.PHONY: check
check: all
	$(GRAMINE) build/FingerprintMatching $(PROBE) $(GALLERY) > OUTPUT
	@echo "[ Success ]"

.PHONY: clean
clean:
	$(RM) *.token *.sig *.manifest.sgx *.manifest
	rm -rf build OUTPUT

.PHONY: distclean
distclean: clean
