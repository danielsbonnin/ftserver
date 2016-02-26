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
#define GET_COMMAND "g"
#define LIST_COMMAND "l"
int waitForClient(const char *portno);
int getPort(int argc, char** argv);
int sendResponse(std::string response, struct sockaddr * clientAddr, int *addrlen, int portNo);
std::string parseCommand(std::string msg, int *portNo);
#endif
