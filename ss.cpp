#include <cstring>
#include "ss.h"

using namespace std;

// debuging boolean
bool debug;

ss::ss() {
    port = 0;
}

/// Print the correct usage in case of user syntax error.
int usage()
{
    cout << "Usage: to start ss with specific port, run \"./ss <port number>\"" << endl;
    cout << "To start ss with default port number, run \"./ss\"" << endl;
    return -1;
}

int ss::startListen() {
    // code based largely off of beej's guide example program, see README
    int sockfd = 0;
    ssize_t nbytes;
    unsigned char buf[MAXDATASIZE];
    struct addrinfo hints{}, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    bool sendit = true;
    string input;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    socklen_t addr_size;
    struct sockaddr_storage their_addr;
    int newfd;


    if ((rv = getaddrinfo(ip, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype,
                             servinfo->ai_protocol)) == -1) {
        perror("socket");
    }
    if (bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) < 0) {
        close(sockfd);
    }

    if (servinfo == nullptr) {
        fprintf(stderr, "failed to connect\n");
        return 2;
    }
    if (debug){
        cout << "listening..." << endl;
    }
    if (listen(sockfd,10) < 0){
        cerr << "listening failed" << endl;
    }

    addr_size = sizeof their_addr;
    newfd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
    if (debug){
        cout << "connection accepted" << endl;
    }
    recv(newfd,buf,sizeof buf,0);
    printf("read: %s",buf);
    freeaddrinfo(servinfo); // all done with this structure

    for (;;) {
        uint16_t vCheck = 0;
        if (sendit) {
            sendit = false;
        } else {
            if ((nbytes = recv(sockfd, &vCheck, 2, 0)) <= 0) {
                // got error or connection closed by server
                if (nbytes == 0) {
                    // connection closed
                    printf("socket disconnected\n");
                    exit(5);
                } else {
                    perror("recv");
                    exit(6);
                }
            } else {
                // print the message
                printf("Request: '%s'\n", buf);
                memset(buf, 0, sizeof(buf));
                sendit = true;
            }
        }
    }
}

// get sockaddr, IPv4 or IPv6:
void *ss::get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char **argv){
    ss stone = ss();
    int portArg = 0;
    char hostname[128];

    gethostname(hostname,sizeof hostname);
    // hostname to ip code based off example at: http://www.cplusplus.com/forum/articles/9742/
    hostent * record = gethostbyname(hostname);
    auto * address = (in_addr * )record->h_addr;
    stone.ip = inet_ntoa(* address);

    if (argc == 1){
        stone.port = "9001";
    } else{
        // check that its a valid number
        istringstream str(argv[1]);
        if (!(str >> portArg)){
            cerr << "Invalid port number" << endl;
            return usage();
        }
        stone.port = argv[1];
    }
    // print hostname and port
    cout << "SteppingStone: " << stone.ip << ", " << stone.port << endl;
    stone.startListen();
}