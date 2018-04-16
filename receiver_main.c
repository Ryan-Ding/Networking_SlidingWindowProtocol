#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "myprotocal.h"

FILE * fd_log;

int handle_output_file(char* file)
{
  int i;
  //printf("file opened %s\n", file);
  fd_log = fopen(file, "w");
  if(!fd_log) return -1;
  
  return 0;
}

int print_to_file(unsigned long long int length, char* buf)
{
  int i;
  fputc(buf[0],fd_log);
  fflush(fd_log);
  return 0;
}

void reliablyReceive(unsigned short int myUDPport, char* destinationFile) {
	char fromAddr[100];
	struct sockaddr_in theirAddr;
	socklen_t theirAddrLen;
	unsigned char recvBuf[1000];
	int i;

	SwpState curr_state;
	curr_state.NFE=0;


	int receiverSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if(receiverSocket < 0)
	{
		perror("socket()");
		exit(1);
	}

	char myAddr[100];
	struct sockaddr_in bindAddr;
	sprintf(myAddr, "127.0.0.1");
	memset(&bindAddr, 0, sizeof(bindAddr));
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_port = htons(myUDPport);
	inet_pton(AF_INET, myAddr, &bindAddr.sin_addr);
	if(bind(receiverSocket, (struct sockaddr*)&bindAddr, sizeof(struct sockaddr_in)) < 0)
	{
		perror("bind");
		close(receiverSocket);
		exit(1);
	}

	if(handle_output_file(destinationFile)<0)
		return;

	int bytesRecvd;
	theirAddrLen = sizeof(theirAddr);
	if ((bytesRecvd = recvfrom(receiverSocket, recvBuf, 1, 0,
				(struct sockaddr*)&theirAddr, &theirAddrLen)) == -1)
	{
		perror("connectivity listener: recvfrom failed");
		exit(1);
	}
	printf("recv:%s\n", recvBuf);
	print_to_file(1, (char*)recvBuf);
	//(should never reach here)
	close(receiverSocket);
}

int main(int argc, char** argv)
{
	unsigned short int udpPort;
	
	if(argc != 3)
	{
		fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
		exit(1);
	}
	
	udpPort = (unsigned short int)atoi(argv[1]);
	
	reliablyReceive(udpPort, argv[2]);
}
