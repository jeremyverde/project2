#include <iostream>
#include <unistd.h>
#include <sstream>

using namespace std;

bool debug = false;

/// Print the correct usage in case of user syntax error.
int usage()
{
    cout << "Usage: to start awget, run \"./awget <URL> [-c <chain file name>\"\n" << endl;
    return -1;
}

int main(int argc, char **argv) {
    int cFlag = 0;
    int hFlag = 0;
    char *chainfile = nullptr;
    char *arg = nullptr;
    int index = 0;
    int c = 0;

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


    return 0;

}