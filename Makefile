SUBDIRS := zstd zlib libelf libbpf-tools

.PHONY: all clean $(SUBDIRS) zlib_clean

all: libbpf-tools

CC ?= gcc
ARCH ?= x86_64

CURR_DIR := $(shell pwd)

ZSTD_PATH=$(CURR_DIR)/zstd
ZLIB_PATH=$(CURR_DIR)/zlib
LIBELF_PATH=$(CURR_DIR)/libelf

LIBZSTD=$(ZSTD_PATH)/lib/libzstd.a

$(LIBZSTD):
	$(MAKE) -C zstd

zlib:
	cd zlib && ([ -f configure.log ] || ./configure CC=$(CC) CHOST=$(ARCH)) && $(MAKE)

zlib_clean:
	$(MAKE) -C zlib clean
	rm zlib/configure.log

libelf: $(LIBZSTD) zlib
	$(MAKE) -C libelf LDFLAGS="-L$(ZSTD_PATH)/lib -L$(ZLIB_PATH)"

libbpf-tools: $(LIBZSTD) zlib libelf
	$(MAKE) -C libbpf-tools

clean:
	$(MAKE) -C zstd clean
	$(MAKE) -C zlib clean
	$(MAKE) -C libelf clean
	$(MAKE) -C libbpf-tools clean
