#ifndef PROJECT2_SS_H
#define PROJECT2_SS_H

#include <iostream>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <fstream>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
using namespace std;
class ss {
public:
    // constructor
    ss();
    int startListen();

    // port number
    const char *port;
    // ip address
    const char *ip;
    string request;
    size_t MAXDATASIZE = 1024;
    vector <string> info;
};


#endif //PROJECT2_SS_H
