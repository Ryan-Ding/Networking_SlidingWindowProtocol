#ifndef MYPROTOCAL_H
#define MYPROTOCAL_H

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/select.h>
#include <sys/time.h>

#define MAXDATASIZE 1
#define SWS 1
#define RWS 10
#define HLEN 3

typedef struct header_struct
{
	/* data */
	uint8_t Send_Sequence_Number;
	uint8_t ACK_Sequence_Number;
	uint8_t flags;
	//0 for data, 1 for ack
} header;


typedef struct {
/* sender side state: */

uint8_t LAR; /* seqno of last ACK received */
uint8_t LFS; /* last frame sent */
struct header_struct SwpHeader; /* pre-initialized header */

struct sendQ_slot {
char msg[MAXDATASIZE];
} sendQ[SWS];

/* receiver side state: */
uint8_t NFE; /* seqno of next frame
expected */

struct recvQ_slot {
char msg[MAXDATASIZE];
} recvQ[RWS];

} SwpState;

#endif