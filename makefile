hellomake: findmesometrucks.c findmesometrucks.c
	gcc -ggdb `pkg-config --cflags opencv` -o `basename findmesometrucks.c .c` findmesometrucks.c `pkg-config --libs opencv`
