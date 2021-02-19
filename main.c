#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#ifdef PI
	#define CAMERA_LAUNCH execlp("motion", "motion")
#else
	#define CAMERA_LAUNCH 0
#endif

#define PORT		8085
#define BUFLEN		sizeof(short)
#define MAX_CLIENTS 5

#define CMD_CONNECT			0xDCAC
#define CMD_DISCONNECT		0xACDC
#define CMD_DISCONNECT_ALL	0xAAAA

pid_t launchCamera();
void disableCamera(pid_t pid);

int connectClient(uint32_t* clients, uint32_t index, uint32_t address);
int disconnectClient(uint32_t* clients, uint32_t index, uint32_t address);
void printClients(const uint32_t* clients);

int main (int argc, char** argv)
{
	int sok = socket(AF_INET, SOCK_DGRAM, 0);
	if (sok < 0)
	{
		printf("Failed to create a socket\n");
		return 1;
	}

	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT);
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sok, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0)
	{
		printf("Failed to bind socket\n");
		return 1;
	}

	struct sockaddr_in clientAddress;
	socklen_t addressLength = sizeof(clientAddress);

	uint32_t clientIndex = 0;
	uint32_t connectedClients[MAX_CLIENTS];
	memset(connectedClients, 0, sizeof(connectedClients));
	
	unsigned char buffer[BUFLEN];
	memset(buffer, 0, BUFLEN);

	pid_t cameraPid = 0;
	while (1)
	{
		int receivedBytes = recvfrom(sok, buffer, BUFLEN, 0, (struct sockaddr*)&clientAddress, &addressLength);
		if (receivedBytes < 0)
		{
			printf("Failed to call receive on socket\n");
			return 1;
		}

		unsigned short receivedValue = *((unsigned short*)buffer);
		printf("Received %d\n", receivedValue);
		int newIndex;
		switch (receivedValue)
		{
			case CMD_CONNECT:;
				newIndex = connectClient(connectedClients, clientIndex, clientAddress.sin_addr.s_addr);
				if (newIndex == 1 && cameraPid == 0)
				{
					cameraPid = launchCamera();
				}
				clientIndex = newIndex;
				break;
			case CMD_DISCONNECT:;
				newIndex = disconnectClient(connectedClients, clientIndex, clientAddress.sin_addr.s_addr);
				if (newIndex == 0 && cameraPid != 0)
				{
					disableCamera(cameraPid);
					cameraPid = 0;
				}
				clientIndex = newIndex;
				break;
			case CMD_DISCONNECT_ALL:
				memset(connectedClients, 0, MAX_CLIENTS * sizeof(uint32_t));
				disableCamera(cameraPid);
				clientIndex = 0;
				cameraPid = 0;
				printf("Disconnected all clients\n");
				printClients(connectedClients);
				break;
		}
	}


	close(sok);
	return 0;
}


int connectClient(uint32_t* clients, uint32_t index, uint32_t address)
{
	if (index >= MAX_CLIENTS)
	{
		return MAX_CLIENTS;
	}

	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (clients[i] == address)
		{
			return index;
		}
	}

	clients[index++] = address;
	printf("Connected client %d\n", address);
	printClients(clients);
	return index;
}


int disconnectClient(uint32_t* clients, uint32_t index, uint32_t address)
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if (clients[i] != address)
		{
			continue;
		}

		int indexSrc = i + 1;
		int indexDst = i;
		int numBytes = (MAX_CLIENTS - indexSrc) * sizeof(uint32_t);
		memcpy(clients + indexDst, clients + indexSrc, numBytes);
		memset(clients + index, 0, (MAX_CLIENTS - index) * sizeof(uint32_t));
		printf("Disconnected client %d\n", address);
		printClients(clients);
		return index - 1;
	}

	return index;
}

pid_t launchCamera()
{
	pid_t pid = fork();
	if (pid == 0)
	{
		if (CAMERA_LAUNCH)
		{
			printf("Failed to launch camera\n");
		}
		exit(1);
	}
	return pid;
}

void disableCamera(pid_t pid)
{
	if (pid > 0)
		kill(pid, SIGKILL);
}

void printClients(const uint32_t* clients)
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		printf("%d ", clients[i]);
	}
	printf("\n");
}
