#include <iostream>
#include <fstream>
#include <limits.h>
#include <dirent.h>
#include "ftserver.h"
#include <string>
#include <cstring>
#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <netinet/in.h> //For inet_ntoa()
#include <arpa/inet.h>
#include <errno.h>
#include <csignal>
using namespace std;

bool notKilled = true;

static void signalHandler(int signum) {
    cout << "Keyboard interrupt received\n";
    notKilled = false;
    exit (signum);
}
int main(int argc, char** argv) {
    signal(SIGINT, signalHandler);   
    int portno = getPort(argc, argv);
	
	if (portno == -1)  //Invalid command line port input
		return 0;  //quit gracefully.

	
	cout << "You entered port number: " << portno << "\n";
    
    //Call server function on port arg.
	waitForClient(to_string((long long)portno).c_str(), &notKilled);

	return 0;
}

int getPort(int argc, char **argv) {
	int portno = 0;

	if (argc != 2) { //Invalid num args.
		cout << "Invalid Command Line Arguments\n" << USAGE;
		return -1;
	}

	/* Help for exception handling: 
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
	if (portno < PORT_MIN || portno > PORT_MAX) {
		cout << "Port Number argument out of range\n" << USAGE;
		return -1;
	}
	else
		return portno;
}

/*
 * Much help obtained from Beej's Guide: Beej.us/quide
 * @param portno port number to listen on.
 * @pre portno is valid.
 */
int waitForClient(const char *portno, bool *notKilled) {
	
	// The following is copy-paste from Beej's guide. Comments are mine (some)
	int status;
    int  s = 0;  // The server socket
	int c = 0; // The client in the control connection.
	int recvRetVal;
    socklen_t addrlen = sizeof(struct sockaddr_storage);
	struct addrinfo hints;
	struct addrinfo *servinfo;  // will point to the results
	struct addrinfo *next;  // Next in linked list of ip addresses
    struct sockaddr_storage c_addr;  // Address info for client control connection
	char buffer[RECV_BUF_LEN];

	memset(&hints, 0, sizeof hints); // Zero-out hints
	hints.ai_family = AF_INET;  // ipv4 socket
	hints.ai_socktype = SOCK_STREAM; // TCP, not UDP
	hints.ai_flags = AI_PASSIVE; // fill in my ip for me

	if ((status = getaddrinfo(NULL, portno, &hints, &servinfo)) != 0) {
		perror("Error with getaddrinfo");
		exit(1);
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
	freeaddrinfo(servinfo);

	
	if ((listen(s, MAX_INCOMING_CONNECTIONS)) == -1) {
	    perror("Listen");
        exit(1);
    }
	
	//Accept clients until interrupt is received. 
	while (*notKilled) {
		if ((c = accept(s, (struct sockaddr *)&c_addr, &addrlen)) == -1) {	
			perror("Accept");
            close(s);
			return 0;
		}

		//TODO: remove sample code
		//do-while loop is all sample code from MSDN
		// Receive until the peer shuts down the connection
		do {
			int dataPortNo = 0;
			recvRetVal = recv(c, buffer, RECV_BUF_LEN, 0);
			if (recvRetVal > 0) {
				printf("Bytes received: %d\n", recvRetVal);
				string bufString(buffer);
				
                bufString = bufString.substr(0, recvRetVal);
				cout << "Received " << bufString << " from client\n";
                string response = parseCommand(bufString, &dataPortNo);			
                sleep(1);  //Allow client time to open a socket.
                sendResponse(
                        response, 
                        (struct sockaddr*)&c_addr, 
                        (socklen_t*) &addrlen, 
                        dataPortNo);
			}
			else if (recvRetVal == 0) {
				printf("Connection closing...\n");
                close(c);
		    }	
            else {
			    perror("Recv");
                close(c); 
			}
		} while (recvRetVal > 0);
	}
    // Close server and client sockets.
    close(c);
	close(s);

	return 0;
}

int sendResponse(
        string response, 
        struct sockaddr* clientAddr, 
        socklen_t* addrlen, int portNo){
    int connectRetVal = 0;
    int sendRetVal = 0;    
   
    //Create data structures for connection
	int dataSocket = 0;
	
	// The following code is borrowed heavily from Beej's guide because I am 
	// learning from it. Note that I am commenting heavily, indicating that
	// I understand the purpose of each line. 
	// Source: https://beej.us/guide/bgnet/output/html/multipage/(continued)
    // (continuation)syscalls.html#getpeername
	struct addrinfo hints;
    
    //The client program is the "server" for this data connection.
	struct addrinfo *serverInfo; 
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	// Ip address parsing help obtained from accepted answer here:
	// http://stackoverflow.com/questions/1276294/(continued)
    // (continuation)getting-ipv4-address-from-a-sockaddr-structure
	// Cast struct sockaddr clientAddr as struct sockaddr_in
    // get sin_addr member(ipv4 addr).
	// convert byte order.
	char *ip = inet_ntoa(((sockaddr_in*)clientAddr)->sin_addr);
	string portNumber = to_string((long long)portNo);
	if ((getaddrinfo(ip, portNumber.c_str(), &hints, &serverInfo))== -1) {
        perror("getaddrinfo");
    }
	dataSocket = socket(
            serverInfo->ai_family, 
            serverInfo->ai_socktype, 
            serverInfo->ai_protocol);
	connectRetVal = connect(dataSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
    if (connectRetVal != 0) {
        perror("Connect");
    }    
    const char *charResponse = response.c_str();
	sendRetVal = send(dataSocket, charResponse, response.length(), MSG_NOSIGNAL);
    if (sendRetVal == -1) {
        perror("Send"); 
    } 
	close(dataSocket);
	return 1;
}
string parseTag(string tag, string msg) {
    int dataLength;
    const string openTag("<" + tag + ">");
    const string closeTag("</" + tag + ">");
    dataLength = msg.find(closeTag) - msg.find(openTag) - openTag.length();
    if ( dataLength > 0)
        return msg.substr((msg.find(openTag) + openTag.length()), dataLength);
    else 
        return "";
}
std::string parseCommand(string msg, int *portNo) {
	string message(msg);
    string command;
    string port;
	string returnMSG = "";

	//Command is 1 letter long.
	command = message.substr(message.find_first_of('<') + 1, 1);
    
    port = parseTag("dataport", message);
	cout << "parseTag port result: " << port<< endl;
    *portNo = stoi(port);

	
	//Check for "Get" command
	if (command.compare(GET_COMMAND) == 0) {
		//Look for filename
		int lenFilename = (int) (message.find("</g>") - message.find("<g>") - 3);
		string filename = message.substr(message.find("<g>") + 3, lenFilename);
		cout << "File " << filename << "requested on port " << port << "\n";
        if(fileExists(filename)) {
            returnMSG = 
                "<ok><name>"            + 
                filename                + 
                "</name><data>"         + 
                fileAsString(filename)  +   
                "</data></ok>";
            cout << "Sending " << filename << " to client.\n"; 
        }
        else {
            returnMSG = "<error>There is no such file</error>";
            cout << "The requested file does not exist, sending error msg\n";
        }
	}
	else if (command.compare(LIST_COMMAND) == 0) {
        cout << "List directory requested on port " << port << " \n";    
		//List files in current directory into a tag-delimited string
		string fileNames = lsCWD();
		//encapsulate files into a message
		returnMSG = "<ok><list>" + fileNames + " </list></ok>";
	}
	else {
		//encapsulate error message into a message
		returnMSG = "<error>Command " + command + " not recognized</error>";
	}
	return returnMSG;
}

string fileAsString(string filename) {
    // File stream refresher provided by:
    // www.cplusplus.com/doc/tutorial/files
    string line, fileString = "";
    ifstream file(filename);
    if (file.is_open()) {
        // Below while loop obtained from source (above)
        while (getline(file, line))
            fileString.append(line + "\n");
        file.close();
    }
    else
        cout << "Error reading file";
    return fileString;
}
bool fileExists(string fileName) {
    bool fileFound = false;
    char *cwd = NULL;
    struct dirent *dirStream;
    string returnString = "";
    //Get cwd path. Dynamically allocated.
    cwd = getcwd(cwd, NULL);
    DIR *thisDir = opendir(cwd);
    if (!thisDir) {
        perror ("Opendir");
    }
    while ((dirStream = readdir(thisDir)) != NULL) {
        if (fileName.compare(dirStream->d_name) == 0) {
            fileFound =  true;
            break;
        }
    }
    free(cwd);
    closedir(thisDir);   
    return fileFound;

}
string lsCWD() {
    char *cwd = NULL;
    struct dirent *dirStream;
    string returnString = "";
    //Get cwd path. Dynamically allocated.
    cwd = getcwd(cwd, NULL);  
    DIR *thisDir = opendir(cwd);
    if (!thisDir) {
        perror ("Opendir");
    }
    
    //Help obtained from the accepted answer at SO here:
    //stackoverflow.com/questions/3554120/open-directory-using-c
    while ((dirStream = readdir(thisDir)) != NULL){
        returnString.append("<item>");
        returnString.append(dirStream->d_name);
        returnString.append("</item>");
    }
     
    free(cwd);
    closedir(thisDir);
    return returnString;  
}
