#!/usr/bin/env python
#^
import argparse
import socket

def main():
    args = cliHandler()

    #Create a tcp socket. 
    s = socket.socket(
        socket.AF_INET, socket.SOCK_STREAM)

    #Verify Server address and contact server
    initContact(args, s)
    sendCommand(s)
    s.close
    print("socket is closed()")
    input() #TODO: fix raw_input/input issue
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
        s.connect((args.hostname, args.port))
    except Exception as e:
        print(args.hostname + ':' +
              str(args.port) +
              ' does not seem to be responding.')
        #print(e)
        s.close()
        exit(1)

def sendCommand(s):
    #@param s the socket object
    
    try:  #format and send command
        s.sendall(("Test client-server").encode('utf-8'))
    except socket.error as e:
        #close program if error sending
        if (e.errno == errno.ECONNRESET or e.errno == errno.ECONNABORTED):
            print("server disconnected")
            s.close()
            exit(1)
        else:
            s.close()
            exit(1)

if __name__ == '__main__':
    main()
