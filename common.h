/*
 
 FILE NAME 	: common.h
 AUTHOR		: GV ASHWIN KUMAR | gopivall@usc.edu
 THIS IS THE COMMON HEADER FILE THAT WILL BE USED BY BOTH
 THE SERVER AND THE CLIENT.

*/
#include<iostream>
#include<stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/stat.h>
#include <netdb.h>
#include <fstream>




#define ADR_REQ htons(0xFE10)
#define FSZ_REQ htons(0xFE20)
#define GET_REQ htons(0xFE30)

#define ADR_RPLY htons(0xFE11)
#define FSZ_RPLY htons(0xFE21)
#define GET_RPLY htons(0xFE31)

#define ADR_FAIL htons(0xFE12)
#define FSZ_FAIL htons(0xFE22)
#define GET_FAIL htons(0xFE32)

#define ALL_FAIL htons(0xFCFE)


#define HDR_LEN 10

#define MAX_DLEN 512



using namespace std;

struct MsgStruct
{
	uint16_t type;
	uint32_t offset;
	uint32_t datalen;
	char *data;
};

typedef MsgStruct msg;



void allFail(int *);
