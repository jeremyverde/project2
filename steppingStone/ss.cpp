#include <cstring>
#include "ss.h"

using namespace std;

// debuging boolean
bool debug = true;

ss::ss() {
    port = nullptr;
    info = vector<string>(10);
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
    char buf[MAXDATASIZE];
    struct addrinfo hints{}, *servinfo;
    int rv;
    string input;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    socklen_t addr_size;
    struct sockaddr_storage their_addr{};
    int newfd;
    char req[MAXDATASIZE];

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

    freeaddrinfo(servinfo); // all done with this structure

    for (;;) {
        listen(sockfd,10);
        if (debug){
            cout << "entered loop" << endl;
        }
        addr_size = sizeof their_addr;
        if ((newfd = accept(sockfd, (struct sockaddr *) &their_addr, &addr_size)) < 0) {
            cerr << "connection failed" << endl;
            return -1;
        } else{
            if ((nbytes = recv(newfd, buf, MAXDATASIZE, 0)) <= 0) {
                // got error or connection closed by server
                if (nbytes == 0) {
                    // connection closed
                    printf("socket disconnected\n");
                    exit(5);
                } else {
                    perror("recv");
                    exit(6);
                }
            } else{
                string temp;
                int count = 0;
                for(int i=0;i<strlen(buf);i++){
                    if(buf[i] == ','){
                        if (info.size() == count){
                            info.resize(info.size()*2);
                        }
                        info.at(count++) = temp;
                        temp = "";
                    } else{
                        temp.push_back(buf[i]);
                    }
                }
                if (debug){
                    cout << "count: " << count << endl;
                }
                info.resize(count);
                request = info.at(0);
                cout << "request: " << request << endl;
                // no step stones sent, run the wget and send back
                if (count == 1){
                    // run wget on the requested page

                } else{ // still more stones to step through, choose one and go
                    // number of stones in the list:
                    count = (count-1)/2;
                    // choose a random one
                    count = rand()%count;
                }
            }
        }
    }
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