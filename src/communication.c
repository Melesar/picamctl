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

static pthread_mutex_t mutex;
static clientDescriptor* connectedClients;
static int clientIndex;

static void printClients();
static void respondToClient(char response, int socket);
static void* clientRoutine(void*);
static void disconnectClient(int clientSocket);
static void disposeClient(const clientDescriptor* client);

int initCommunication(communicationParams params)
{
	parameters = params;

	int memorySize = parameters.maxClients * CLIENT_SIZE;
	connectedClients = malloc(memorySize);
	memset(connectedClients, 0, memorySize);

	clientIndex = 0;

	return pthread_mutex_init(&mutex, NULL);
}

int connectClient(int clientSocket)
{
	pthread_mutex_lock(&mutex);

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

	int savedIndex = clientIndex;
	pthread_mutex_unlock(&mutex);

	return savedIndex;
}

int readClients()
{
	pthread_mutex_lock(&mutex);
	int index = clientIndex;
	pthread_mutex_unlock(&mutex);
	return index;
}

void disconnectAll()
{
	pthread_mutex_lock(&mutex);
	for(size_t i = 0; i < clientIndex; ++i)
	{
		disposeClient(&connectedClients[i]);
	}
	clientIndex = 0;
	memset(connectedClients, 0, parameters.maxClients * CLIENT_SIZE);
	printf("Disconnected all clients\n");
	printClients();
	pthread_mutex_unlock(&mutex);
}

void closeCommunication()
{
	disconnectAll();
	free(connectedClients);
}

void* clientRoutine(void* arg)
{
	int clientSocket = *(int*)arg;
	unsigned short buffer;
	while(1)
	{
		int receivedBytes = recv(clientSocket, &buffer, sizeof(buffer), 0);
		if (receivedBytes < 0)
		{
			printf("Failed to read data from the client\n");
			return 0;
		}

		if (receivedBytes == 0)
		{
			disconnectClient(clientSocket);
			return 0;
		}

		if (receivedBytes < sizeof(buffer)) 
		{
			continue;
		}

		if (buffer == CMD_DISCONNECT_ALL)
		{
			disconnectAll();
			return 0;
		}
	}
	return 0;
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
	send(socket, &response, sizeof(response), 0);
}

void disconnectClient(int clientSocket)
{
	pthread_mutex_lock(&mutex);
	int removedClientIndex = -1;
	for (size_t i = 0; i < clientIndex; ++i)
	{
		if (connectedClients[i].socket == clientSocket)
		{
			disposeClient(&connectedClients[i]);
			removedClientIndex = i;
			break;
		}
	}

	if (removedClientIndex < 0)
	{
		pthread_mutex_unlock(&mutex);
		return;
	}

	connectedClients[removedClientIndex] = connectedClients[clientIndex - 1];
	memset(connectedClients + clientIndex, 0, CLIENT_SIZE);
	clientIndex -= 1;
	printClients();
	
	pthread_mutex_unlock(&mutex);
}

void disposeClient(const clientDescriptor* client)
{
	close(client->socket);
	pthread_cancel(client->threadId);
	pthread_join(client->threadId, NULL);
}
