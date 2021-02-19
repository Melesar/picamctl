#include "communication.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define RES_SUCCESS			  0
#define RES_FULL			  1
#define RES_CONNECTED_ALREADY 2

static communicationParams parameters;

static int sok;
static uint32_t* connectedClients;
static int clientIndex;

static void printClients();
static void respondToClient(char response, struct sockaddr_in address);

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

int connectClient(struct sockaddr_in address)
{
	if (clientIndex >= parameters.maxClients)
	{
		respondToClient(RES_FULL, address);
		return parameters.maxClients;
	}

	uint32_t addressValue = address.sin_addr.s_addr;
	for(int i = 0; i < parameters.maxClients; ++i)
	{
		if (connectedClients[i] == addressValue)
		{
			respondToClient(RES_CONNECTED_ALREADY, address);
			return clientIndex;
		}
	}

	connectedClients[clientIndex++] = addressValue;
	respondToClient(RES_SUCCESS, address);
	printf("Connected client %d\n", addressValue);
	printClients();

	return clientIndex;
}

int disconnectClient(struct sockaddr_in address)
{
	uint32_t addressValue = address.sin_addr.s_addr;
	for(int i = 0; i < parameters.maxClients; i++)
	{
		if (connectedClients[i] != addressValue)
		{
			continue;
		}

		int indexSrc = i + 1;
		int indexDst = i;
		int numBytes = (parameters.maxClients - indexSrc) * sizeof(uint32_t);
		memcpy(connectedClients + indexDst, connectedClients + indexSrc, numBytes);
		memset(connectedClients + clientIndex, 0, (parameters.maxClients - clientIndex) * sizeof(uint32_t));
		printf("Disconnected client %d\n", addressValue);
		printClients();

		return --clientIndex;
	}

	return clientIndex;
}

void disconnectAll()
{
	clientIndex = 0;
	memset(connectedClients, 0, parameters.maxClients * sizeof(uint32_t));
	printf("Disconnected all clients\n");
	printClients();
}

void closeCommunication()
{
	free(connectedClients);
	close(sok);
}

void printClients()
{
	for(int i = 0; i < parameters.maxClients; i++)
	{
		printf("%d ", connectedClients[i]);
	}
	printf("\n");
}

void respondToClient(char response, struct sockaddr_in address)
{
	sendto(sok, &response, sizeof(response), 0, (struct sockaddr*)&address, sizeof(address));
}
