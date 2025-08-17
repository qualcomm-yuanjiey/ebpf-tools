SUBDIRS := zstd zlib libelf libbpf-tools

.PHONY: all $(SUBDIRS) clean

all: libbpf-tools

CC ?= gcc
ARCH ?= x86_64

CURR_DIR := $(shell pwd)
ZSTD_PATH=$(CURR_DIR)/zstd
ZLIB_PATH=$(CURR_DIR)/zlib
LIBELF_PATH=$(CURR_DIR)/libelf

# ARCH=arm64
# CROSS_COMPILE=aarch64-linux-gnu-
# CC=aarch64-linux-gnu-gcc

zstd:
	$(MAKE) -C zstd ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) CC=$(CC)

zlib:
	$(MAKE) -C zlib ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) CC=$(CC)

libelf: zlib zstd
	$(MAKE) -C libelf ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) CC=$(CC) LDFLAGS="-L$(ZSTD_PATH)/lib -L$(ZLIB_PATH)"

libbpf-tools: zstd zlib libelf
	$(MAKE) -C libbpf-tools ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) CC=$(CC)

# $(SUBDIRS):
# 	$(MAKE) -C $@

clean:
	$(MAKE) -C zstd clean
	$(MAKE) -C zlib
	$(MAKE) -C libelf
	$(MAKE) -C libbpf-tools

# clean:
# 	for dir in $(SUBDIRS); do \
# 		$(MAKE) -C $$dir clean; \
# 	done
