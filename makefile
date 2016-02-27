all: ftserver.h 
	g++ -std=c++0x -Wall -pedantic -o ftserver -g ftserver.cpp
