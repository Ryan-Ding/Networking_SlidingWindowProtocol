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
#include <time.h>
#include "myprotocal.h"

int global_file_offset;
int Send_Sequence_Number;

void reliablyTransfer(char* hostname, unsigned short int hostUDPport, char* filename, unsigned long long int bytesToTransfer);
void send_packet(int socket, struct sockaddr_in * send_address, struct sendQ_slot * msg, int length);

int handle_input_file(char* file, unsigned long long int length, char * buf, int offset)
{
  FILE * fd_send;
  int i;
  //printf("file opened %s\n", file);
  fd_send = fopen(file, "r");
  fseek(fd_send, offset, SEEK_SET);
  if(!fd_send) return -1;
  
  for(i=0; i<length; i++)
  {
  	if((buf[i]=fgetc(fd_send))==EOF)
  	{
  		fclose(fd_send);
  		return 1;
  	}
  }
  fclose(fd_send);
  return 0;
}

void send_multiple_packet (int socket, struct sockaddr_in * send_address, SwpState * state,
	int start_frame, int last_frame)
{
	int i;
	for(i = start_frame; i <= last_frame; i++)
	{
		send_packet(socket, send_address, &(state -> sendQ[i]), MAXDATASIZE);
	}
}

void send_packet(int socket, struct sockaddr_in * send_address, struct sendQ_slot * msg, int length)
{
    if(sendto(socket, msg->msg, length, 0, (struct sockaddr*) send_address, sizeof(struct sockaddr)) < 0)
      perror("sendto()");
}

bool swpInWindows(uint8_t AckNum, uint8_t left, uint8_t right)
{
	if( AckNum >= left && AckNum <= right)
		return true;

	return false;
}

void fill_sending_window(SwpState * state, uint8_t LAR, uint8_t LFS, char* filename)
{
	int i;
	for(i = LFS + 1; i < LAR + SWS + 1 ; i++)
	{
		state->sendQ[i].msg[0] = Send_Sequence_Number;
		handle_input_file(filename, MAXDATASIZE, &state->sendQ[i].msg[1], global_file_offset);
		Send_Sequence_Number++;
		global_file_offset += MAXDATASIZE;
	}
}

static int deliverSWP(SwpState * state, char * recvBuf, 
	int socket, struct sockaddr_in * send_address, char * filename)
{
	uint8_t ACK_Sequence_Number;
	ACK_Sequence_Number = recvBuf[0];
	if(swpInWindows(ACK_Sequence_Number, state -> LAR + 1, state-> LFS))
	{
		if(state -> LAR == ACK_Sequence_Number)
			return 0;

		do
		{
			struct sendQ_slot * slot;
			slot = &state -> sendQ[++state -> LAR % SWS];
			//msgDestroy(&slot->msg);
			//semSignal(&state->sendWindowNotFull);
		} while (state->LAR != ACK_Sequence_Number);

		fill_sending_window(state, state-> LAR, state-> LFS, filename);
		send_multiple_packet(socket, send_address, state, state->LAR + 1, state -> LFS);
		return 1;
	}
}


int main(int argc, char** argv)
{
	unsigned short int udpPort;
	unsigned long long int numBytes;
	
	if(argc != 5)
	{
		fprintf(stderr, "usage: %s receiver_hostname receiver_port filename_to_xfer bytes_to_xfer\n\n", argv[0]);
		exit(1);
	}
	udpPort = (unsigned short int)atoi(argv[2]);
	numBytes = atoll(argv[4]);
	
	reliablyTransfer(argv[1], udpPort, argv[3], numBytes);
} 

void reliablyTransfer(char* hostname, unsigned short int hostUDPport, char* filename, unsigned long long int bytesToTransfer) {
	long send_start;
	long last_ack,received_ack;
	int i;
	int recv_socket;
	socklen_t transfer_Len;
	struct timespec ack_timer;
	struct timespec my_timer;
	//struct Msg my_packet[SWS];
	struct sockaddr_in transfer_addr;
	struct sockaddr_in from_addr;
	fd_set rfds;
	struct timeval tv;

	char recvBuf;

	unsigned long long int numBytes;

	global_file_offset = 0;
	Send_Sequence_Number = 0;

	//create socket
	int senderSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if(senderSocket < 0)
		perror("socket()");

		//get receiver IP address
		
	memset(&transfer_addr, 0, sizeof(transfer_addr));
	socklen_t transfer_addr_len = sizeof(transfer_addr);
	transfer_addr.sin_family = AF_INET;
	transfer_addr.sin_port = htons(hostUDPport);
	inet_pton(AF_INET, hostname, &transfer_addr.sin_addr);

	SwpState curr_state;
	curr_state.LAR = 0;
	curr_state.LFS = 0;
	// curr_state.SwpHeader.Send_Sequence_Number=0;
	// curr_state.SwpHeader.ACK_Sequence_Number=0;
	// curr_state.SwpHeader.flags=0;

	curr_state.sendQ[0].msg[0] = 0;
	handle_input_file(filename, 1, &curr_state.sendQ[0].msg[1], global_file_offset);
	Send_Sequence_Number ++;
	// send_packet(senderSocket, &transfer_addr, &buffer, 1);
	send_multiple_packet(senderSocket, &transfer_addr, &curr_state, curr_state.LAR + 1, curr_state.LFS);
	while(1){
		 if((numBytes = recvfrom(senderSocket,&recvBuf,sizeof(uint8_t),0,
		 		(struct sockaddr *) &transfer_addr,&transfer_addr_len))==-1){
		 	if (errno != EAGAIN || errno != EWOULDBLOCK){
		 		perror("can not receive ack");
            	exit(2);
		 	}
		 	else
		 	{
		 		//resend because of time out
		 		send_packet(senderSocket, &transfer_addr, &(curr_state.sendQ[curr_state.LAR + 1]), MAXDATASIZE);
		 	}

        }
        deliverSWP(&curr_state, &recvBuf, senderSocket, &transfer_addr, filename);
	}

}