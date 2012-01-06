#include"client.h"

/* 

THIS FUNCTION SEND OUT A REQUEST TO GET A FILE FROM THE SERVER

*/

void getReq(void)
{

	// MESSAGE CONSTRUCTION
	int bytesRecvd=0;
	msg *msgToSend=new msg();
	memset(msgToSend,0,sizeof(msg));
	msgToSend->type=GET_REQ;
	msgToSend->offset=htonl(offset);
	msgToSend->datalen=htonl(Qlen);
	msgToSend->data=new char[Qlen+1];
	strncpy(msgToSend->data,Qstring,Qlen);
	msgToSend->data[Qlen]='\0';
	//cout<<" Msg constructed "<<endl;
	//cout<<" type is "<<msgToSend->type<<endl;
	//cout<<" offset is "<<msgToSend->offset<<endl;
	//cout<<" data len is "<<msgToSend->datalen<<endl;
	//cout<<" data is "<<msgToSend->data<<endl;
	// MESSAGE CONSTRUCTION DONE 
	
	// BUFFER CONSTRUCTION
	int msgSize = sizeof(msgToSend->type)+sizeof(msgToSend->offset)+sizeof(msgToSend->datalen)+Qlen;
	char *msgBuffer= new char[msgSize];
	memset(msgBuffer,0,msgSize);
	memcpy(&msgBuffer[0],(char *)&msgToSend->type,sizeof(msgToSend->type));
	memcpy(&msgBuffer[2],(char *)&msgToSend->offset,sizeof(msgToSend->offset));
	memcpy(&msgBuffer[6],(char *)&msgToSend->datalen,sizeof(msgToSend->datalen));
	memcpy(&msgBuffer[10],(char *)msgToSend->data,Qlen);
	// BUFFER CONSTRUCTION IS DONE 

	// SOCKET IS CREATED FOR COMMUNICATION
	struct sockaddr_in servAddr;
	int cliSock;
	struct hostent *serverIp;
	serverIp=gethostbyname(server);
	cliSock=socket(AF_INET,SOCK_STREAM,0);
	if(cliSock<0)
	{
		cout<<"Error : cannot create a socket for communication "<<endl;
		return;
	}
	memset(&servAddr,0,sizeof(struct sockaddr_in));
	servAddr.sin_family=AF_INET;
	//cout<<"Server is "<<server<<endl;
	memcpy(&servAddr.sin_addr,serverIp->h_addr,serverIp->h_length);
	servAddr.sin_port=htons(serverPort);
	//cout<<"Sevrer port"<<serverPort<<endl;
	int conStat=connect(cliSock,(struct sockaddr*)&servAddr,sizeof(struct sockaddr_in));
	if(conStat<0)
	{
		
		cout<<"Error : Connection failed :"<<conStat<<endl;
		return;
	}
	
	// SENDING HEADER FIRST
	for(int i=0;i<HDR_LEN;i++)
	{
		send(cliSock,&msgBuffer[i],1,0);
	}
	
	// SENDING HEADER DONE
	
	// SENDING PAYLOAD
	//cout<<"\n msg sie is "<<msgSize<<endl;
	for(int i=HDR_LEN;i<msgSize;i++)
	{
		//cout<<"Sending "<<i<<endl;
		send(cliSock,&msgBuffer[i],1,0);
	}

	// SENDING PAYLOAD DONE
	
	// REQUEST SENDING DONE WAITING FOR REPY
	// RECEIVING HEADER
	char *headBuf=new char[HDR_LEN];
	memset(headBuf,'\0',HDR_LEN);
	for(int i=0;i<HDR_LEN;i++)
	{
		if(recv(cliSock,&headBuf[i],1,0)==1)
		{
			bytesRecvd++;
		}
		else
		{
			cout<<"Connection interrupted with server"<<endl;
			exit(1);
		}
		//cout<<"\n recvied "<<i<<"bytes"<<endl;
	}
	msg *receivedMsg=new msg();
	memset(receivedMsg,0,sizeof(msg));
	/*cout<<"\n Received header is "<<endl;
	cout<<" 1 "<<receivedMsg->type<<endl;
	cout<<" 2 "<<receivedMsg->offset<<endl;
	cout<<" 3 "<<ntohl(receivedMsg->datalen)<<endl;*/
	memcpy(&receivedMsg->type,(uint16_t *)&headBuf[0],sizeof(uint16_t));
	memcpy(&receivedMsg->offset,(uint32_t *)&headBuf[2],sizeof(uint32_t));
	memcpy(&receivedMsg->datalen,(uint32_t *)&headBuf[6],sizeof(uint32_t));
	/*cout<<"\n Received header is "<<endl;
	cout<<" 1 "<<receivedMsg->type<<endl;
	cout<<" 2 "<<receivedMsg->offset<<endl;
	cout<<" 3 "<<ntohl(receivedMsg->datalen)<<endl;*/
	// RECEIVING HEADER DONE
	if(receivedMsg->type==GET_RPLY)
	{
		MD5_CTX md;
		unsigned char auth_buffer[16];
		// RECEVING PAYLOAD
		int fileSize=ntohl(receivedMsg->datalen);
		if(fileSize<MAX_DLEN)
		{
			//  FILE SIZE IS LESS THAN 512 BYTES 
			char *payload=new char[(ntohl(receivedMsg->datalen))];
			for(unsigned int i=0;i<(ntohl(receivedMsg->datalen));i++)
			{
				if(recv(cliSock,&payload[i],1,0)==1)
				{
					bytesRecvd++;		// INCREMENTING THE BYTES RECEIVED
				}
				else
				{
					cout<<"Connection intrrupted with server"<<endl;
					exit(1);
				}
			}
			// MUST COMPUTE MD5 HERE
			//cout<<"\n bytes recvied is "<<bytesRecvd<<endl;
			MD5_Init(&md);
			MD5_Update(&md, payload,ntohl(receivedMsg->datalen));
			MD5_Final(auth_buffer, &md);
		    /*cout<<"Md5 is ";
			for(int i=0;i<16;i++)
			{
				printf("%02x",auth_buffer[i]);
			}*/
			cout<<endl;
			if(mset==1)
			{
				// -m OPTION IS SET SO MUST PRINT VERBOSE INFORMATION
				struct sockaddr_in peer_addr;
				socklen_t peer_addrLen;
				peer_addrLen=sizeof(peer_addr);
				int peername=getpeername(cliSock,(struct sockaddr *)&peer_addr,&peer_addrLen);
				if(peername<0)
				{
					cout<<"Error: getpeername failed "<<endl;
				}
				cout<<endl;
				cout<<"\tReceived "<<bytesRecvd<<" bytes from "<<inet_ntoa(peer_addr.sin_addr)<<endl;
				printf("\t  MessageType: 0x%02x\n",receivedMsg->type);
				printf("\t       Offset: 0x%08x\n",receivedMsg->offset);
				printf("\t   DataLength: 0x%08x\n",receivedMsg->datalen);	
				printf("\tFILESIZE = %d",receivedMsg->datalen);
				cout<<" MD5 is ";
				for(int i=0;i<16;i++)
				{
					printf("%02x",auth_buffer[i]);
				}
				cout<<endl;
				
			}
			
		}
		else
		{
			// FILE SIZE IS MORE THAN 512 BYTES
			int bytesRecvdinP=0;
			MD5_Init(&md);
			int bytesToRecv=512;
			int numPackets=fileSize/MAX_DLEN;
			char *payload;
			if((fileSize%MAX_DLEN)>0)
			{
				numPackets=numPackets+1;
			}
			for(int j=0;j<numPackets;j++)
			{
				
				//cout<<" The reply msg buf size is "<<replyBufSize<<endl;
				payload=new char[bytesToRecv];
				for(int i=0;i<bytesToRecv;i++)
				{
					if(recv(cliSock,&payload[i],1,0)==1)
					{
						bytesRecvd++;		// INCREMENTING THE BYTES RECEIVED
						bytesRecvdinP++;
					}
					else
					{
						cout<<"Connection intereuppted with server"<<endl;
						exit(1);
					}
				}
				//cout<<"\n packet "<<j<<" bytes recvd is "<<bytesRecvdinP<<endl;
				//cout<<"\n bytes to recv is "<<bytesToRecv<<endl;
				MD5_Update(&md,payload,bytesToRecv);
				if((bytesRecvdinP+bytesToRecv)>fileSize)
				{
					bytesToRecv=(fileSize-bytesRecvdinP);
				}
				// MUST COMPUTE MD5 HERE
				
			}
			MD5_Final(auth_buffer, &md);
			if(mset==1)
			{
				// -m OPTION IS SET SO MUST PRINT VERBOSE INFORMATION
				struct sockaddr_in peer_addr;
				socklen_t peer_addrLen;
				peer_addrLen=sizeof(peer_addr);
				int peername=getpeername(cliSock,(struct sockaddr *)&peer_addr,&peer_addrLen);
				if(peername<0)
				{
					cout<<"Error: getpeername failed "<<endl;
				}
				cout<<endl;
				cout<<"\tReceived "<<bytesRecvd<<" bytes from "<<inet_ntoa(peer_addr.sin_addr)<<endl;
				printf("\t   MessageType: 0x%02x\n",receivedMsg->type);
				printf("\t        Offset: 0x%08x\n",receivedMsg->offset);
				printf("\t    DataLength: 0x%08x\n",receivedMsg->datalen);	
				printf("\tFILESIZE = %d",receivedMsg->datalen);
				cout<<" MD5 is ";
				for(int i=0;i<16;i++)
				{
					printf("%02x",auth_buffer[i]);
				}
				cout<<endl;
				
			}	
		}
	}
	else if(receivedMsg->type==ALL_FAIL)
	{
			if(mset==1)
			{
				// -m OPTION IS SET SO MUST PRINT VERBOSE INFORMATION
				struct sockaddr_in peer_addr;
				socklen_t peer_addrLen;
				peer_addrLen=sizeof(peer_addr);
				int peername=getpeername(cliSock,(struct sockaddr *)&peer_addr,&peer_addrLen);
				if(peername<0)
				{
					cout<<"Error: getpeername failed "<<endl;
				}
				cout<<endl;
				cout<<"\tReceived "<<bytesRecvd<<" bytes from "<<inet_ntoa(peer_addr.sin_addr)<<endl;
				printf("\t   MessageType: 0x%02x\n",receivedMsg->type);
				printf("\t        Offset: 0x%08x\n",receivedMsg->offset);
				printf("\t    DataLength: 0x%08x\n",receivedMsg->datalen);	
				cout<<"\tALL FAIL received"<<endl;
			}	
	}
	else if(receivedMsg->type==GET_FAIL)
	{
			// GET REQUEST FAILED
			if(mset==1)
			{
				// -m OPTION IS SET SO MUST PRINT VERBOSE INFORMATION
				struct sockaddr_in peer_addr;
				socklen_t peer_addrLen;
				peer_addrLen=sizeof(peer_addr);
				int peername=getpeername(cliSock,(struct sockaddr *)&peer_addr,&peer_addrLen);
				if(peername<0)
				{
					cout<<"Error: getpeername failed "<<endl;
				}
				cout<<endl;
				cout<<"\tReceived "<<bytesRecvd<<" bytes from "<<inet_ntoa(peer_addr.sin_addr)<<endl;
				printf("\t   MessageType: 0x%02x\n",receivedMsg->type);
				printf("\t        Offset: 0x%08x\n",receivedMsg->offset);
				printf("\t    DataLength: 0x%08x\n",receivedMsg->datalen);	
				cout<<"\tGET request for '"<<msgToSend->data<<"' failed"<<endl;
			}	
	}
		
		
		
		//cout<<"\n the pay load is "<<receivedMsg->data<<endl;
		//cout<<"\n len of payload is "<<strlen(receivedMsg->data)<<endl;
		// RECEIVING PAYLOAD DONE 
	}



/*
THIS FUNCTION SENDS A REQUEST TO GET A ADDRESS OF A HOST NAME
*/

void adrReq(void)
{

	// MESSAGE CONSTRUCTION
	int bytesRecvd=0;
	msg *msgToSend=new msg();
	memset(msgToSend,0,sizeof(msg));
	msgToSend->type=ADR_REQ;
	msgToSend->datalen=htonl(Qlen);
	msgToSend->data=new char[Qlen+1];
	strncpy(msgToSend->data,Qstring,Qlen);
	msgToSend->data[Qlen]='\0';
	//cout<<" Msg constructed "<<endl;
	//cout<<" type is "<<msgToSend->type<<endl;
	//cout<<" offset is "<<msgToSend->offset<<endl;
	//cout<<" data len is "<<msgToSend->datalen<<endl;
	//cout<<" data is "<<msgToSend->data<<endl;
	
	// MESSAGE CONSTRUCTION DONE 
	
	// BUFFER CONSTRUCTION
	int msgSize = sizeof(msgToSend->type)+sizeof(msgToSend->offset)+sizeof(msgToSend->datalen)+Qlen;
	char *msgBuffer= new char[msgSize];
	memset(msgBuffer,0,msgSize);
	memcpy(&msgBuffer[0],(char *)&msgToSend->type,sizeof(msgToSend->type));
	memcpy(&msgBuffer[2],(char *)&msgToSend->offset,sizeof(msgToSend->offset));
	memcpy(&msgBuffer[6],(char *)&msgToSend->datalen,sizeof(msgToSend->datalen));
	memcpy(&msgBuffer[10],(char *)msgToSend->data,Qlen);
	
	// BUFFER CONSTRUCTION IS DONE 

	// SOCKET IS CREATED FOR COMMUNICATION
	struct sockaddr_in servAddr;
	int cliSock;
	struct hostent *serverIp;
	serverIp=gethostbyname(server);
	cliSock=socket(AF_INET,SOCK_STREAM,0);
	if(cliSock<0)
	{
		cout<<"Error : cannot create a socket for communication "<<endl;
		return;
	}
	memset(&servAddr,0,sizeof(struct sockaddr_in));
	servAddr.sin_family=AF_INET;
	//cout<<"Server is "<<server<<endl;
	memcpy(&servAddr.sin_addr,serverIp->h_addr,serverIp->h_length);
	servAddr.sin_port=htons(serverPort);
	//cout<<"Sevrer port"<<serverPort<<endl;
	int conStat=connect(cliSock,(struct sockaddr*)&servAddr,sizeof(struct sockaddr_in));
	if(conStat<0)
	{
		
		cout<<"Error : Connection failed :"<<conStat<<endl;
		return;
	}
	
	// SENDING HEADER FIRST
	for(int i=0;i<HDR_LEN;i++)
	{
		send(cliSock,&msgBuffer[i],1,0);
	}
	
	// SENDING HEADER DONE
	
	// SENDING PAYLOAD
	//cout<<"\n msg sie is "<<msgSize<<endl;
	for(int i=HDR_LEN;i<msgSize;i++)
	{
		//cout<<"Sending "<<i<<endl;
		send(cliSock,&msgBuffer[i],1,0);
	}

	// SENDING PAYLOAD DONE
	
	// REQUEST SENDING DONE WAITING FOR REPY
	
	// RECEIVING HEADER
	char *headBuf=new char[HDR_LEN];
	for(int i=0;i<HDR_LEN;i++)
	{
		if(recv(cliSock,&headBuf[i],1,0)==1)
		{
			bytesRecvd++;
		}
		else
		{
			cout<<"Connection interuppted with server"<<endl;
			exit(1);
		}
		//cout<<"\n recvied "<<i<<"bytes"<<endl;
	}
	msg *receivedMsg=new msg();
	memcpy(&receivedMsg->type,(uint16_t *)&headBuf[0],sizeof(uint16_t));
	memcpy(&receivedMsg->offset,(uint32_t *)&headBuf[2],sizeof(uint32_t));
	memcpy(&receivedMsg->datalen,(uint32_t *)&headBuf[6],sizeof(uint32_t));
	//cout<<"\n Received header is "<<endl;
	//cout<<" 1 "<<receivedMsg->type<<endl;
	//cout<<" 2 "<<receivedMsg->offset<<endl;
	//cout<<" 3 "<<ntohl(receivedMsg->datalen)<<endl;
	// RECEIVING HEADER DONE 
	
	if(receivedMsg->type==ADR_RPLY)
	{
		// RECEVING PAYLOAD
		receivedMsg->data= new char[ntohl(receivedMsg->datalen)+1];
		memset(receivedMsg->data,'\0',ntohl(receivedMsg->datalen)+1);
		char *payload=new char[(ntohl(receivedMsg->datalen))];
		for(unsigned int i=0;i<(ntohl(receivedMsg->datalen));i++)
		{
			if(recv(cliSock,&payload[i],1,0)==1)
			{
				bytesRecvd++;		// INCREMENTING THE BYTES RECEIVED
				//cout<<"\n recvied "<<i<<"bytes"<<endl;
			}
			else
			{
				cout<<"Connection interrupted with server"<<endl;
				exit(1);
			}
		}
		memcpy(receivedMsg->data,(char *)&payload[0],(ntohl(receivedMsg->datalen)));
		//cout<<"\n the pay load is "<<receivedMsg->data<<endl;
		//cout<<"\n len of payload is "<<strlen(receivedMsg->data)<<endl;
		// RECEIVING PAYLOAD DONE 
	}
	
	// CHECKING IF -m OPTION IS SET OR NOT
	if(mset==1)
	{
		// -m OPTION IS SET SO MUST PRINT VERBOSE INFORMATION
		struct sockaddr_in peer_addr;
		socklen_t peer_addrLen;
		peer_addrLen=sizeof(peer_addr);
		int peername=getpeername(cliSock,(struct sockaddr *)&peer_addr,&peer_addrLen);
		if(peername<0)
		{
			cout<<"Error: getpeername failed "<<endl;
		}
		cout<<endl;
		cout<<"\tReceived "<<bytesRecvd<<" bytes from "<<inet_ntoa(peer_addr.sin_addr)<<endl;
		printf("\t  MessageType: 0x%02x\n",receivedMsg->type);
		printf("\t       Offset: 0x%08x\n",receivedMsg->offset);
		printf("\t   DataLength: 0x%08x\n",receivedMsg->datalen);
		if(receivedMsg->type==ADR_RPLY)
		{	
			printf("\tADDR = %s",receivedMsg->data);
			cout<<endl;
		}
		else if(receivedMsg->type==ALL_FAIL)
		{
			cout<<"\tALL FAIL received "<<endl;
		}
		else
		{
			// ADDR REQ FAILED 
			cout<<"\tADDR request for '"<<msgToSend->data<<"' failed"<<endl;
		}
	}
}

/*
THIS FUNCTION SENDS A REQUEST TO GET A SZIE OF A FILE
*/
void fszReq(void)
{
	// MESSAGE CONSTRUCTION
	int bytesRecvd=0;
	msg *msgToSend=new msg();
	//msgToSend=(msg *)malloc(sizeof(msg));
	memset(msgToSend,0,sizeof(msg));
	msgToSend->type=FSZ_REQ;
	msgToSend->datalen=htonl(Qlen);
	msgToSend->data=new char[Qlen+1];
	strncpy(msgToSend->data,Qstring,Qlen);
	msgToSend->data[Qlen]='\0';
	//cout<<" Msg constructed "<<endl;
	//cout<<" type is "<<msgToSend->type<<endl;
	//cout<<" offset is "<<msgToSend->offset<<endl;
	//cout<<" data len is "<<msgToSend->datalen<<endl;
	//cout<<" data is "<<msgToSend->data<<endl;
	int msgSize = sizeof(msgToSend->type)+sizeof(msgToSend->offset)+sizeof(msgToSend->datalen)+Qlen;
	char *msgBuffer= new char[msgSize];
	memset(msgBuffer,0,msgSize);
	memcpy(&msgBuffer[0],(char *)&msgToSend->type,sizeof(msgToSend->type));
	memcpy(&msgBuffer[2],(char *)&msgToSend->offset,sizeof(msgToSend->offset));
	memcpy(&msgBuffer[6],(char *)&msgToSend->datalen,sizeof(msgToSend->datalen));
	memcpy(&msgBuffer[10],(char *)msgToSend->data,Qlen);

	// SOCKET IS CREATED FOR COMMUNICATION
	struct sockaddr_in servAddr;
	int cliSock;
	struct hostent *serverIp;
	serverIp=gethostbyname(server);
	cliSock=socket(AF_INET,SOCK_STREAM,0);
	if(cliSock<0)
	{
		cout<<"Error : cannot create a socket for communication "<<endl;
		return;
	}
	memset(&servAddr,0,sizeof(struct sockaddr_in));
	servAddr.sin_family=AF_INET;
	//cout<<"Server is "<<server<<endl;
	memcpy(&servAddr.sin_addr,serverIp->h_addr,serverIp->h_length);
	servAddr.sin_port=htons(serverPort);
	//cout<<"Sevrer port"<<serverPort<<endl;
	int conStat=connect(cliSock,(struct sockaddr*)&servAddr,sizeof(struct sockaddr_in));
	if(conStat<0)
	{
		
		cout<<"Error : Connection failed :"<<conStat<<endl;
		return;
	}
	
	// SENDING HEADER FIRST
	for(int i=0;i<HDR_LEN;i++)
	{
		send(cliSock,&msgBuffer[i],1,0);
	}
	
	// SENDING HEADER DONE
	
	// SENDING PAYLOAD
	//cout<<"\n msg sie is "<<msgSize<<endl;
	for(int i=HDR_LEN;i<msgSize;i++)
	{
		//cout<<"Sending "<<i<<endl;
		send(cliSock,&msgBuffer[i],1,0);
	}

	// SENDING PAYLOAD DONE
	
	// REQUEST SENDING DONE WAITING FOR REPLY
	
	// RECEIVING HEADER
	char *headBuf=new char[HDR_LEN];
	for(int i=0;i<HDR_LEN;i++)
	{
		if(recv(cliSock,&headBuf[i],1,0)==1)
		{
			bytesRecvd++;
		}
		else
		{
			cout<<"Connection interupted with server"<<endl;
			exit(1);
		}
		//cout<<"\n recvied "<<i<<"bytes"<<endl;
	}
	msg *receivedMsg=new msg();
	memcpy(&receivedMsg->type,(uint16_t *)&headBuf[0],sizeof(uint16_t));
	memcpy(&receivedMsg->offset,(uint32_t *)&headBuf[2],sizeof(uint32_t));
	memcpy(&receivedMsg->datalen,(uint32_t *)&headBuf[6],sizeof(uint32_t));
	//cout<<"\n Received header is "<<endl;
	//cout<<" 1 "<<receivedMsg->type<<endl;
	//cout<<" 2 "<<receivedMsg->offset<<endl;
	//cout<<" 3 "<<ntohl(receivedMsg->datalen)<<endl;
	// RECEIVING HEADER DONE 
	
	
	if(receivedMsg->type==FSZ_RPLY)
	{
		// RECEVING PAYLOAD
		receivedMsg->data= new char[ntohl(receivedMsg->datalen)+1];
		memset(receivedMsg->data,'\0',ntohl(receivedMsg->datalen)+1);
		char *payload=new char[(ntohl(receivedMsg->datalen))];
		for(unsigned int i=0;i<(ntohl(receivedMsg->datalen));i++)
		{
			if(recv(cliSock,&payload[i],1,0)==1)
			{
				bytesRecvd++;		// INCREMENTING THE BYTES RECEIVED
				//cout<<"\n recvied "<<i<<"bytes"<<endl;
			}
			else
			{
				cout<<"connection intreuppted with server"<<endl;
				exit(1);
			}
		}
		memcpy(receivedMsg->data,(char *)&payload[0],(ntohl(receivedMsg->datalen)));
		//cout<<"\n the pay load is "<<receivedMsg->data<<endl;
		//cout<<"\n len of payload is "<<strlen(receivedMsg->data)<<endl;
		// RECEIVING PAYLOAD DONE 
	}
	
	// CHECKING IF -m OPTION IS SET OR NOT
	if(mset==1)
	{
		// -m OPTION IS SET SO MUST PRINT VERBOSE INFORMATION
		struct sockaddr_in peer_addr;
		socklen_t peer_addrLen;
		peer_addrLen=sizeof(peer_addr);
		int peername=getpeername(cliSock,(struct sockaddr *)&peer_addr,&peer_addrLen);
		if(peername<0)
		{
			cout<<"Error: getpeername failed "<<endl;
		}
		cout<<endl;
		cout<<"\tReceived "<<bytesRecvd<<" bytes from "<<inet_ntoa(peer_addr.sin_addr)<<endl;
		printf("\t  MessageType: 0x%02x\n",receivedMsg->type);
		printf("\t       Offset: 0x%08x\n",receivedMsg->offset);
		printf("\t   DataLength: 0x%08x\n",receivedMsg->datalen);
		if(receivedMsg->type==FSZ_RPLY)
		{	
			printf("\tFILESIZE = %s",receivedMsg->data);
			cout<<endl;
		}
		else if(receivedMsg->type==ALL_FAIL)
		{
			cout<<"\tALL FAIL Received"<<endl;
		}
		else
		{
			// FSZ REQ FAILED 
			cout<<"\tFILESIZE request for '"<<msgToSend->data<<"' failed"<<endl;
		}
	}
	

}

int main(int argc,char *argv[])
{
	if((argc)<4)
	{
		cout<<"Malformed command"<<endl;
		exit(1);
	}
	hostNport= new char[strlen(argv[argc-2])];
	strncpy(hostNport,argv[argc-2],strlen(argv[argc-2]));
	Qlen=strlen(argv[argc-1]);
	if(Qlen >512)
	{
		cout<<"malformed command"<<endl;
		exit(1);
	}
	//cout<<"LEN "<<Qlen<<endl;;
	Qstring= new char[Qlen+1];
	//strncpy(Qstring,argv[argc-1],strlen(argv[argc-1]));
	strncpy(Qstring,argv[argc-1],Qlen);
	Qstring[Qlen]='\0';
	//cout<<"Main len of query string is "<<strlen(Qstring)<<endl;
	
	for (int i=1;i<argc;i++)
	{
		if(!(strncmp(argv[i],"get",3)))
		{
			get=1;
		}
		if(!(strncmp(argv[i],"fsz",3)))
		{
			fsz=1;
		}
		if(!(strncmp(argv[i],"adr",3)))
		{
			adr=1;
		}
		if(!(strncmp(argv[i],"-m",2)))
		{
			mset=1;
		}
		if(!(strncmp(argv[i],"-o",2)))
		{
			int digit=1;
			char *timer=new char[strlen(argv[i+1])+1];
			memset(timer,'\0',strlen(argv[i+1])+1);
			strncpy(timer,argv[i+1],strlen(argv[i+1]));
			for(unsigned int j=0;j<strlen(timer);j++)
			{
				if(!isdigit(timer[j]))
				{
					digit=0;
				}
			}
			if(digit==0)
			{
				cout<<"bad offset argument"<<endl;
				exit(1);
			}
			else
			{
				offset=atoi(argv[i+1]);
				offsetGiven=1;
			}
			
			//cout<<"\n offset given"<<offset<<endl;
		}
		/*else
		{
			cout<<"Malformed command "<<endl;
			exit(1);
		}*/
	}
	if((offsetGiven==1) && (adr==1 || fsz==1) )
	{
		cout<<" Error wrong use of command line arguments "<<endl;
		exit(1);
	}
	//cout<<" The Qstring is "<<hostNport<<endl;
	//cout<<" The hostNport "<<Qstring<<endl;
	char *token=strtok(hostNport,":");
	if(token==NULL)
	{
		cout<<"Not a valid host to connect to"<<endl;
		exit(1);
	}
	server=new char[strlen(token)+1];
	strncpy(server,token,strlen(token));
	server[strlen(token)]='\0';
	//cout<<" the server is "<<server<<endl;
	token=strtok(NULL,":");
	if(token==NULL)
	{
		cout<<"Not a valid host to connect to"<<endl;
		exit(1);
	}
	serverPort=atoi(token);
	//cout<<"\n the server is at port "<<serverPort<<endl;
	free(hostNport);
	if(get==1 && fsz==0 && adr==0)
	{
		//  CALL THE METHOD TO SEND A GET REQUEST
		getReq();
	}
	else if(get==0 && fsz==1 && adr==0)
	{
		//  CALL THE METHOD TO SEND A FILE SIZE REQUEST
		fszReq();
	}
	else if(get==0 && fsz==0 && adr==1)
	{
		// CALL THE METHOD TO SEND ADDR REQUEST
		adrReq();
	}
	else
	{
		// ERROR INVALID NUMBER/COMBINATION OF COMMAND LINE ARGUEMENTS
		cout<<" Error : Invalid combination of command line arguments"<<endl;
	}
}
