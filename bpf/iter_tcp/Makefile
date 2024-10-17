ARCH := $(shell uname -m | sed 's/x86_64/x86/' | sed 's/aarch64/arm64/' \
		| sed 's/ppc64le/powerpc/' | sed 's/mips.*/mips/' \
		| sed 's/riscv64/riscv/' | sed 's/loongarch.*/loongarch/')

BPFTOOL := bpftool
CC := clang

# define the sources
ITER_TCP := iter_tcp
ITER_TCP_OBJ := $(ITER_TCP:=.o)
ITER_TCP_C := $(ITER_TCP:=.c)

LIBBPF := ../libbpf
LIBBPF_HEADER_PATH := $(LIBBPF)/src/root/usr/include
CFLAGS := -I $(LIBBPF_HEADER_PATH)
CFLAGS += -I ../$(ARCH)

$(ITER_TCP_OBJ): $(ITER_TCP_C)
	@echo "Generating bpf object file..."
	$(CC) -O2 -g -Wall -target bpf -c $(ITER_TCP_C) $(CFLAGS) -o $(ITER_TCP_OBJ)

.PHONY: generate
generate:
	mkdir -p $(ARCH)
	@echo "Generating..."
	$(BPFTOOL) btf dump file /sys/kernel/btf/vmlinux format c > $(ARCH)/vmlinux.h


.PHONY: clean
clean:
	@echo "Cleaning..."
	rm $(ITER_TCP_OBJ)


.PHONY: cleanall
cleanall:
	make clean
	rm -rf $(ARCH)


