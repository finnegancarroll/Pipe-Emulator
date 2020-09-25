#Finnegan Carroll
#Makefile

CC = g++
DEBUG = -g
CFLAGS = -Wall $(DEBUG)
MyShell : 
	$(CC) $(CFLAGS) PipeEmulator.cpp
clean:
	\rm *.o *~ 
