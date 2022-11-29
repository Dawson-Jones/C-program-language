NAME = mytbf

RAW = ${NAME:=.c}
OBJ = ${RAW:.c=.o}
SO = ${RAW:.c=.so}

CFLAG +=  -fPIC -shared

$(SO): $(RAW)
	$(CC) $^ $(CFLAG) -o $@

clean:
	rm -rf *.o mytbf