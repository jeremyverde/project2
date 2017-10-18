#include "awget.h"

using namespace std;

// set for debugging output on/off
bool debug = false;

/// Print the correct usage in case of user syntax error.
int usage()
{
    cout << "Usage: to start awget, run \"./awget <URL> [-c <chain file name>]\"\n" << endl;
    return -1;
}

int runTheGet(unsigned int index) {
    int next = rand() % index;
    stepStone stone = ss.at(static_cast<unsigned long>(next));
    ss.erase(ss.begin() + next);
    ss.resize(index - 1);
    cout << "next SS is " << stone.addr << ", " << stone.port << endl;
    cout << "waiting for file..." << endl;
    const char *PORTNUM = (stone.port).c_str();
    const char *IP = (stone.addr).c_str();
    int sok = 0;     // stone socket descriptor
    //struct sockaddr_storage remoteaddr{}; // client address
    char buf[MAXDATASIZE];
    memset(buf,0,sizeof(buf));
    ssize_t nbytes;
    string input;
    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int status;
    struct addrinfo hints{}, *ai;// will point to the results
    bool empty = true;
    bool sendit = true;
    // output file
    int lastSlash = static_cast<int>(request.find_last_of('/'));
    if (lastSlash < 0) {
        filename = const_cast<char *>("index.html");
    } else {
        string temp = request.substr(static_cast<unsigned long>(lastSlash));
        filename = const_cast<char *>(temp.c_str());
    }
    ofstream out(filename);

    if (index >= 1) empty = false;
    //build the list of stones to send, if neccessary
    for (auto &s : ss) {
        // add values to the string that will be sent to the stone later via buf
        sendStones.append(s.addr);
        sendStones.append(",");
        sendStones.append(s.port);
        sendStones.append(",");
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

    if ((connect(sok, ai->ai_addr, ai->ai_addrlen)) < 0){
        cerr << "connection failed" << endl;
        return -1;
    }

    // where request ends, and stones start
    int dex = 0;
    // initially, buffer just contains the requested url
    for (unsigned long i = 0;i < request.length();i++){
        buf[i] = request.at(i);
        dex++;
    }
    buf[dex++] = ',';
    freeaddrinfo(ai); // all done with this

    for(;;){
        if(sendit) {
            if (debug) {
                cout << "sending: " << buf << endl;
            }
            // if the stepping stone list isn't empty, add the stones to buf
            if(!empty) {
                for (unsigned long i = 0; i < sendStones.length(); i++) {
                    if (i == strlen(buf)) {
                        // send what you've got, empty buf, then keep going
                        if (debug) {
                            cout << "buffer full, sending: " << buf << endl;
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
        } else{
            memset(buf,0,sizeof(buf));
            if ((nbytes = recv(sok, buf, MAXDATASIZE, 0)) <= 0) {
                // got error or connection closed by server
                if (nbytes == 0) {
                    // connection closed
                    printf("socket disconnected\n");
                    exit(5);
                } else {
                    perror("recv");
                    exit(6);
                }
            }
            else{
                if(debug){
                    cout << "page: " << buf << endl;
                }
                // see if EOF has been sent, if so job is done
                if (buf[MAXDATASIZE] == 0) {
                    out << buf;
                    //close(sok);
                    cout << "Goodbye!" << endl;
                    return 0;
                }
                out << buf;
            }
        }
    }
}

int main(int argc, char **argv) {
    // seed random for later
    srand(static_cast<unsigned int>(time(nullptr)));
    // local vars
    int cFlag = 0;
    int hFlag = 0;
    char *chainfile = nullptr;
    char *arg = nullptr;
    unsigned int index;
    int c = 0;

    // check that there is at least one argument before proceeding
    if (argc <= 1) return usage();
    // print out the name of the requested page
    request = argv[1];
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
    ss.resize(index);
    // temp variables to hold ss info
    string addr;
    string port;
    int portNum;
    string delim = " ";
    // loop for designated number of stepping stones, add to list
    for (unsigned int i = 0; i < index; i++){
        istr0 >> addr;
        istr0 >> port;
        // check that the port number is valid
        istringstream strm(port);
        if (!(strm >> portNum)){
            cerr << "Invalid number " << port << endl;
            return -1;
        }
        // add empty object to list, then fill it in from file
        ss.emplace_back(stepStone());
        ss.at(i).addr = addr;
        ss.at(i).port = port;
        if (istr0.fail() && !istr0.eof()) {
            cerr << "file not formatted correctly, exiting." << endl;
            return -1;
        }
    }
    if (debug){
        cout << "Index: " << index << endl;
    }
    cout << "Chainlist is" << endl;
    for (unsigned int i = 0; i < index;i++){
        cout << ss.at(i).addr << ", " << ss.at(i).port << endl;
    }
    if (runTheGet(index) == 0) {
        cout << "received file " << request << endl;
        string cmd = "xdg-open index.html.1";
        if (system(cmd.c_str()) < 0) {
            perror("could not delete file after use");
            exit(5);
        }
    }
    return 0;
}