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
int getPort(int argc, char** argv);
#endif
