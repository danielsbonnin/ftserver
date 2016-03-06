#!/usr/bin/env python
#^
"""ftclient.py: A simple file transfer client for class

This module connects to the ftserver and reads contents of server directory
or transfers a text file.

ftclient.py uses 2 tcp connections: 1 to connect to server and send a command, 
the other, to accept the server data connection on a specified port.

Example:
    $python ftclient.py serverHost <server port> <client port> -g <filename>

"""
import argparse
import socket
import re

#Command identifiers. Go inside html style <\> tags to be sent to server. 
MAX_RECV = 8096
GET_COMMAND = "g"
LIST_COMMAND = "l"
OK_TAG = "ok"
FILE_TAG = "file"
ERROR_TAG = "error"
LIST_TAG = "list"
ITEM_TAG = "item"
NAME_TAG = "name"
DATA_TAG = "data"

def main():
    # get valid command line input
    args = cliHandler()

    # Create 2 sockets: control and data. 
    cntrl = socket.socket(
        socket.AF_INET, socket.SOCK_STREAM)
    dataSocket = socket.socket(
        socket.AF_INET, socket.SOCK_STREAM)

    # Verify Server address and contact server
    initContact(args, cntrl)

    # Create formatted command message
    commandMSG = parseCommand(args)

    # Open TCP client socket conn with server
    sendCommand(cntrl, commandMSG)

    # Open TCP server socket on specified port.
    # Wait for server data
    data = receiveData(dataSocket, args)
    
    # Close sockets
    cntrl.close()
    dataSocket.close()
    
    # Save files and/or print data to terminal
    parseServerData(data)

    exit(0)

# Use the argparse library to process commandline arguments
def cliHandler():
    #Use argparse library to process cli
    
    #Help obtained from python docs at
    #docs.python.org/3/library/argparse.html
    parser = argparse.ArgumentParser(description='User\'s command line options')
    parser.add_argument(
        'SERVER_HOST',
        type=str,
        help='The server\'s hostname')
    parser.add_argument(
        'SERVER_PORT',
        type=int,
        help='The server\'s port number')
    parser.add_argument('DATA_PORT', type=str, help='The data port number')
    
    # User must enter 1 and only 1 command option
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument(
        '-l',
        action='store_true',
        help='list files in current server directory')
    
    # The "Get File" command option must be followed by a filename argument 
    group.add_argument(
        '-g',
        metavar='FILENAME',
        type=str,
        help='retrieve <FILENAME> from server')
    
    return parser.parse_args()

# Connect to server on specified SERVER_HOST and SERVER_PORT
# @param args the cli arguments of hostname and port
# @param s the tcp socket object
def initContact(args, s):
    #Establish connection with server
    
    #This error handling code heavily influenced by the accepted answer here:
    #stackoverflow.com/questions/177389/testing-socket-connection-in-python
    try:
        s.connect((args.SERVER_HOST, args.SERVER_PORT))
    except Exception as e:
        print(args.SERVER_HOST + ':' +
              str(args.SERVER_PORT) +
              ' does not seem to be responding.')
        #print(e)
        s.close()
        exit(1)

# Send formatted message and close client socket
def sendCommand(s, commandMSG):
    #@param s the socket object
    try:  #format and send command
        s.sendall((commandMSG).encode('utf-8'))
    except socket.error as e:
        #close program if error sending
        if (e.errno == errno.ECONNRESET or e.errno == errno.ECONNABORTED):
            print("server disconnected")
            s.close()
            exit(1)
        else:
            s.close()
            print("Socket Exception")
            exit(1)

# wait on server socket for ftserver to connect and send response
def receiveData(dataSocket, args):

    returnString = ""
    # Get formatted host name. 
    # source: stackoverflow.com/questions/161030786/why-am-i-getting-the
    # error-connection-refused-in-python-sockets
    host = socket.gethostname()
    dataSocket.bind((host, int(args.DATA_PORT)))
    dataSocket.listen(1)
    print"listening on port ", args.DATA_PORT
    data, serverAddr = dataSocket.accept()
    while True:
        received = data.recv(MAX_RECV)
        if received == "":
            break
        returnString += received
        # empty block
    return returnString

# Process server data
def parseServerData(serverMessage):
    if OK_TAG in serverMessage:
        if LIST_TAG in serverMessage:
            printList(serverMessage)
        elif NAME_TAG in serverMessage:
            saveFile(serverMessage)
    else:
        if ERROR_TAG in serverMessage:
            printError(serverMessage);
        else:
            print("Server Message is in an unrecognized format" + serverMessage)  

# Print directory contents to terminal
def printList(serverMessage):
    filenames = parseTag(ITEM_TAG, serverMessage)
    print("***Files in remote directory***")
    for i in filenames:
        print(i)

# Save requested file in local directory
def saveFile(serverMessage):
    filename = parseTag(NAME_TAG, serverMessage)
    fileData = parseTag(DATA_TAG, serverMessage)
    if not fileData:
        return

    newFile = open(filename[0], 'w+')
    newFile.write(fileData[0])
    newFile.close()
    


def printError(serverMessage):
    errorMSG = parseTag(ERROR_TAG, serverMessage)
    print(errorMSG[0])

# Create formatted command message to send to ftserver
def parseCommand(args):
    if (args.l):
        command = '<' + LIST_COMMAND + '>  </' + LIST_COMMAND + '>'
    else:
        command = '<' + GET_COMMAND + '>' + args.g + '</' + GET_COMMAND + '>'
    return command + '<dataport>' + str(args.DATA_PORT) + '</dataport>'

# Return contents of specified tag label
# @param firstTagOnly whether to ignore closing tag
def parseTag(tag, msg):
    openTag = '<' + tag + '>'
    closeTag = '</' + tag + '>'
    if openTag not in msg or closeTag not in msg:
        print("***TAG NOT FOUND***")
        return None
    else:
        return re.findall(openTag + '(.*?)' + closeTag, msg, re.DOTALL)
    
if (__name__ == '__main__'):
    main()
