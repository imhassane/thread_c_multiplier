prog: main.o func.o helper.o
	gcc -D_GNU_SOURCE -pthread -o prog main.o func.o helper.o

main.o:
	gcc -pthread -c main.c

func.o: func.h func.c
	gcc -D_GNU_SOURCE -c func.c

helper.o: helper.h helper.c
	gcc -c helper.c

clean:
	rm -rf *.o