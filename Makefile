
all: captool

captool:
	gcc -Wall -Isrc/ -o captool src/captool.c src/dvblib/dvblib.c

clean:
	rm -f  *.o *~ captool

