all: read read_trailer

read: read.o common.o
	g++ -O2 -Wall -o read read.o common.o

read.o: read.cpp
	g++ -O2 -Wall -c -o read.o read.cpp

read_trailer: read_trailer.o common.o
	g++ -O2 -Wall -o read_trailer read_trailer.o common.o

read_trailer.o: read_trailer.cpp
	g++ -O2 -Wall -c -o read_trailer.o read_trailer.cpp

common.o: common.cpp
	g++ -O2 -Wall -c -o common.o common.cpp

clean:
	rm -f read read_trailer *.o
