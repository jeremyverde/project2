#include <cstring>
#include "awget.h"

using namespace std;

// debuging boolean
bool debug = false;

/// Print the correct usage in case of user syntax error.
int usage()
{
    cout << "Usage: to start ss with specific port, run \"./ss <port number>\"" << endl;
    cout << "To start ss with default port number, run \"./ss\"" << endl;
    return -1;
}

// get sockaddr, IPv4 or IPv6:
void *awget::get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int awget::startListen() {
    // code based largely off of beej's guide example program, see README
    fd_set master{};    // master file descriptor list
    fd_set read_fds{};  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number
    struct sockaddr_storage remoteaddr; // client address
    struct addrinfo hints{}, *ai, *p;
    char remoteIP[INET6_ADDRSTRLEN];
    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    socklen_t addrlen;
    ssize_t nbytes;
    char buf[MAXDATASIZE];
    memset(buf,0,sizeof(buf));
    int rv;
    int listener = 0;
    info = vector<string>(10);
    string input;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int newfd;
    char page[] = "index.html";

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    if ((rv = getaddrinfo(ip, port, &hints, &ai)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for(p = ai; p != nullptr; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // if we got here, it means we didn't get bound
    if (p == nullptr) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this structure

    if(listen(listener,10) < 0){
        perror("listen failed");
        exit(5);
    }
    FD_SET(listener, &master);
    fdmax = listener;

    for (;;) {
        read_fds = master; // copy it
        if (select(fdmax + 1, &read_fds, nullptr, nullptr, nullptr) == -1) {
            perror("select");
            exit(4);
        }
        for (int i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                                   (struct sockaddr *) &remoteaddr,
                                   &addrlen);

                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        printf("selectserver: new connection from %s on "
                                       "socket %d\n",
                               inet_ntop(remoteaddr.ss_family,
                                         get_in_addr((struct sockaddr *) &remoteaddr),
                                         remoteIP, INET6_ADDRSTRLEN),
                               newfd);
                    }
                } else {
                        // handle data from a client
                        if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                            // got error or connection closed by client
                            if (nbytes == 0) {
                                // connection closed
                                if(debug)
                                    printf("connection closed: socket %d disconnected\n", i);
                            } else {
                                perror("recv");
                            }
                            close(i); // bye!
                            FD_CLR(i, &master); // remove from master set
                        } else {
                            string temp;
                            unsigned long count = 0;
                            for (unsigned int i = 0; i < strlen(buf); i++) {
                                if (buf[i] == ',') {
                                    if (info.size() == count) {
                                        info.resize(info.size() * 2);
                                    }
                                    info.at(count++) = temp;
                                    temp = "";
                                } else {
                                    temp.push_back(buf[i]);
                                }
                            }
                            if (debug) {
                                cout << "count: " << count << endl;
                            }
                            info.resize(count);
                            request = info.at(0);
                            cout << "request: " << request << endl;
                            // no step stones sent, run the wget and send back
                            if (count == 1) {
                                // run wget on the requested page
                                // based on example found at: http://www.cplusplus.com/forum/unices/790/
                                cout << "last stone, getting page" << endl;
                                string cmd = "wget " + request;
                                if (system(cmd.c_str()) < 0){
                                    perror("wget failed");
                                    exit(7);
                                }

                                // get the goods, then get ready to send it back
                                memset(buf,0,sizeof(buf));
                                if(debug) {
                                    if (send(i, page, strlen(page), 0) < 0) {
                                        perror("send failed");
                                        exit(6);
                                    }
                                } else{
                                        char *file = readFile(page);
                                        int filled = 0;
                                        unsigned int n = 0;
                                        for (unsigned int j = 0;j < strlen(file);j++) {
                                            // fix hardcoding of 1024, but ok for now
                                            while(filled < 1024 && n!=strlen(file)){
                                                buf[filled++] = file[n++];
                                            }
                                            if(send(i,buf,MAXDATASIZE,0) < 0){
                                                perror("transmission of webpage failed");
                                                exit(3);
                                            } else{
                                                memset(buf,0,sizeof(buf));
                                                filled = 0;
                                            }
                                        }
                                        // tear down the connection, get rid of the web file, then get back to waiting
                                        // close(i);
                                        cmd = "rm index.html"; // hardcoded for now
                                        if(system(cmd.c_str()) < 0){
                                            perror("could not delete file after use");
                                            exit(5);
                                        }
                                }
                            } else { // still more stones to step through, choose one and go
                                // number of stones in the list:
                                count = (count - 1) / 2;
                                // choose a random one
                                count = rand() % count;
                            }
                        }
                }
            }
        }
    }
}

char *awget::readFile(char * in){
    // based on example at: http://www.cplusplus.com/doc/tutorial/files/
    streampos size;
    char * memblock;

    ifstream file (in, ios::in|ios::binary|ios::ate);
    if (file.is_open())
    {
        size = file.tellg();
        memblock = new char [size];
        file.seekg (0, ios::beg);
        file.read (memblock, size);
        file.close();
        if(debug) {
            cout << "the entire file content is in memory" << endl;
        }
        return memblock;
    }
    else cout << "Unable to open file";
    return nullptr;
}


int main(int argc, char **argv){
    int portArg = 0;
    char hostname[128];
    srand(time(NULL));
    awget stone = awget();
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