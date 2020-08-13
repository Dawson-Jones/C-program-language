test : custom_define_error.o 02_copy_file.o
	cc -o test custom_define_error.o 02_copy_file.o

custom_define_error.o : ./utils/custom_define_error.c
	cc -c ./utils/custom_define_error.c
02_copy_file.o : 02_copy_file.c
	cc -c 02_copy_file.c

.PHONY : clean
clean :
	rm test custom_define_error.o 02_copy_file.o
