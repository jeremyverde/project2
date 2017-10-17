//
// Created by jeremy on 10/13/17.
//

#ifndef PROJECT2_AWGET_H_INCLUDE
#define PROJECT2_AWGET_H_INCLUDE
using namespace std;


class awget {
public:
    awget();
    void *get_in_addr(struct sockaddr *sa);
    /// structure for storing info from chain file
    struct stepStone{string addr = "";string port = "";};
    int runTheGet(int index, vector <stepStone>*ss);
    // globals
    // stepping stone list
    vector <stepStone>ss;
    string request;
    size_t MAXDATASIZE = 1024;
    string sendStones;
};


#endif //PROJECT2_AWGET_H_INCLUDE
