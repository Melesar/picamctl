#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdint.h>

typedef struct
{
	int maxClients;
} communicationParams;


// Returns 0 if success, -1 otherwise
int initCommunication(communicationParams params);

//Returns the number of currently connected clients
int connectClient(uint32_t address); 

//Returns the number of currently connected clients
int disconnectClient(uint32_t address);

void disconnectAll();

void closeCommunication();

#endif
