#include <cstring>
#include <assert.h>
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
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int runTheGet(unsigned int index) {
    int next = 0;
    if (index == 0) {
        next = 0;
    } else {
        next = rand() % (index);
    }
    stepStone stone = ss.at(static_cast<unsigned long>(next));
    ss.erase(ss.begin() + next);
    cout << "next SS is " << stone.addr << ", " << stone.port << endl;
    cout << "waiting for file..." << endl;
    const char *PORTNUM = (stone.port).c_str();
    const char *IP = (stone.addr).c_str();
    int sok = 0;     // stone socket descriptor
    //struct sockaddr_storage remoteaddr{}; // client address
    char buf[MAXDATASIZE];
    memset(buf, 0, sizeof(buf));
    ssize_t nbytes;
    string input;
    int yes = 1;        // for setsockopt() SO_REUSEADDR, below
    int status;
    struct addrinfo hints{}, *ai;// will point to the results
    bool empty = true;
    bool sendit = true;
    // output file
    ofstream out(filename);

    if (index != 1) {
        empty = false;
        //build the list of stones to send, if neccessary
        for (auto &s : ss) {
            // add values to the string that will be sent to the stone later via buf
            sendStones.append(s.addr);
            sendStones.append(",");
            sendStones.append(s.port);
            sendStones.append(",");
        }
    }
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

    if ((connect(sok, ai->ai_addr, ai->ai_addrlen)) < 0) {
        cerr << "connection failed" << endl;
        return -1;
    }

    // where request ends, and stones start
    int dex = 0;
    // initially, buffer just contains the requested url
    for (unsigned long i = 0; i < request.length(); i++) {
        buf[i] = request.at(i);
        dex++;
    }
    buf[dex++] = ',';
    freeaddrinfo(ai); // all done with this

    for (;;) {
        if (sendit) {
            if (debug) {
                cout << "sending: " << buf << endl;
            }
            // if the stepping stone list isn't empty, add the stones to buf
            if (!empty) {
                for (unsigned long i = 0; i < sendStones.length(); i++) {
                    if (i == strlen(buf)) {
                        // send what you've got, empty buf, then keep going
                        if (debug) {
                            cout << "buffer full, sending: " << buf << endl;
                            return 0;
                        }
                        if (send(sok, buf, MAXDATASIZE, 0) == -1) {
                            perror("send failed");
                            exit(6);
                        }
                    }
                    buf[i + dex] = sendStones[i];
                }
            }
            if (send(sok, buf, strlen(buf), 0) == -1) {
                perror("send failed");
                exit(6);
            }
            sendit = false;
        } else {
            memset(buf, 0, sizeof(buf));
            if ((nbytes = recv(sok, buf, MAXDATASIZE, 0)) <= 0) {
                // got error or connection closed by server
                if (nbytes == 0) {
                    // connection closed
                    if (debug)
                        printf("get socket disconnected\n");
                    return 0;
                } else {
                    perror("recv");
                    exit(6);
                }
            } else {
                if (debug) {
                    cout << "page: " << buf << endl;
                }
                // see if EOF has been sent, if so job is done
                if (buf[MAXDATASIZE] == 0) {
                    out << buf;
                    //close(sok);
                    return 0;
                }
                out << buf;
            }
        }
    }
}

int startListen() {
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
    int lastSlash = 0;

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
        fprintf(stderr, "failed to bind\n");
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
                        if (debug) {
                            printf("New connection from %s on "
                                       "socket %d\n",
                               inet_ntop(remoteaddr.ss_family,
                                         get_in_addr((struct sockaddr *) &remoteaddr),
                                         remoteIP, INET6_ADDRSTRLEN),
                               newfd);
                        }
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
                            unsigned int count = 0;
                            for (unsigned int k = 0; k < strlen(buf); k++) {
                                if (buf[k] == ',') {
                                    if (info.size() == count) {
                                        info.resize(info.size() * 2);
                                    }
                                    info.at(count++) = temp;
                                    temp = "";
                                } else {
                                    temp.push_back(buf[k]);
                                }
                            }
                            if (debug) {
                                cout << "count: " << count << endl;
                            }
                            info.resize(count);
                            request = info.at(0);
                            lastSlash = static_cast<int>(request.find_last_of('/'));
                            if (lastSlash < 0) {
                                filename = const_cast<char *>("index.html");
                            } else {
                                temp = request.substr(static_cast<unsigned long>(lastSlash));
                                filename = const_cast<char *>(temp.c_str());
                            }
                            cout << "request: " << request << endl;
                            // no step stones sent, run the wget and send back
                            if (count == 1) {
                                cout << "chainlist is empty" << endl;
                                // run wget on the requested page
                                cout << "issuing wget for file " << filename << endl;
                                // based on example found at: http://www.cplusplus.com/forum/unices/790/
                                string cmd = "wget " + request;
                                if (system(cmd.c_str()) < 0){
                                    perror("wget failed");
                                    exit(7);
                                } else {
                                    cout << "File received" << endl;
                                }
                                // get the goods, then get ready to send it back
                                cout << "relaying file..." << endl;
                                memset(buf,0,sizeof(buf));
                                if(debug) {
                                    if (send(i, filename, strlen(filename), 0) < 0) {
                                        perror("send failed");
                                        exit(6);
                                    }
                                } else{
                                    char *file = readFile(filename);
                                        int filled = 0;
                                        unsigned int n = 0;
                                    for (n = 0; n < strlen(file); n) {
                                        // fix hardcoding of 1024, but ok for now
                                        while (filled < 1024 && n != strlen(file)) {
                                            buf[filled++] = file[n++];
                                        }
                                        if (send(i, buf, MAXDATASIZE, 0) < 0) {
                                                perror("transmission of webpage failed");
                                                exit(3);
                                        } else {
                                            memset(buf, 0, sizeof(buf));
                                                filled = 0;
                                            }
                                        }
                                    cmd = "rm ";
                                    cmd.append(filename); // hardcoded for now
                                    if (system(cmd.c_str()) < 0) {
                                        perror("could not delete file after use");
                                        exit(5);
                                    }
                                    cout << "Goodbye!" << endl;
                                }

                            } else { // still more stones to step through, choose one and go
                                // number of stones in the list:
                                count = (count - 1) / 2;
                                unsigned int x = 1;
                                ss = vector<stepStone>(count);
                                for (unsigned int j = 0; j < count; j++) {
                                    ss.at(j).addr = info.at(x++);
                                    ss.at(j).port = info.at(x++);
                                }
                                cout << "Chainlist is" << endl;
                                for (auto &s : ss) {
                                    cout << s.addr << ", " << s.port << endl;
                                }

                                runTheGet(count);
                                cout << "relaying file" << endl;
                                memset(buf, 0, sizeof(buf));
                                if (debug) {
                                    if (send(i, filename, strlen(filename), 0) < 0) {
                                        perror("send failed");
                                        exit(6);
                                    }
                                } else {
                                    string bulletproofFName = filename;
                                    //bulletproofFName.append("");
                                    char *file = readFile(bulletproofFName);
                                    int filled = 0;
                                    if (file != nullptr) {
                                        for (unsigned int n = 0; n < strlen(file); n) {
                                            // fix hardcoding of 1024, but ok for now
                                            while (filled < 1024 && n != strlen(file)) {
                                                buf[filled++] = file[n++];
                                            }
                                            if (n != strlen(file)) {
                                                if (send(i, buf, MAXDATASIZE, 0) < 0) {
                                                    perror("transmission of webpage failed");
                                                    exit(3);
                                                } else {
                                                    memset(buf, 0, sizeof(buf));
                                                    filled = 0;
                                                }
                                            } else {
                                                if (send(i, buf, static_cast<size_t>(filled), 0) < 0) {
                                                    perror("transmission of webpage failed");
                                                    exit(3);
                                                } else {
                                                    memset(buf, 0, sizeof(buf));
                                                    filled = 0;
                                                }
                                            }
                                        }
                                    }
                                    string cmd = "rm ";
                                    cmd.append(bulletproofFName); // hardcoded for now
                                    if (system(cmd.c_str()) < 0) {
                                        perror("could not delete file after use");
                                        exit(5);
                                    }
                                    cout << "Goodbye!" << endl;
                                }
                            }
                            close(i);
                            FD_CLR(i, &master); // remove from master set
                        }
                }
            }
        }
    }
}

char *readFile(const string in) {
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
    } else cout << "Unable to open file " << in << endl;
    return nullptr;
}

// based on example at: https://stackoverflow.com/questions/212528/get-the-ip-address-of-the-machine
void GetPrimaryIp(char *buffer, size_t buflen) {
    assert(buflen >= 16);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    assert(sock != -1);

    const char *kGoogleDnsIp = "8.8.8.8";
    uint16_t kDnsPort = 53;
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(kGoogleDnsIp);
    serv.sin_port = htons(kDnsPort);

    int err = connect(sock, (const sockaddr *) &serv, sizeof(serv));
    assert(err != -1);

    sockaddr_in name;
    socklen_t namelen = sizeof(name);
    err = getsockname(sock, (sockaddr *) &name, &namelen);
    assert(err != -1);

    const char *p = inet_ntop(AF_INET, &name.sin_addr, buffer, buflen);
    assert(p);

    close(sock);
}

int main(int argc, char **argv){
    int portArg = 0;
    char hostname[128];
    srand(static_cast<unsigned int>(time(nullptr)));
    gethostname(hostname,sizeof hostname);
    // hostname to ip code based off example at: http://www.cplusplus.com/forum/articles/9742/
    char buffer[128];
    size_t buflen = 128;
    GetPrimaryIp(buffer, buflen);
    ip = buffer;
    if (argc == 1){
        port = "9001";
    } else{
        // check that its a valid number
        istringstream str(argv[1]);
        if (!(str >> portArg)){
            cerr << "Invalid port number" << endl;
            return usage();
        }
        port = argv[1];
    }
    // print hostname and port
    cout << "SteppingStone: " << ip << ", " << port << endl;
    startListen();
}