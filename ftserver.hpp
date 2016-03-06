#ifndef FTSERVER_H
#define FTSERVER_H
/**
 * File:    ftserver.hpp
 * Author:  Daniel Bonnin
 * email:   bonnind@oregonstate.edu
 *
 * Descr:   This file contains the interfaces of ftserver, a simple 
 *          server-client file transfer application.
 *
 *          ftserver provides 2 services: 1) "list": list the contents of the
 *          current directory 2) "Get File": Get the requested file by name.
 *
 *          ftserver fulfills requests over a second tcp connection on a 
 *          port specified by the client. 
 *
 *          ftserver will wait on the specified port until a client connects
 *          or a keyboard interrupt is received.  
 */

// Valid ftserver port ranges
#define PORT_MAX 65535
#define PORT_MIN 1024
#define USAGE "Usage: ./ftserver <int PORTNO (1024 - 65535)>\n"

// ftserver can only accomodate 1 concurrent connection while not
// multi-threaded
#define MAX_INCOMING_CONNECTIONS 1

#define RECV_BUF_LEN 1024  // Socket incoming buffer

#define MAX_SEND_LEN 8096  // Max send buffer 
#define MAX_HOST_LEN 256  // Client hostname buffer size

// Intentifiers from client which delimit client commands inside
// of '<>' braces (eg. "<dataport>44444</dataport>").
#define GET_COMMAND "g" 
#define LIST_COMMAND "l"
#define PORT_TAG "dataport"

/*
 * Process commandline port argument.
 * @param argc number of commandline arguments
 * @param argv array of argument strings
 * @return valid port number or -1 on invalid
 */
int getPort(int argc, char** argv);

/*
 * Wait on a socket for a client to connect.   
 * 
 * @param portno port number to listen on.
 * @param notKilled whether keyboard interrupt has been received
 *
 * @pre portno is valid.
 */
int waitForClient(const char *portno, bool *notKilled);


/**
 *  Send response to client on specified port
 *
 *  @param response the entire data to send
 *  @param clientAddr ip address of client
 *  @param portno the port specified by client 
 */
int sendResponse(
        std::string response, 
        struct sockaddr *clientAddr, 
        socklen_t *addrlen, 
        int portNo);

/**
 * Process response based on client request
 *
 * @param msg Client request message
 * @param portNo the client's requested response port
 * @param cHostname the client's hostname 
 *
 * @return formatted data to send back to client 
 */
std::string parseCommand(std::string msg, int *portNo, std::string cHostname);

/**
 * Return a listing of files in this directory
 *
 * @return string tag-delimited file and directory listing
 */
std::string lsCWD();

/**
 * Search this directory for filename
 *
 * @param fileName a file or directory name
 * 
 * @return if fileName exists in this directory
 */
bool fileExists(std::string filename);

/**
 * Return string representation of file
 *
 * @param filename The relative path of the file to read
 * 
 */
std::string fileAsString(std::string);

/**
 * Return contents of tag or empty string
 *
 * @param tag the string inside xml style '<>' braces to find
 * @param msg The unprocessed client request string
 *
 * @return  The string between the first instance of openTag and 
 *          closeTag
 */
std::string parseTag(std::string tag, std::string msg);

/**
 * Return a listing of files in this directory
 *
 * @param cHostname client hostname (for status message)
 * @return string tag-delimited file and directory listing
 */
std::string lsCWD();

#endif
