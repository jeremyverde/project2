//
// Created by jeremy on 10/13/17.
//

#ifndef PROJECT2_AWGET_H_INCLUDE
#define PROJECT2_AWGET_H_INCLUDE

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <fstream>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
using namespace std;


class awget {
public:
    awget(){ss = vector<stepStone>(0); ip = nullptr;port = nullptr;};
    void *get_in_addr(struct sockaddr *sa);
    char *readFile(char * in);
    /// structure for storing info from chain file
    struct stepStone{string addr = "";string port = "";};
    int runTheGet(unsigned int index, vector <stepStone>*ss);
    int startListen();
    // globals
    // stepping stone list
    vector <stepStone>ss;
    // port number
    const char *port;
    // ip address
    const char *ip;
    string request;
    size_t MAXDATASIZE = 1024;
    vector <string> info;
    string sendStones;
};


#endif //PROJECT2_AWGET_H_INCLUDE
