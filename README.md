file:       README.txt
author:     Daniel Bonnin
username:   bonnind
email:      bonnind@oregonstate.edu
class:      CS372
project:    2

This file describes the running of ftserver and ftclient for project 2, 
CS372, Winter term, 2016, at Oregon State University.

There are 4 files contained in the archive file: 
    ftserver.cpp
    ftserver.hpp
    ftclient.py
    makefile
    README.md

In order to build the server, ensure that ftserver.cpp, ftserver.hpp, 
and makefile are in the same directory. From within the same directory,
type the following command.

    $make

In order to run the server, type the following command from the same directory.

    $ ./ftserver <server port number> 

In order to run the client, ensure that chatclient.py is in the 
working directory, and type the following command:
    
    $ ./ftclient.py <server host name> <server port number> \
     <data port number> [-l|-g filename] 

In case there are issues due to different environment variables or 
permissions from those in my testing environment, you may need to 
try the full command:
    
    $python ftclient.py <server host name> <server port number> \
    <data port number> [-l|-g filename]

Commandline variables:
    server host name:   URL or ip address of the server
    server port number: The open port on ftserver's host
    data port number:   The open port on ftclient's host

Options:
    ftclient requires a command option. The user must select 1 and only 1 of
    the following 2 options: "-l" or "-g". 

    The "-l" ("List") command returns a list of all the objects in the
    ftserver current directory.

    The "-g" ("Get File") command copies a file from ftserver's current 
    directory to ftclient's current directory. This command requires a 
    "filename" argument which is a relative path + filename from ftserver's
    current directory. 

    If the filename argument is not valid, ftserver returns an error message,
    which is printed to the terminal by ftclient.

Termination Conditions
    ftclient terminates automatically, after receiving a server response.  

    ftserver terminates via keyboard interrupt (CNTRL-C)

Testing environment:
    The testing environment I used were the flip1, flip2, and flip3 servers 
    at access.engr.oregonstate.edu.


