#include "communication.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define RES_SUCCESS 0
#define RES_FULL	1

static communicationParams parameters;

static int sok;
static uint32_t* connectedClients;
static int clientIndex;

static void printClients()
{
	for(int i = 0; i < parameters.maxClients; i++)
	{
		printf("%d ", connectedClients[i]);
	}
	printf("\n");
}


int initCommunication(communicationParams params)
{
	parameters = params;

	int memorySize = parameters.maxClients * sizeof(uint32_t);
	connectedClients = malloc(memorySize);
	memset(connectedClients, 0, memorySize);

	clientIndex = 0;

	sok = socket(AF_INET, SOCK_DGRAM, 0);
	return sok != -1 ? 0 : -1;
}

int connectClient(uint32_t address)
{
	if (clientIndex >= parameters.maxClients)
	{
		return parameters.maxClients;
	}

	for(int i = 0; i < parameters.maxClients; ++i)
	{
		if (connectedClients[i] == address)
		{
			return clientIndex;
		}
	}

	connectedClients[clientIndex++] = address;
	printf("Connected client %d\n", address);
	printClients();

	return clientIndex;
}

int disconnectClient(uint32_t address)
{
	for(int i = 0; i < parameters.maxClients; i++)
	{
		if (connectedClients[i] != address)
		{
			continue;
		}

		int indexSrc = i + 1;
		int indexDst = i;
		int numBytes = (parameters.maxClients - indexSrc) * sizeof(uint32_t);
		memcpy(connectedClients + indexDst, connectedClients + indexSrc, numBytes);
		memset(connectedClients + clientIndex, 0, (parameters.maxClients - clientIndex) * sizeof(uint32_t));
		printf("Disconnected client %d\n", address);
		printClients();

		return --clientIndex;
	}

	return clientIndex;
}

void disconnectAll()
{
	clientIndex = 0;
	memset(connectedClients, 0, parameters.maxClients * sizeof(uint32_t));
}

void closeCommunication()
{
	free(connectedClients);
	close(sok);
}
