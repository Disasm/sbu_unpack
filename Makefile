all: read

read: read.cpp
	g++ -O2 -Wall -o read read.cpp

clean:
	rm -f read
