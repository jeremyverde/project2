#Makefile

INCLUDES=-I. -O3
CXX=g++ -std=c++11 -pthread

all: awget ss

awget: awget.o
	$(CXX) $(INCLUDES) awget.o -o awget

ss: ss.o
	$(CXX) $(INCLUDES) ss.o -o ss

awget.o: awget.cpp
	$(CXX) $(INCLUDES) -c awget.cpp

ss.o: ss.cpp
	$(CXX) $(INCLUDES) -c ss.cpp

clean:
	rm -f awget.o awget ss.o ss index.*
