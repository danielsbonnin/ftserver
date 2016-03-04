#!/usr/bin/env python
#^
import argparse
import socket
import re

#Command identifiers. Go inside html style <\> tags to be sent to server. 
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
    args = cliHandler()

    #Create 2 sockets: control and data. 
    cntrl = socket.socket(
        socket.AF_INET, socket.SOCK_STREAM)
    dataSocket = socket.socket(
        socket.AF_INET, socket.SOCK_STREAM)

    #Verify Server address and contact server
    initContact(args, cntrl)
    commandMSG = parseCommand(args)
    sendCommand(cntrl, commandMSG)
    data = receiveData(dataSocket, args)
    print("Received from server: " + data)
    cntrl.close()
    dataSocket.close()
    parseServerData(data)

    
    
    exit(0)

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
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument(
        '-l',
        action='store_true',
        help='list files in current server directory')
    group.add_argument(
        '-g',
        metavar='FILENAME',
        type=str,
        help='retrieve <FILENAME> from server')
    
    return parser.parse_args()

def initContact(args, s):
    args.hostname = 'localhost'
    args.port = 4545
    #Establish connection with server
    #@param args the cli arguments of hostname and port
    #@param s the tcp socket object
    
    #This error handling code heavily influenced by the accepted answer here:
    #stackoverflow.com/questions/177389/testing-socket-connection-in-python
    try:
        s.connect((args.hostname, args.SERVER_PORT))
    except Exception as e:
        print(args.hostname + ':' +
              str(args.SERVER_PORT) +
              ' does not seem to be responding.')
        #print(e)
        s.close()
        exit(1)

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

def receiveData(dataSocket, args):
    dataSocket.bind(('localhost', int(args.DATA_PORT)))
    dataSocket.listen(1)
    data, serverAddr = dataSocket.accept()
    return data.recv(1024).decode('utf-8')

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

def printList(serverMessage):
    filenames = parseTag(ITEM_TAG, serverMessage)
    print("***Files in remote directory***")
    for i in filenames:
        print(i)

def saveFile(serverMessage):
    filename = parseTag(NAME_TAG, serverMessage)
    fileData = parseTag(DATA_TAG, serverMessage)
    print("data: ", fileData[0])
    newFile = open(filename[0], 'w+')
    newFile.write(fileData[0])
    newFile.close()
    
    print("contents of new file: " + filename[0] + ": ")
    newFile = open(filename[0], 'r')
    contents = newFile.read(50)
    print(contents)
    newFile.close()

def printError(serverMessage):
    errorMSG = parseTag(ERROR_TAG, serverMessage)
    print(errorMSG[0])

def parseCommand(args):
    if (args.l):
        command = '<' + LIST_COMMAND + '>  </' + LIST_COMMAND + '>'
    else:
        command = '<' + GET_COMMAND + '>' + args.g + '</' + GET_COMMAND + '>'
    return command + '<dataport>' + str(args.DATA_PORT) + '</dataport>'

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
