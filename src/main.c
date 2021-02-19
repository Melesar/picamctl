#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "communication.h"

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
	
	communicationParams params;
	params.maxClients = MAX_CLIENTS;
	if (initCommunication(params) < 0)
	{
		printf("Failed to establish communication\n");
		return -1;
	}
	
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

		uint32_t addressValue = clientAddress.sin_addr.s_addr;
		unsigned short receivedValue = *((unsigned short*)buffer);
		printf("Received %d\n", receivedValue);

		int clientsNum;
		switch (receivedValue)
		{
			case CMD_CONNECT:;
				clientsNum = connectClient(addressValue);
				if (clientsNum > 0 && cameraPid == 0)
				{
					cameraPid = launchCamera();
				}
				break;
			case CMD_DISCONNECT:;
				clientsNum = disconnectClient(addressValue);
				if (clientsNum == 0 && cameraPid != 0)
				{
					disableCamera(cameraPid);
					cameraPid = 0;
				}
				break;
			case CMD_DISCONNECT_ALL:
				disconnectAll();
				disableCamera(cameraPid);
				cameraPid = 0;
				break;
		}
	}

	closeCommunication();
	close(sok);
	return 0;
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

