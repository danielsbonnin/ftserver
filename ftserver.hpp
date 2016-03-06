#ifndef FTSERVER_H
#define FTSERVER_H
/*
 * Process commandline port argument.
 * @param argc number of commandline arguments
 * @param argv array of argument strings
 * @return valid port number or -1 on invalid
 */

#define PORT_MAX 65535
#define PORT_MIN 1024
#define USAGE "Usage: ./ftserver <int PORTNO (1024 - 65535)>\n"
#define MAX_INCOMING_CONNECTIONS 1
#define RECV_BUF_LEN 1024
#define MAX_SEND_LEN 8096 
#define MAX_HOST_LEN 256
#define GET_COMMAND "g"
#define LIST_COMMAND "l"
#define PORT_TAG "dataport"
int waitForClient(const char *portno, bool *notKilled);
int getPort(int argc, char** argv);
int sendResponse(
        std::string response, 
        struct sockaddr *clientAddr, 
        socklen_t *addrlen, 
        int portNo);
std::string parseCommand(std::string msg, int *portNo, std::string cHostname);
std::string lsCWD();
bool fileExists(std::string filename);
std::string fileAsString(std::string);
#endif
