all : consolekmid 
CFLAGS=-Wall -O2 

consolekmid : main.o midiout.o track.o midfile.o player.o dattypes.o midimapper.o midistat.o
	gcc -Wall -o consolekmid main.o midiout.o track.o midfile.o player.o dattypes.o midimapper.o midistat.o
	strip consolekmid

main.o : main.cc midiout.h player.h
	gcc $(CFLAGS) -c main.cc

midiout.o : midiout.cc midiout.h midimapper.h
	gcc $(CFLAGS) -c midiout.cc 

midistat.o : midistat.cc midistat.h midiout.h
	gcc $(CFLAGS) -c midistat.cc

track.o : track.cc track.h
	gcc $(CFLAGS) -c track.cc

midfile.o : midfile.cc midfile.h track.h
	gcc $(CFLAGS) -c midfile.cc

player.o : player.cc player.h midiout.h midfile.h track.h
	gcc $(CFLAGS) -c player.cc

dattypes.o : dattypes.cc dattypes.h
	gcc $(CFLAGS) -c dattypes.cc

midimapper.o : midimapper.cc midimapper.h
	gcc $(CFLAGS) -c midimapper.cc

all_copy : midiout.o track.o midfile.o player.o dattypes.o midimapper.o midistat.o
	cp midiout.o ..
	cp track.o ..
	cp midfile.o ..
	cp player.o ..
	cp dattypes.o ..
	cp midimapper.o ..
	cp midistat.o ..

clean :
	rm -f *.o
	rm -f consolekmid
