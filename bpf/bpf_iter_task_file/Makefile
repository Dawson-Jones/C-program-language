ARCH := $(shell uname -m | sed 's/x86_64/x86/' | sed 's/aarch64/arm64/' \
		| sed 's/ppc64le/powerpc/' | sed 's/mips.*/mips/' \
		| sed 's/riscv64/riscv/' | sed 's/loongarch.*/loongarch/')

BPFTOOL := bpftool
CC := clang

# define the sources
ITER_TASK_FILE := bpf_iter_task_file
ITER_TASK_FILE_OBJ := $(ITER_TASK_FILE:=.o)
ITER_TASK_FILE_C := $(ITER_TASK_FILE:=.c)
ITER_TASK_FILE_SKEL := $(ITER_TASK_FILE:=.skel.h)
ITER_TASK_FILE_LOADER := bpf_iter_task_file_loader
ITER_TASK_FILE_LOADER_C := $(ITER_TASK_FILE_LOADER:=.c)
ITER_TASK_FILE_LOADER2 := bpf_iter_task_file_loader2
ITER_TASK_FILE_LOADER2_C := $(ITER_TASK_FILE_LOADER2:=.c)

LIBBPF := ../libbpf
LIBBPF_HEADER_PATH := $(LIBBPF)/src/root/usr/include
LIBBPF_OBJ := $(LIBBPF)/src/build/libbpf.a

.PHONY: all
all: $(ITER_TASK_FILE_LOADER) $(ITER_TASK_FILE_LOADER2)

CFLAGS := -I $(LIBBPF_HEADER_PATH)

DEPS := $(ITER_TASK_FILE_LOADER_C) common.c $(LIBBPF_OBJ)
$(ITER_TASK_FILE_LOADER): $(DEPS)
	$(CC) $(CFLAGS) $^ -lelf -lz -o $@

$(ITER_TASK_FILE_LOADER2): $(DEPS)
	$(CC) $(CFLAGS) $^ -lelf -lz -o $@


# $(ITER_TASK_FILE_SKEL): $(ITER_TASK_FILE_OBJ)
# 	@echo "Generating skeleton..."
# 	$(BPFTOOL) gen skeleton $(ITER_TASK_FILE_OBJ) > $(ITER_TASK_FILE_SKEL)


CFLAGS += -I ../$(ARCH)
# clang -I ./arm64 -I ./libbpf/src/root/usr/include -target bpf -c bpf_iter_task_file.c -O2 -o bpf_iter_task_file.o
$(ITER_TASK_FILE_OBJ): $(ITER_TASK_FILE_C)
	@echo "Generating bpf object file..."
	$(CC) -O2 -g -Wall -target bpf -c $(ITER_TASK_FILE_C) $(CFLAGS) -o $(ITER_TASK_FILE_OBJ)

.PHONY: skel
skel: $(ITER_TASK_FILE_OBJ)
	@echo "Generating skeleton..."
	$(BPFTOOL) gen skeleton $^ > $(ITER_TASK_FILE_SKEL)

.PHONY: generate
generate:
	mkdir -p $(ARCH)
	@echo "Generating..."
	$(BPFTOOL) btf dump file /sys/kernel/btf/vmlinux format c > $(ARCH)/vmlinux.h


.PHONY: clean
clean:
	@echo "Cleaning..."
	rm $(ITER_TASK_FILE_OBJ)  $(ITER_TASK_FILE_SKEL)
	rm $(ITER_TASK_FILE_LOADER)
	rm $(ITER_TASK_FILE_LOADER2)


.PHONY: cleanall
cleanall:
	make clean
	rm -rf $(ARCH)


