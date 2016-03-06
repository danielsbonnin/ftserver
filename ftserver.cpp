/**
 * File:    ftserver.cpp
 * Author:  Daniel Bonnin
 * email:   bonnind@oregonstate.edu
 *
 * Descr:   This file contains the implementation of ftserver, a simple 
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
#include <iostream>    
#include <stdio.h>
#include <string>
#include <cstring>
#include <fstream>      // file io
#include <dirent.h>     // directory services
#include <stdexcept>    // exception handling  
#include <netdb.h>      // socket-related data structures (addrinfo etc)
#include <arpa/inet.h>  // inet_ntoa()
#include <csignal>      // signal handling
#include "ftserver.hpp"
using namespace std;

bool notKilled = true; // Do not gracefully quit

// Handle keyboard interrupt.
// Print status message
// Set notKilled to false 
static void signalHandler(int signum) {
    cout << "Keyboard interrupt received\n";
    notKilled = false;
    exit (signum);
}

int main(int argc, char** argv) {
    int portno = 0;
    // Connect signal handler to gracefully close server socket on interrupt
    signal(SIGINT, signalHandler);   

    // Get valid port number argument
    if ((portno = getPort(argc, argv)) == -1)
	    return 0;  // Gracefully close on invalid port argument.
	
	cout << "Server open on " << portno << "\n";
    
    //Call server function on port arg.
	waitForClient(to_string((long long)portno).c_str(), &notKilled);

	return 0;
}

/**
 * Process commandline args
 *
 * @param argv Commandline argument array
 * @return valid port number or -1
 */
int getPort(int argc, char **argv) {
	int portno = 0;

	if (argc != 2) { //Invalid num args.
		cout << "Invalid Command Line Arguments\n" << USAGE;
		return -1;
	}

	/* 
     * Help for exception handling: 
	 * http://www.cplusplus.com/reference/stdexcept/invalid_argument/
	 */
	try {
		portno = stoi(argv[1]);
	}
	catch (const invalid_argument &e) // non-integer entered.
	{
		cout << "Please enter an integer port argument.\n" << USAGE;
		return -1;
	}
    
    // Port number argument out of range
	if (portno < PORT_MIN || portno > PORT_MAX) {
		cout << "Port Number argument out of range\n" << USAGE;
		return -1;
	}
	else
		return portno;
}

/*
 * Wait on a socket for a client to connect.   
 * 
 * @param portno port number to listen on.
 * @param notKilled whether keyboard interrupt has been received
 *
 * @pre portno is valid.
 */
int waitForClient(const char *portno, bool *notKilled) {
	
	int status;
    int  s = 0;  // The server socket
	int c = 0; // The client in the control connection.
    int dataPortNo = 0;  // Connecting client's specified receiving port
	int recvRetVal; // Bytes read or error

    // Obtained much socket data structure help from Beej's Guide: 
    // https://beej.us/guide/bgnet/output/html/multipage/ipstructsdata.html
    socklen_t addrlen = sizeof(struct sockaddr_storage);
	struct addrinfo hints;
	struct addrinfo *servinfo;  // will point to the results
	struct addrinfo *next;  // Next in linked list of ip addresses
    struct sockaddr_storage c_addr;  // Address info about client
	char buffer[RECV_BUF_LEN];  // holds incoming data
    char clientHost[MAX_HOST_LEN]; // The connecting client hostname

    memset(&clientHost, 0, MAX_HOST_LEN);  // Zero-out clientHost
    memset(&hints, 0, sizeof hints); // Zero-out hints
	hints.ai_family = AF_INET;  // ipv4 socket
	hints.ai_socktype = SOCK_STREAM; // TCP, not UDP
	hints.ai_flags = AI_PASSIVE; // fill in my ip for me

    // Fill in socket info from hints into servinfo
	if ((status = getaddrinfo(NULL, portno, &hints, &servinfo)) != 0) {
		perror("Error with getaddrinfo");
		return 0;
	}
    
    // The following for loop heavily borrowed from Beej's guide (link above)
	// Walk the linked list of struct addrinfos in order to find a valid one. 
	for (next = servinfo; next != NULL; next = next->ai_next) {
        // Init the server socket
        if ((s = socket(
                next->ai_family, 
                next->ai_socktype, 
                next->ai_protocol))
                == -1) {
            // Call to socket failed. 
            // Report error, but try the next addrinfo in linked list.
            perror("Server socket");
            continue;
        }

	    // Bind socket to port (specified in struct addrinfo hints)
	    if (bind(s, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
            close(s);
            // Call to bind() failed. 
            // Report error, but try the next addrinko in linked list.
            perror("Bind");
            continue;
        }
    }

    // Free the memory for servinfo now that it's not needed
    freeaddrinfo(servinfo);

    // Prepare socket s to accept clients
	if ((listen(s, MAX_INCOMING_CONNECTIONS)) == -1) {
	    perror("Listen");
        return 0;
    }
	
	// Accept clients until interrupt is received. 
	while (*notKilled) {

        // Client has attempted to connect
		if ((c = accept(s, (struct sockaddr *)&c_addr, &addrlen)) == -1) {	
			perror("Accept");
            close(s);
			return 0;
		}
        
        // Cast first item in c_addr as sockaddr *
        struct sockaddr * clientAddr = (struct sockaddr *)&c_addr; 
        
        // Fill client's hostname (clientHost) using the info in clientAddr
        getnameinfo(clientAddr, addrlen, clientHost, MAX_HOST_LEN, NULL, 0, 0);
        
        // std::string repr of clientHost (For ease of passing as argument)
        string cHostname(clientHost);  
        cout << "Connection from " << cHostname << endl;
		
        // Help for the following do-while loop obtained at msdn:
        // msdn.microsoft.com/en-us/windows/desktop/bb530746(v=vs.85).aspx 
        
		// Loop recv until no more data to read.
		do {
			dataPortNo = 0;
			recvRetVal = recv(c, buffer, RECV_BUF_LEN, 0);
			if (recvRetVal > 0) {  // Data has been received
                
                // Create a std::string from the buffer
                string bufString(buffer);
				
                // Ensure that no junk data is included in bufString
                bufString = bufString.substr(0, recvRetVal);

                // get a formatted response to send to client
                string response = parseCommand(bufString, &dataPortNo, cHostname);
                
                // Check for abort condition
                if (dataPortNo == -1) {  // Problem with client port
                    close(c);
                    break;
                }
                // Allow client time to open a socket and listen.   
                sleep(1);

                // Send response to client on client-specified port 
                sendResponse(
                        response, 
                        (struct sockaddr*)&c_addr, 
                        (socklen_t*) &addrlen, 
                        dataPortNo);
			}

            // All data has been received
			else if (recvRetVal == 0) {
                close(c);
		    }	

            // -1 indicates an error condition
            else {
			    perror("Recv");
                close(c); 
			}
		} while (recvRetVal > 0);  // More data to read on socket
	}

    // Close server and client sockets.
    close(c);
	close(s);

	return 0;
}

/**
 *  Send response to client on specified port
 *
 *  @param response the entire data to send
 *  @param clientAddr ip address of client
 *  @param portno the port specified by client 
 */
int sendResponse(
        string response, 
        struct sockaddr* clientAddr, 
        socklen_t* addrlen, 
        int portNo){
   
    //Create data structures for connection
	int dataSocket = 0; // Socket descriptor
    int totalSent = 0;  // Total bytes sent
    int sent = 0;       // Bytes sent this iteration
    int toSend = 0;     // Bytes to send this iteration
    int responseLength = response.length();

	/* 
     * Much of the socket code in this function is paraphrased from Beej's guide
     * I tried to make this my original work, but I fully attribute any 
     * similarities of this file to Beej's guide to the author of that 
     * guide. 
	 * Source: https://beej.us/guide/bgnet/output/html/multipage/(continued)
     * (continuation)syscalls.html#getpeername
     */
	struct addrinfo hints;  // Address struct to fill for socket
    
    //The client program is the "server" for this data connection.
	struct addrinfo *serverInfo; 

    // Zero-out the memory of hints.
	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_UNSPEC;  
	hints.ai_socktype = SOCK_STREAM;  // TCP socket
    
    /*
	 * Ip address parsing help obtained from accepted answer here:
	 * http://stackoverflow.com/questions/1276294/(continued)
     * (continuation)getting-ipv4-address-from-a-sockaddr-structure
	 */
    
    // convert byte order from network byte order (for portability)
	char *ip = inet_ntoa(((sockaddr_in*)clientAddr)->sin_addr);
    
    // Get portnumber into a cstring for use in getaddrinfo()
    string portNumber = to_string((long long)portNo);
    
    // Set up serverInfo struct
	if ((getaddrinfo(ip, portNumber.c_str(), &hints, &serverInfo))!= 0) {
        perror("getaddrinfo");
        return 0;
    }

    // Create client socket on serverInfo struct info
	dataSocket = socket(
            serverInfo->ai_family, 
            serverInfo->ai_socktype, 
            serverInfo->ai_protocol);
	
    // Create TCP connection
    if (connect(dataSocket, serverInfo->ai_addr, serverInfo->ai_addrlen) != 0) {
        perror("Connect");
        close(dataSocket);
        return 0;
    }    

    // socket info no longer needed.
    free(serverInfo);

    // Send the response to client
    // MSG_NOSIGNAL prevents broken pipe signal
    while (totalSent < responseLength) {
        toSend = responseLength - totalSent;

        // Either send MAX_SEND_LEN bytes or remaining bytes, whichever is less
        toSend = (toSend < MAX_SEND_LEN) ? toSend : MAX_SEND_LEN;
        sent = send(
            dataSocket, 
            response.substr(totalSent).c_str(), 
            toSend, 
            MSG_NOSIGNAL);
        if (sent == -1) {
            perror("Send");
            close(dataSocket);
            return 0; 
        } 
        else {
            totalSent += sent;
        }
    }
       
	close(dataSocket);
	return 1;
}

/**
 * Return contents of tag or empty string
 *
 * @param tag the string inside xml style '<>' braces to find
 * @param msg The unprocessed client request string
 *
 * @return  The string between the first instance of openTag and 
 *          closeTag
 */
string parseTag(string tag, string msg) {
    int dataLength = 0;  // Character length of content between tags
    
    // Add braces to tag to differentiate open and close tags
    const string openTag("<" + tag + ">");
    const string closeTag("</" + tag + ">");

    // Calculate length of content
    dataLength = msg.find(closeTag) - msg.find(openTag) - openTag.length();
    
    // Verify content length and cut out tags.
    if ( dataLength > 0)
        return msg.substr((msg.find(openTag) + openTag.length()), dataLength);
    else 
        return "";
}

/**
 * Process response based on client request
 *
 * @param msg Client request message
 * @param portNo the client's requested response port
 * @param cHostname the clien's hostname(for status messages)
 *
 * @return formatted data to send back to client 
 * @return portNo* is now the int value of the client's requested data port
 */
string parseCommand(string message, int *portNo, string cHostname) {
    string tagContents;
    string port;
	string returnMSG = "";
    string filename;
    int * portno = portNo; 
    // String port number from client message
    port = parseTag(PORT_TAG, message);

	/* 
     * Help for exception handling: 
	 * http://www.cplusplus.com/reference/stdexcept/invalid_argument/
	 */
	try {
		*portno = stoi(port);
	}
	catch (const invalid_argument &e) // non-integer entered.
	{
        *portno = -1;
	}

	if (*portno < 1024 || *portno > 65535) {
		cout << "Client sent invalid data port number" << endl;
        *portno = -1;
        return "";
    }
    // Check for "Get file" command
	if ((tagContents = parseTag(GET_COMMAND, message)).compare("") != 0) 
    {
		filename = tagContents;

        // Client sent "Get" command. 
        // Validate filename and add file to return message.

        // Print status message to terminal.
        cout << "File \"" << filename << "\" requested on port " << port << ".\n";
        
        // If valid filename, add file to return string.
        if(fileExists(filename)) {  // File exists in current directory
            returnMSG = 
                "<ok><name>"            + 
                filename                + 
                "</name><data>"         + 
                fileAsString(filename)  + // Entire file as string. 
                "</data></ok>";

            // Print status message to terminal.
            cout << "Sending \"" << filename << "\" to ";
            cout << cHostname << ":" << port << endl ; 
        }
        else {  // filename is invalid
            // Add error message to return string
            returnMSG = "<error>FILE NOT FOUND</error>";

            // Print status message to terminal
            cout << "File not found. Sending error message to ";
            cout << cHostname << ":" << port << endl;
        }
	}  // End "Get file" command
	
    // Check for "list" command.
    else if ((tagContents = parseTag(LIST_COMMAND, message)).compare("") != 0) 
    { 
        // Print status message to terminal
        cout << "List directory requested on port " << port << " \n";    
		
        // List files in current directory into a tag-delimited string
		string fileNames = lsCWD();
	
        //encapsulate directory object names into a message
		returnMSG = "<ok><list>" + fileNames + " </list></ok>";
        // Print status message to terminal.
        cout << "Sending directory contents to ";
        cout << cHostname << ":" << port << endl ; 
	}
	else { 
		//encapsulate error message into a message
		returnMSG = "<error>Command " + tagContents + " not recognized</error>";
	}
	return returnMSG;
}

/**
 * Return string representation of file
 *
 * @param filename The relative path of the file to read
 * 
 */
string fileAsString(string filename) {
   
    // File stream refresher provided by:
    // www.cplusplus.com/doc/tutorial/files
   
    string line, fileString;
   
    // Attempt to open filename.
    // ifstream default flag is RDONLY
    ifstream file(filename);

   if (file.is_open()) {
        // Below while loop obtained from source (above)
        // Read line-by-line from file until EOF
        while (getline(file, line))
            fileString.append(line + "\n"); // replace stripped newlines
        file.close();
    }

    else
        cout << "Error reading file";
    
    // While loop adds an unnecessary newline; remove that.
    fileString = fileString.substr(0, fileString.size() -1);
    
    return fileString;  
}

/**
 * Search this directory for filename
 *
 * @param fileName a file or directory name
 * 
 * @return if fileName exists in this directory
 */
bool fileExists(string fileName) {
    bool fileFound = false;

    // Will contain path. Memory allocated by getcwd()
    char *cwd = NULL;
    struct dirent *dirStream;
    string returnString = "";
    
    // Get cwd path. Dynamically allocated.
    cwd = getcwd(cwd, NULL);
    
    // Open the current directory  
    DIR *thisDir = opendir(cwd);
    if (!thisDir) {
        perror ("Opendir");
    }

    // Help obtained from the accepted answer at SO here:
    // stackoverflow.com/questions/3554120/open-directory-using-c
    
    // Compare the string representation of each object in thisDir
    // to fileName.
    while ((dirStream = readdir(thisDir)) != NULL) {
        if (fileName.compare(dirStream->d_name) == 0) {
            fileFound =  true;
            break;
        }
    }

    // Path buffer must be freed
    free(cwd);
    closedir(thisDir);   
    return fileFound;
}


/**
 * Return a listing of files in this directory
 *
 * @return string tag-delimited file and directory listing
 */
string lsCWD() {
    // Will hold path. Allocated by getcwd()
    char *cwd = NULL;
    struct dirent *dirStream;
    string returnString = "";
    
    //Get cwd path. Dynamically allocated.
    cwd = getcwd(cwd, NULL);  
    
    //Open the current directory
    DIR *thisDir = opendir(cwd);
    if (!thisDir) {
        perror ("Opendir");
    }
    
    // Help obtained from the accepted answer at SO here:
    // stackoverflow.com/questions/3554120/open-directory-using-c
    
    // Add the string representation of each object in thisDir
    // to returnString.
    while ((dirStream = readdir(thisDir)) != NULL){
        returnString.append("<item>");
        returnString.append(dirStream->d_name);
        returnString.append("</item>");
    }
     
    // Path buffer must be freed.
    free(cwd);
    closedir(thisDir);
    return returnString;  
}
