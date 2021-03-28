#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>

#include "communication.h"

#ifdef PI
	#define CAMERA_LAUNCH execlp("motion", "motion")
#else
	#define CAMERA_LAUNCH 0
#endif

#define PORT		8085
#define MAX_CLIENTS 5
#define MAX_PENDING_CONNECTIONS 2

pid_t launchCamera();
void* acceptRoutine(void*);
void disableCamera(pid_t pid);

int main (int argc, char** argv)
{
	int sok = socket(AF_INET, SOCK_STREAM, 0);
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

	if (listen(sok, MAX_PENDING_CONNECTIONS) < 0)
	{
		printf("Failed to listen on the socket\n");
		return 1;
	}
	
	communicationParams params;
	params.maxClients = MAX_CLIENTS;
	if (initCommunication(params) < 0)
	{
		printf("Failed to establish communication\n");
		return -1;
	}

	pthread_t threadId;
	pthread_create(&threadId, NULL, &acceptRoutine, (void*)&sok);

	pid_t cameraPid = 0;
	while(1)
	{
		int numClients = readClients();
		if (numClients > 0 && cameraPid == 0)
		{
			cameraPid = launchCamera();
		}
		else if (numClients == 0 && cameraPid != 0)
		{
			disableCamera(cameraPid);
			cameraPid = 0;
		}
	}

	closeCommunication();
	close(sok);
	return 0;
}

void* acceptRoutine(void* arg)
{
	int serverSocket = *(int*)arg;
	struct sockaddr_in clientAddress;
	socklen_t addressLength = sizeof(clientAddress);
	while(1)
	{
		int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &addressLength);
		if (clientSocket < 0)
		{
			printf("Failed to accept connection\n");
			continue;
		}
		
		connectClient(clientSocket);
	}
	return NULL;
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

