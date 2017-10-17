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

class ss {
public:
    // constructor
    ss();
    int startListen();
    void *get_in_addr(struct sockaddr *sa);

    // port number
    const char *port;
    // ip address
    const char *ip;
    const int MAXDATASIZE = 144;
};


#endif //PROJECT2_SS_H
