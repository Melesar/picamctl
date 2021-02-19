#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>

typedef struct
{
	int maxClients;
} communicationParams;


// Returns 0 if success, -1 otherwise
int initCommunication(communicationParams params);

//Returns the number of currently connected clients
int connectClient(struct sockaddr_in address); 

//Returns the number of currently connected clients
int disconnectClient(struct sockaddr_in address);

void disconnectAll();

void closeCommunication();

#endif
