#include <iostream>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <fstream>
#include <sys/socket.h>
#include <cstring>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "awget.h"

using namespace std;

// set for debugging output on/off
bool debug = true;

// constructor
awget::awget(){
    ss = vector<stepStone>(0);
}

/// Print the correct usage in case of user syntax error.
int usage()
{
    cout << "Usage: to start awget, run \"./awget <URL> [-c <chain file name>]\"\n" << endl;
    return -1;
}

void *awget::get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int awget::runTheGet(int index, vector <awget::stepStone> *ss) {
    int next = rand() % index;
    stepStone stone = ss->at(next);
    cout << "next SS is " << stone.addr << ", " << stone.port << endl;
    cout << "waiting for file..." << endl;
    const char *PORTNUM = (stone.port).c_str();
    const char *IP = (stone.addr).c_str();
    // do more stuff
    int sok = 0;     // stone socket descriptor
    struct sockaddr_storage remoteaddr{}; // client address
    char buf[MAXDATASIZE];
    string input;
    int yes=1;        // for setsockopt() SO_REUSEADDR, below

    int status;
    struct addrinfo hints, *ai;// will point to the results

    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    if ((status = getaddrinfo(IP, PORTNUM, &hints, &ai)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    sok = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (sok < 0) {
        // lose the pesky "address already in use" error message
        setsockopt(sok, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    }

    if ((connect(sok, ai->ai_addr, ai->ai_addrlen)) < 0){
        cerr << "connection failed" << endl;
        return -1;
    }

    // where request ends, and stones start
    int dex = 0;
    // initially, buffer just contains the requested url
    for (int i = 0;i < request.length();i++){
        buf[i] = request.at(i);
        dex++;
    }
    buf[dex++] = ',';
    for (int i = 0;i < sendStones.length();i++){
        if (i == strlen(buf)){
            // send what you've got, empty buf, then keep going
            if (debug){
                cout << "buffer full, sending: " << buf << endl;
            }
            if (send(sok, buf, MAXDATASIZE, 0) == -1){
                perror("send failed");
                exit(6);
            }
        }
        buf[i+dex] = sendStones[i];
    }
    if (debug){
        cout << "sending: " << buf << endl;
    }
    if (send(sok, buf, strlen(buf), 0) == -1){
        perror("send failed");
        exit(6);
    }
    freeaddrinfo(ai); // all done with this

    return 0;
}

int main(int argc, char **argv) {
    // seed random for later
    srand(time(NULL));
    // local vars
    int cFlag = 0;
    int hFlag = 0;
    char *chainfile = nullptr;
    char *arg = nullptr;
    int index;
    int c = 0;

    // check that there is at least one argument before proceeding
    if (argc <= 1) return usage();
    // print out the name of the requested page
    cout << "Request: " << argv[1] << endl;
    // code based on provided example at: https://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html#Example-of-Getopt
    // loops while there are options ("-h", "-c") provided at cmd line, and assigns them to variables
    while ((c = getopt (argc, argv, "c:")) != -1) {
        switch (c) {
            case 'h':
                hFlag = 1;
                break;
            case 'c':
                cFlag = 1;
                arg = optarg;
                break;
            case '?':
                if (optopt == 'c')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n",
                            optopt);
                return 1;
            default:
                abort();
        }

    }
    if (hFlag){
        return usage();
    }

    if (cFlag){
        chainfile = arg;
    }
    else chainfile = const_cast<char *>("chaingang.txt");

    ifstream istr0(chainfile);
    if (istr0.fail()){
        cout << "File not read, exiting." << endl;
        return usage();
    } else if (debug){
        cout << "Reading file... " << chainfile << endl;
    }
    string num;
    istr0 >> num;
    istringstream inNum(num);
    if (!(inNum >> index)){
        cerr << "File must begin with number of stepping stones" << endl;
    }
    // awget object
    awget a = awget();
    a.request = argv[1];
    a.ss.resize(index);
    // temp variables to hold ss info
    string addr;
    string port;
    int portNum;
    string delim = " ";
    // loop for designated number of stepping stones, add to list
    for (int i = 0; i < index; i++){
        istr0 >> addr;
        istr0 >> port;
        // add values to this string that will be sent to the stone later
        a.sendStones.append(addr);
        a.sendStones.append(",");
        a.sendStones.append(port);
        a.sendStones.append(",");
        // check that the port number is valid
        istringstream strm(port);
        if (!(strm >> portNum)){
            cerr << "Invalid number " << port << endl;
            return -1;
        }
        // add empty object to list, then fill it in from file
        a.ss.emplace_back(awget::stepStone());
        a.ss.at(i).addr = addr;
        a.ss.at(i).port = port;
        if (istr0.fail() && !istr0.eof()){
            cerr << "file not formatted correctly, exiting." << endl;
            return -1;
        } else if (istr0.eof()){
            a.sendStones.pop_back();
        }
    }
    if (debug){
        cout << "Index: " << index << endl;
    }
    cout << "Chainlist is" << endl;
    for (int i = 0; i < index;i++){
        cout << a.ss.at(i).addr << ", " << a.ss.at(i).port << endl;
    }
    a.runTheGet(index,&a.ss);
    return 0;
}