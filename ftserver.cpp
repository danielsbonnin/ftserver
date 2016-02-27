#include <iostream>
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
using namespace std;



int main(int argc, char** argv) {
	int portno = getPort(argc, argv);
	
	if (portno == -1)  //Invalid command line port input
		return 0;  //quit gracefully.

	
	cout << "You entered port number: " << portno << "\n";
	waitForClient(to_string((long long)portno).c_str());  //Call server function on port arg.

	getchar();  // TODO: Windows so the cmd window stays open long enough to read.
	return 0;
}

int getPort(int argc, char **argv) {
	int portno = 0;

	if (argc != 2) { //Invalid num args.
		cout << "Invalid Command Line Arguments\n" << USAGE;
		// TODO: remove this for linux.
		getchar();
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
		getchar(); //TODO: remove this for linux.
		return -1;
	}
	if (portno < PORT_MIN || portno > PORT_MAX) {
		cout << "Port Number argument out of range\n" << USAGE;
		getchar();  //TODO: remove this for linux.
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
int waitForClient(const char *portno) {
	
	// The following is copy-paste from Beej's guide. Comments are mine (some)
	int status;
    int  s = 0;  // The server socket
	int c = 0; // The client in the control connection.
	int recvRetVal, bindRetVal, listenRetVal;//TODO: figure out return values., sendRetVal;
    socklen_t addrlen = sizeof(struct sockaddr_storage);
	struct addrinfo hints;
	struct addrinfo *servinfo = NULL;  // will point to the results
	struct sockaddr_storage c_addr;  // Address info for client control connection
	char buffer[RECV_BUF_LEN];

	memset(&hints, 0, sizeof hints); // Zero-out hints
	hints.ai_family = AF_INET;  // ipv4 socket
	hints.ai_socktype = SOCK_STREAM; // TCP, not UDP
	hints.ai_flags = AI_PASSIVE; // fill in my ip for me

	if ((status = getaddrinfo(NULL, portno, &hints, &servinfo)) != 0) {
		// TODO: handle errno with getaddrinfo
		cout << "Error with getaddrinfo\n";
		exit(1);
	}

	// servinfo now points to a linked list of 1 or more struct addrinfos
	//TODO: walk the linked list of struct addrinfos in order to find a valid one. ?
	// Init the server socket.
	s = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

	// Bind socket to port (specified in struct addrinfo hints)
	bindRetVal = bind(s, servinfo->ai_addr, servinfo->ai_addrlen);
	freeaddrinfo(servinfo);

	// TODO: handle errno on listen().
	if ((listenRetVal = listen(s, MAX_INCOMING_CONNECTIONS)) == -1) {
		cout << "Error on listen()\n";
	}
	
	//Accept clients until interrupt is received. TODO: gracefully accept a shutdown command or signal
	while (true) {
		// TODO: handle errno on accept()
		if ((c = accept(s, (struct sockaddr *)&c_addr, &addrlen)) == -1) {
			close(s);
			cout << "Error on accept()\n";
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
                sendResponse(
                        response, 
                        (struct sockaddr*)&c_addr, 
                        (socklen_t*) &addrlen, 
                        dataPortNo);

			}
			else if (recvRetVal == 0)
				printf("Connection closing...\n");
			else {
				//TODO: linux error code. printf("recv failed with error: %d\n", WSAGetLastError());
				close(c); 
				return 1;
			}
		} while (recvRetVal > 0);
	}
	close(s);

	return 0;

}

int sendResponse(string response, struct sockaddr* clientAddr, socklen_t* addrlen, int portNo){
	//Create data structures for connection
	int dataSocket = 0;
	
	// The following code is borrowed heavily from Beej's guide because I am 
	// learning from it. Note that I am commenting heavily, indicating that
	// I understand the purpose of each line. 
	// Source: https://beej.us/guide/bgnet/output/html/multipage/syscalls.html#getpeername
	struct addrinfo hints;
	struct addrinfo *serverInfo; //The client program is the "server" for this data connection.
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	// Ip address parsing help obtained from accepted answer here:
	// http://stackoverflow.com/questions/1276294/getting-ipv4-address-from-a-sockaddr-structure
	// Cast struct sockaddr clientAddr as struct sockaddr_in and get sin_addr member(ipv addr).
	// Then, convert byte order.
	char *ip = inet_ntoa(((sockaddr_in*)clientAddr)->sin_addr);
	string portNumber = to_string((long long)portNo);
	//memset(&portNumber, 0, sizeof(char) * 6);
	//itoa(portNo, portNumber, 10);
	int status = getaddrinfo(ip, portNumber.c_str(), &hints, &serverInfo);

	dataSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	connect(dataSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	const char *charResponse = response.c_str();
	send(dataSocket, charResponse, response.length(), 0);
	close(dataSocket);
	cout << "client ip: " << ip << "\n";


	//call connect on specific addr/port
	//send response
	//close connection
	cout << "Entered sendFile() with data port number: " << portNo << "\n";
	return 1;
}

std::string parseCommand(string msg, int *portNo) {
	string message(msg);
	string returnMSG = "";
	cout << "parseCommand() is parsing " << message << "\n";

	//Command is 1 letter long.
	//TODO: redo all the message processing in a function.
	string command = message.substr(message.find_first_of('<') + 1, 1);
	string portTag("<dataport>");
	string portTagEnd("</dataport>");
	int lenPort = message.find(portTagEnd) - message.find(portTag) - portTag.length();
	string port = message.substr(message.find(portTag) + portTag.length(), lenPort);
	cout << "the port number sent by the client is: " << port <<  "\n";
	*portNo = stoi(port);

	
	//Check for "Get" command
	if (command.compare(GET_COMMAND) == 0) {
		//Look for filename
		int lenFilename = (int) (message.find("</g>") - message.find("<g>") - 3);
		string filename = message.substr(message.find("<g>") + 3, lenFilename);
		cout << "The filename received was " << filename << "\n";
		//TODO: validate filename
		//encapsulate file in message
		returnMSG = "<ok><name>VALID-NAME</name><data>VALID-DATA and bunch of lines and junk</data></ok>";
	}
	else if (command.compare(LIST_COMMAND) == 0) {
		//List files in current directory into a string
		string fileNames = "";
		//encapsulate files into a message
		returnMSG = "<ok><list><item>FIRST-NAME</item><item>SECOND-NAME</item></list></ok>";
	}
	else {
		//encapsulate error message into a message
		returnMSG = "<error>Command " + command + " not recognized</error>";
	}
	//send back message
	cout << returnMSG;
	return returnMSG;
}
