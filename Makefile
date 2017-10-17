#Makefile

INCLUDES=-I. -O3
CXX=g++ -std=c++11 -pthread

all: awget ss

awget: awget.o
	$(CXX) -Wall $(INCLUDES) awget.o -o awget

ss: ss.o
	$(CXX) -Wall $(INCLUDES) ss.o -o ss

awget.o: awget.cpp
	$(CXX) -Wall $(INCLUDES) -c awget.cpp

ss.o: ss.cpp
	$(CXX) -Wall $(INCLUDES) -c ss.cpp

clean:
	rm -f awget.o awget ss.o ss
