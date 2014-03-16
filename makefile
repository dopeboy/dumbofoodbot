dumbofoodbot: dumbofoodbot.c dumbofoodbot.c
	gcc -ggdb `pkg-config --cflags opencv` -o `basename dumbofoodbot.c .c` dumbofoodbot.c `pkg-config --libs opencv`

analyzer: analyzer.c analyzer.c
	gcc -ggdb `pkg-config --cflags opencv` -o `basename analyzer.c .c` analyzer.c `pkg-config --libs opencv`
