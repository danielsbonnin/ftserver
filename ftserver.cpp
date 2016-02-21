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
	waitForClient(to_string(portno).c_str());  //Call server function on port arg.

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
	SOCKET s = INVALID_SOCKET;  // The server socket
	SOCKET c = INVALID_SOCKET; // The client in the control connection.
	int recvRetVal, bindRetVal, listenRetVal, sendRetVal;
	struct addrinfo hints;
	struct addrinfo *servinfo = NULL;  // will point to the results
	struct sockaddr_storage c_addr;  // Address info for client control connection
	socklen_t addrlen = sizeof(struct sockaddr_storage);
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
		if ((c = accept(s, (struct sockaddr *)&c_addr, &addrlen)) == INVALID_SOCKET) {
			closesocket(s);
			WSACleanup();  //TODO: Windows-specific func to remove.
			cout << "Error on accept()\n";
			return 0;
		}


		//TODO: remove sample code
		//do-while loop is all sample code from MSDN
		// Receive until the peer shuts down the connection
		do {
			recvRetVal = recv(c, buffer, RECV_BUF_LEN, 0);
			if (recvRetVal > 0) {
				printf("Bytes received: %d\n", recvRetVal);
				string bufString(buffer);
				bufString = bufString.substr(0, recvRetVal);
				cout << "Received " << bufString << " from client\n";
				parseCommand(bufString);
				sendFile();
			}
			else if (recvRetVal == 0)
				printf("Connection closing...\n");
			else {
				printf("recv failed with error: %d\n", WSAGetLastError());
				closesocket(c); //TODO: change to "close()" for linux.
				WSACleanup();  //TODO: windows-specific to remove.
				return 1;
			}
		} while (recvRetVal > 0);
	}
	closesocket(s);

	WSACleanup(); // TODO: get rid of this function: Windows-specific.
	return 0;

}

int sendFile() {
	cout << "Entered sendFile()\n";
	return 1;
}

int parseCommand(string msg) {
	string message(msg);
	cout << "parseCommand() is parsing " << message << "\n";
	string command = message.substr(message.find_first_of('<') + 1, message.find_first_of('>') - 1);
	cout << "the parsed command is: " << command << "\n";
	return 1;
}
