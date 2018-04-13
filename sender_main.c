#include <stdio.h>
#include <stdlib.h>

static int sendSWP(SwpState * state, Msg * frame)
{
	header hdr;
	char * hbuf;

	hbuf = msgStripHdr(frame, HLEN);
	load_swp_hdr(&hdr, hbuf);
	if(swpInWindows(hdr.ACK_Sequence_Number, state -> LAR + 1, state->LFS))
	{
		do
		{
			struct sendQ_slot * slot;
			slot = &state -> sendQ[++state -> LAR % SWS];
			evCancel(slot->timeout);
			msgDestroy(&slot->msg);
			semSignal(&state->sendWindowNotFull);
		} while (state->LAR != hdr.AckNum);
	}
}

void reliablyTransfer(char* hostname, unsigned short int hostUDPport, char* filename, unsigned long long int bytesToTransfer) {

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
	int file_offset=0;
	struct timespec ack_timer;
	struct timespec my_timer;
	//struct Msg my_packet[SWS];
	struct sockaddr_in transfer_addr;
	struct sockaddr_in from_addr;
	fd_set rfds;
	struct timeval tv;

	//create socket
	int senderSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if(senderSocket < 0)
		perror("socket()");

		//get receiver IP address
		
	memset(&transfer_addr, 0, sizeof(transfer_addr));
	transfer_addr.sin_family = AF_INET;
	transfer_addr.sin_port = htons(hostUDPport);
	inet_pton(AF_INET, hostname, &transfer_addr.sin_addr);

	SwpState curr_state;
	curr_state.LAR=0;
	curr_state.LFS=0;
	curr_state.SwpHeader.Send_Sequence_Number=0;
	curr_state.SwpHeader.ACK_Sequence_Number=0;
	curr_state.SwpHeader.flags=0;

	while(1)
	{

	}

}