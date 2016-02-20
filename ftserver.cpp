#include <iostream>
#include "ftserver.h"
#include <string>
#include <stdexcept>
#include <sys/types.h>
#include <WinSock2.h>
#include <ws2tcpip.h>  // winsock library. 

#pragma comment(lib, "Ws2_32.lib") //Link to winsock library file.
using namespace std;



int main(int argc, char** argv) {
	int portno = getPort(argc, argv);
	if (portno == -1)  //Invalid command line port input
		return 0;  //quit gracefully.

	
	cout << "You entered port number: " << portno << "\n";
	waitForClient(to_string(portno).c_str());

	getchar();
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
int waitForClient(const char  *portno) {
	// The following is copy-paste from Beej's guide. Comments are mine (some)
	//WSA startup function "initiates the use of the Winsock DLL" -MSDN

	WSADATA wsaData;   // if this doesn't work
						//WSAData wsaData; // then try this instead

						// MAKEWORD(1,1) for Winsock 1.1, MAKEWORD(2,0) for Winsock 2.0:

	if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
		fprintf(stderr, "WSAStartup failed.\n");
		exit(1);
	}

	int status;
	int s;  // The server socket
	int c; // The client in the control connection.
	struct addrinfo hints;
	struct addrinfo *servinfo;  // will point to the results
	struct sockaddr_storage c_addr;  // Address info for client control connection
	socklen_t addrlen = sizeof(struct sockaddr_storage);

	memset(&hints, 0, sizeof hints); // Zero-out hints
	hints.ai_family = AF_INET;  // ipv4 socket
	hints.ai_socktype = SOCK_STREAM; // TCP, not UDP
	hints.ai_flags = AI_PASSIVE; // fill in my ip for me

	if ((status = getaddrinfo(NULL, portno, &hints, &servinfo)) != 0) {
		// TODO: handle errno with getaddrinfo
		cout << "Error with getaddrinfo\n";
		/*fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));*/
		exit(1);
	}

	// servinfo now points to a linked list of 1 or more struct addrinfos
	//TODO: walk the linked list of struct addrinfos in order to find a valid one. ?
	// Init the server socket.
	s = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

	// Bind socket to port (specified in struct addrinfo hints)
	bind(s, servinfo->ai_addr, servinfo->ai_addrlen);

		// TODO: handle errno on listen().
	if (listen(s, MAX_INCOMING_CONNECTIONS) == -1) {
		cout << "Error on listen()\n";
	}
	else {
		cout << "Incoming connection. . .";
		// TODO: handle errno on accept()
		if (c = accept(s, (struct sockaddr *)&c_addr, &addrlen) != -1) {
			cout << "Error on accept()\n";
		}
		else {
			cout << "Received a connection from a client";
			closesocket(c);  // This needs to be "close()" for linux.
		}
	}


	// ... do everything until you don't need servinfo anymore ....


	freeaddrinfo(servinfo); // free the linked-list
	WSACleanup(); // Windows-specific.

}
