objects = a.o b.o

test : $(objects)
	cc -o test $(objects)

a.o : a.c
	cc -c a.c
b.o : b.c
	cc -c b.c

.PHONY : clean
clean :
	rm test $(objects) ./path/file
