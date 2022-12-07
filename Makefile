all: read read_trailer

read: read.cpp
	g++ -O2 -Wall -o read read.cpp

read_trailer: read_trailer.cpp
	g++ -O2 -Wall -o read_trailer read_trailer.cpp
