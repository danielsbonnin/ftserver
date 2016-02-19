#include <iostream>
#include "ftserver.h"
#include <string>
#include <stdexcept>
using namespace std;



int main(int argc, char** argv) {
	int portno = getPort(argc, argv);
	if (portno == -1)  //Invalid command line port input
		return 0;  //quit gracefully.

	
	cout << "You entered port number: " << portno << "\n";


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