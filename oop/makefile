TARGET = main
RAW = ${TARGET:=.c}
OBJ = ${RAW:.c=.o}

$(TARGET): $(RAW) shape.c
	clang $^ -o $@

.PHONY: clean
clean:
	rm main