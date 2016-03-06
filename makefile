# file: 		makefile
# author: 		Daniel Bonnin
# email:		bonnind@oregonstate.edu
# descr:		Builds ftserver for project 2
all: ftserver.hpp 
	g++ -std=c++0x -Wall -pedantic -o ftserver -g ftserver.cpp
