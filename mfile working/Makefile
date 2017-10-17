#Makefile

INCLUDES=-I. -O3
CXX=g++ -std=c++11 -pthread

all: awget

awget: awget.o
	$(CXX) -Wall $(INCLUDES) awget.o -o awget

awget.o: $(SOURCE)
	$(CXX) -Wall $(INCLUDES) -c awget.cpp

clean:
	rm -f awget.o awget