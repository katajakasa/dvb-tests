
all: captool

captool:
	gcc -Wall -o captool captool.c dvblib.c

clean:
	rm -f  *.o *~ captool

