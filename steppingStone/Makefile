#Makefile

INCLUDES=-I. -O3
CXX=g++ -std=c++11 -pthread

all: ss

ss: ss.o
	$(CXX) -Wall $(INCLUDES) ss.o -o ss

ss.o: ss.cpp
	$(CXX) -Wall $(INCLUDES) -c ss.cpp

clean:
	rm -f ss.o ss
