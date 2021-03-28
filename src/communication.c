#include "communication.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#define BUFLEN		sizeof(short)
#define CLIENT_SIZE sizeof(clientDescriptor)

#define RES_SUCCESS			  0
#define RES_FULL			  1
#define RES_CONNECTED_ALREADY 2

#define CMD_CONNECT			0xDCAC
#define CMD_DISCONNECT		0xACDC
#define CMD_DISCONNECT_ALL	0xAAAA

typedef struct clientDescriptor
{
	pthread_t threadId;
	int socket;
} clientDescriptor;

static communicationParams parameters;

static clientDescriptor* connectedClients;
static int clientIndex;

static void printClients();
static void respondToClient(char response, int socket);
static void* clientRoutine(void*);

int initCommunication(communicationParams params)
{
	parameters = params;

	int memorySize = parameters.maxClients * CLIENT_SIZE;
	connectedClients = malloc(memorySize);
	memset(connectedClients, 0, memorySize);

	clientIndex = 0;

	return 0;
}

int connectClient(int clientSocket)
{
	//TODO add synchronization
	if (clientIndex >= parameters.maxClients)
	{
		respondToClient(RES_FULL, clientSocket);
		close(clientSocket);
		return clientIndex;
	}

	for (size_t i = 0; i < clientIndex; ++i)
	{
		//Dunno if it actually can happen, but let's check
		if (connectedClients[i].socket == clientSocket)
		{
			respondToClient(RES_CONNECTED_ALREADY, clientSocket);
			return clientIndex;
		}
	}

	clientDescriptor* client = &connectedClients[clientIndex++];
	client->socket = clientSocket;
	respondToClient(RES_SUCCESS, clientSocket);
	pthread_create(&client->threadId, NULL, &clientRoutine, (void*)&client->socket);
	return clientIndex;
}

int readClients()
{
	//TODO add synchronization
	return clientIndex;
}

void disconnectAll()
{
	//TODO add synchronization
	//TODO close all sockets and terminate all threads
	clientIndex = 0;
	memset(connectedClients, 0, parameters.maxClients * CLIENT_SIZE);
	printf("Disconnected all clients\n");
	printClients();
}

void closeCommunication()
{
	free(connectedClients);
}

void* clientRoutine(void* arg)
{
	int clientSocket = *(int*)arg;
	short buffer;
	while(1)
	{
		int receivedBytes = recv(clientSocket, &buffer, sizeof(buffer), 0);
		if (receivedBytes < sizeof(buffer)) 
		{
			continue;
		}

		switch(buffer)
		{
			//TODO figure out how to handle this
			case CMD_DISCONNECT:;
				break;
			case CMD_DISCONNECT_ALL:;
				disconnectAll();
				break;

		}
	}
	return NULL;
}

void printClients()
{
	for(int i = 0; i < parameters.maxClients; i++)
	{
		printf("%d ", connectedClients[i].socket);
	}
	printf("\n");
}

void respondToClient(char response, int socket)
{
	//TODO implement
}
