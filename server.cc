#include"server.h"
void allFail(int *);

/*  THIS FUNCTION SHUTS DOWN THE SERVER */
void shutDown(void)
{
	pthread_mutex_lock(threadRefLok);
	for(int i=0;i<(int)threadRefs.size();i++)
	{
		if(threadRefs[i]!=pthread_self()) //  A THREAD CANNOT CANCEL ITSELF
		{
			pthread_cancel(threadRefs[i]);	// CANCELLING ALL OTHER THREADS
		}
	}
	pthread_mutex_unlock(threadRefLok);
}

/*
 THIS IS THE FUNCTION THREAD KEEPS TRACK OF THE 
 TIMER SHUTDOWN TIME
*/
void *timerFn(void *arg)
{
	//cout<<"\n Inside timer fn"<<endl;
	for(int i=0;i<autoShut;i++)
	{
		pthread_testcancel();
		sleep(1);
	}
	cout<<"\n Timer out expired in server"<<endl;
	shutDown();
	pthread_exit(NULL);
}

/*
A SIGNAL HANDLER THREAD WHICH  HANDLES ALL THE SIGNALS
*/
void *signalHandler(void *arg)
{

    int recvSignal;
    for(;;)
     {
        sigwait(&signals,&recvSignal);
        if(recvSignal == SIGPIPE)
        {
            cout<<"Connection interuppted with client. Broken pipe handled"<<endl;
        }
        else if( recvSignal == SIGINT)
        {
            //terminateServer();
        //pthread_exit(NULL);
         //cout<<"Termination caught"<<endl;
		 cout<<" Ctrl+c pressed by the user "<<endl;
         shutDown();
		 pthread_exit(NULL);
        }
      }


}

/*
 THIS FUNCTION HANDLES THE FILE GET REQUEST
 THIS TAKES THE CLIENT SOCKET DESCRIPTOR , FILE NAME, AND THE FILE NAME LENGTH
 AS ITS ARGUMENTS. ITS A VOID FUNCTION AND RETURNS NOTHING 
*/
void getReq(int *cliSock,char *fileName,int nameLen, int fileOffset)
{
	
	//cout<<"file offset "<<fileOffset<<endl;
    char *file= new char[nameLen+1];
    memset(file,'\0',nameLen+1);
    strncpy(file,fileName,nameLen);
   // cout<<" the file in get request is "<<file<<endl;
    struct stat *fileStat=new struct stat();
	ifstream fileptr;
	fileptr.open(file,ios::in|ios::binary);
	/*if(fileptr.is_open())
	{
		cout<<"\n ------------------------------------------->can open file"<<endl;
	}
	else
	{
		cout<<"\n --------------------------------------------> cannot open file"<<endl;
	}
	cout<<"file name is "<<file<<endl;
	cout<<"file size is "<<stat(file,fileStat)<<endl;*/
	
    if(stat(file,fileStat)!=0 || !fileptr.is_open())
    {
		// file not found
        cout<<"\n Error : file not found or not enough permissions to access the file "<<endl;
        msg *replyMsg=new msg();
        memset(replyMsg,0,sizeof(msg));
        replyMsg->type=GET_FAIL;
        replyMsg->datalen=0;
        // REPLY MSG CONSTRUCTION IS DONE
       
        // REPLY BUFFER CONSTRUCTION BEGINS
        int replyBufSize=sizeof(replyMsg->type)+sizeof(replyMsg->offset)+sizeof(replyMsg->datalen);
        //cout<<" The reply msg buf size is "<<replyBufSize<<endl;
        char *replyBuf=new char[replyBufSize];
        memcpy(&replyBuf[0],(char *)&replyMsg->type,sizeof(replyMsg->type));
        memcpy(&replyBuf[2],(char *)&replyMsg->offset,sizeof(replyMsg->offset));
        memcpy(&replyBuf[6],(char *)&replyMsg->datalen,sizeof(replyMsg->datalen));
        // REPLY BUFFER CONSTRUCTION IS DONE
       
        // SEDING HEADER
        for(int i=0;i<HDR_LEN;i++)
        {
            if(send(*cliSock,&replyBuf[i],1,0)<0)
            {
				pthread_testcancel();
                cout<<"Error in conncection "<<endl;
                return;
            }
            //cout<<" Sending "<< i <<"byte"<<endl;
			pthread_testcancel();
        }
        // SENDING HEADER DONE
		return;
   
    }
    else
    {
        // THE REQUESTED FILE IS FOUND
        //cout<<" The file size is "<<fileStat->st_size<<endl;
        int fileSize=fileStat->st_size;
		if((fileOffset)>=fileSize)
		{
			msg *replyMsg=new msg();
			memset(replyMsg,0,sizeof(msg));
			replyMsg->type=GET_RPLY;
			replyMsg->datalen=htonl(0);
			// REPLY MSG CONSTRUCTION IS DONE
			
			// REPLY BUFFER CONSTRUCTION BEGINS
			int replyBufSize=sizeof(replyMsg->type)+sizeof(replyMsg->offset)+sizeof(replyMsg->datalen);
			//cout<<" The reply msg buf size is "<<replyBufSize<<endl;
			char *replyBuf=new char[replyBufSize];
			memcpy(&replyBuf[0],(char *)&replyMsg->type,sizeof(replyMsg->type));
			memcpy(&replyBuf[2],(char *)&replyMsg->offset,sizeof(replyMsg->offset));
			memcpy(&replyBuf[6],(char *)&replyMsg->datalen,sizeof(replyMsg->datalen));
			// REPLY BUFFER CONSTRUCTION IS DONE
			   
			// SEDING HEADER
			for(int i=0;i<HDR_LEN;i++)
			{
				if(send(*cliSock,&replyBuf[i],1,0)<0)
				{
					pthread_testcancel();
					cout<<"Error in conncection "<<endl;
					return;
				 }
					//cout<<" Sending "<< i <<"byte"<<endl;
					pthread_testcancel();
			}
				// SENDING HEADER DONE      
				// NO NEED TO SEND PAYLOAD SINCE THE ADDRESS IS FAIL
			return;
		}
		else
		{
			if((fileSize-fileOffset)<=MAX_DLEN)
			{
				// REPLY MSG CONSTRUCTION BEGINS
				msg *replyMsg=new msg();
				memset(replyMsg,0,sizeof(msg));
				replyMsg->type=GET_RPLY;
				replyMsg->datalen=htonl((fileSize-fileOffset));
				replyMsg->data=new char[MAX_DLEN];
				memset(replyMsg->data,'\0',MAX_DLEN);
				//  THE REQUESTED FILE SIZE IS WITHIN THE 512 BYTES
				//    JUST SENDING ONE PACKET IS ENOUGH
				//ifstream fileptr(file,ios::in|ios::binary); COMMENTED BY ME FOR SHADOW
				/*if (fileptr==NULL)
				{
					cout<<"\n No File permissions "<<endl;
				}*/
				// SINCE THE FILE SIZE IS LESS THAN 512
				// READING THE WHOLE FILE INTO THE FILE BUFFER
				fileptr.seekg(fileOffset); // FOR SEEKING THE FILE
				fileptr.read(replyMsg->data,fileSize);
				// REPLY MSG CONSTRUCTION IS DONE
		   
				// REPLY BUFFER CONSTRUCTION BEGINS
				int replyBufSize=sizeof(replyMsg->type)+sizeof(replyMsg->offset)+sizeof(replyMsg->datalen)+(fileSize-fileOffset);
				//cout<<" The reply msg buf size is "<<replyBufSize<<endl;
				char *replyBuf=new char[replyBufSize];
				memcpy(&replyBuf[0],(char *)&replyMsg->type,sizeof(replyMsg->type));
				memcpy(&replyBuf[2],(char *)&replyMsg->offset,sizeof(replyMsg->offset));
				memcpy(&replyBuf[6],(char *)&replyMsg->datalen,sizeof(replyMsg->datalen));
				memcpy(&replyBuf[10],(char *)replyMsg->data,fileSize);
				// REPLY BUFFER CONSTRUCTION IS DONE
			   
				// SEDING HEADER
				for(int i=0;i<HDR_LEN;i++)
				{
					if(send(*cliSock,&replyBuf[i],1,0)<0)
					{
						pthread_testcancel();
						cout<<"Header : Error in conncection "<<endl;
						return;
					}
					//cout<<" Sending "<< i <<"byte"<<endl;
					pthread_testcancel();
				}
				// SENDING HEADER DONE
	   
				// SENDING PAYLOAD
				//cout<<"replyBufSize"<<replyBufSize<<endl;
				for(int i=HDR_LEN;i<replyBufSize;i++)
				{
					//cout<<"Sending "<<i<<endl;
					if(send(*cliSock,&replyBuf[i],1,0)<0)
					{
						pthread_testcancel();
						cout<<"Error in conncection "<<endl;
						return;
					}
					pthread_testcancel();
				}
				// SENDING PAYLOAD DONE
			}
			else
			{
				//  THE REQUESTED FILE SIZE IS MORE THAN 512 BYTES
				//    MUST DO MORE THAN ONE PACKET
				int bytesRead=0;
				int bytesToRead=MAX_DLEN;
				int numPackets=fileSize/MAX_DLEN;
				if(((fileSize-fileOffset)%MAX_DLEN)>0)
				{
					numPackets=numPackets+1;
				}
				msg *replyMsg=new msg();
				memset(replyMsg,0,sizeof(msg));
				replyMsg->type=GET_RPLY;
				replyMsg->datalen=htonl(fileSize-fileOffset);
				replyMsg->data=new char[MAX_DLEN];
				//memset(replyMsg->data,'\0',MAX_DLEN);
				/*cout<<"1 "<<replyMsg->type<<endl;
				cout<<"2 "<<replyMsg->offset<<endl;
				cout<<"3 "<<replyMsg->datalen<<endl;*/
			   
				// REPLY BUFFER IS CREATED FOR HEADER
				char *replyHBuf=new char[HDR_LEN];
				
				memcpy(&replyHBuf[0],(char *)&replyMsg->type,sizeof(replyMsg->type));
				memcpy(&replyHBuf[2],(char *)&replyMsg->offset,sizeof(replyMsg->offset));
				memcpy(&replyHBuf[6],(char *)&replyMsg->datalen,sizeof(replyMsg->datalen));
				// REPLY BUFFER CREATION IS DONE FOR HEADER
			   
				// SEDING HEADER
				for(int i=0;i<HDR_LEN;i++)
				{
					if(send(*cliSock,&replyHBuf[i],1,0)<0)
					{
						pthread_testcancel();
						cout<<"Error in conncection "<<endl;
						return;
					}
					pthread_testcancel();
				//cout<<" Sending "<< i <<"byte"<<endl;
				}
				// SENDING HEADER DONE
				char *replyDBuf=new char[bytesToRead];
				//ifstream fileptr(file,ios::in|ios::binary);  COMMENTED BY ME FOR SHADOW
				fileptr.seekg(fileOffset); // FOR SEEKING THE FILE
				for(int j=0;j<numPackets;j++)
				{
					pthread_testcancel();
					//cout<<"\n sending "<<j<<"packet"<<endl;
					//cout<<"\n bytes to read"<<bytesToRead<<endl;
					// SINCE THE FILE SIZE IS MORE THAN 512
					// READING THE PART OF THE FILE INTO THE FILE BUFFER
					//fileptr.seekg(bytesRead);
					memset(replyDBuf,'\0',bytesToRead);
					fileptr.read(replyMsg->data,bytesToRead);
				   
					//cout<<" The reply msg buf size is "<<replyBufSize<<endl;
					
					memcpy(&replyDBuf[0],(char *)replyMsg->data,bytesToRead);
					// REPLY BUFFER CONSTRUCTION IS DONE
					// SENDING PAYLOAD
					
					for(int i=0;i<bytesToRead;i++)
					{
						//cout<<"Sending "<<i<<endl;
						//test=send(*cliSock,&replyDBuf[i],1,0);
						//cout<<"\n Send return is "<<test<<endl;
						//cout<<" i is "<<i<<endl;
						//cout<<" test is "<<test<<endl;
						if(send(*cliSock,&replyDBuf[i],1,0)!=1)
						{
							pthread_testcancel();
							cout<<"Error in conncection "<<endl;
							return;
						}
						pthread_testcancel();
					}
					bytesRead=bytesRead+bytesToRead;
					//cout<<" bytes read "<<bytesRead<<endl;
					// SENDING PAYLOAD DONE
					if((bytesRead+bytesToRead)>fileSize)
					{
						bytesToRead=(fileSize-bytesRead);
					}
				}
			   


			}
		}
       
    }

}


/*

THIS FUNCTION HANDLES THE FILE SIZE REQUEST
THIS TAKES THE CLIENT SOCKET DESCRIPTOR , FILE NAME, AND THE FILE NAME LENGTH
AS ITS ARGUMENTS. ITS A VOID FUNCTION AND RETURNS NOTHING

*/

void fszReq(int *cliSock,char *fileName,int nameLen)
{
    char *file= new char[nameLen+1];
    memset(file,'\0',nameLen+1);
    strncpy(file,fileName,nameLen);
    //cout<<" the file in file size is "<<file<<endl;
    struct stat *fileStat=new struct stat();
    if(stat(file,fileStat)<0)
    {
        // THE CANNOt FIND THE REQUESTED FILE
        // SHOULD SEND THE FSZ_FAIL
        cout<<"\n Error : Finding the size of the file "<<endl;
        msg *replyMsg=new msg();
        memset(replyMsg,0,sizeof(msg));
        replyMsg->type=FSZ_FAIL;
        replyMsg->datalen=0;
        // REPLY MSG CONSTRUCTION IS DONE
       
        // REPLY BUFFER CONSTRUCTION BEGINS
        int replyBufSize=sizeof(replyMsg->type)+sizeof(replyMsg->offset)+sizeof(replyMsg->datalen);
        //cout<<" The reply msg buf size is "<<replyBufSize<<endl;
        char *replyBuf=new char[replyBufSize];
        memcpy(&replyBuf[0],(char *)&replyMsg->type,sizeof(replyMsg->type));
        memcpy(&replyBuf[2],(char *)&replyMsg->offset,sizeof(replyMsg->offset));
        memcpy(&replyBuf[6],(char *)&replyMsg->datalen,sizeof(replyMsg->datalen));
        // REPLY BUFFER CONSTRUCTION IS DONE
       
        // SEDING HEADER
        for(int i=0;i<HDR_LEN;i++)
        {
            if(send(*cliSock,&replyBuf[i],1,0)<0)
            {
				pthread_testcancel();
                cout<<"Error in conncection "<<endl;
                return;
            }
            //cout<<" Sending "<< i <<"byte"<<endl;
			pthread_testcancel();
        }
        // SENDING HEADER DONE
       
        // NO NEED TO SEND PAYLOAD SINCE THE ADDRESS IS FAIL
       
    }
    else
    {
        // THE REQUESTED FILE IS FOUND
        //cout<<" The file size is "<<fileStat->st_size<<endl;
        msg *replyMsg=new msg();
        memset(replyMsg,0,sizeof(msg));
        replyMsg->type=FSZ_RPLY;
        replyMsg->data= new char[MAX_DLEN];
        memset(replyMsg->data,'\0',MAX_DLEN);
        sprintf(replyMsg->data,"%d",(int)fileStat->st_size);
        //cout<<" Len of data is "<<strlen(replyMsg->data)<<endl;
        replyMsg->datalen=strlen(replyMsg->data);
       
        // REPLY MSG CONSTRUCTION IS DONE
       
        int replyBufSize=sizeof(replyMsg->type)+sizeof(replyMsg->offset)+sizeof(replyMsg->datalen)+strlen(replyMsg->data);
        //cout<<" The reply msg buf size is "<<replyBufSize<<endl;
        char *replyBuf=new char[replyBufSize];
        memcpy(&replyBuf[0],(char *)&replyMsg->type,sizeof(replyMsg->type));
        memcpy(&replyBuf[2],(char *)&replyMsg->offset,sizeof(replyMsg->offset));
        memcpy(&replyBuf[6],(char *)&replyMsg->datalen,sizeof(replyMsg->datalen));
        memcpy(&replyBuf[10],(char *)replyMsg->data,strlen(replyMsg->data));
        // REPLY BUFFER CONSTRUCTION IS DONE
   
        // SEDING HEADER
        for(int i=0;i<HDR_LEN;i++)
        {
       
            if(send(*cliSock,&replyBuf[i],1,0)<0)
            {
				pthread_testcancel();
                cout<<"Error in conncection "<<endl;
                return;
            }
            //cout<<" Sending "<< i <<"byte"<<endl;
			pthread_testcancel();
        }
        // SENDING HEADER DONE
   
        // SENDING PAYLOAD
        for(int i=HDR_LEN;i<replyBufSize;i++)
        {
            //cout<<"Sending "<<i<<endl;
			pthread_testcancel();
            if(send(*cliSock,&replyBuf[i],1,0)<0)
            {
				pthread_testcancel();
                cout<<"Error in conncection "<<endl;
                return;
            }
        }
        // SENDING PAYLOAD DONE
    }
   
}

/*

THIS FUNCTION HANDLES ALL THE ADDRESS REQUEST FROM THE CLIENT
THIS TAKES THE CLIENT SOCKET DESCRIPTOR HOST NAME AND HOST NAME LENGTH
AS ITS ARGUMENTS ITS A VOID FUNCTION AND RETURNS NOTHING

*/
void adrReq(int *cliSock,char *hostName,int nameLen)
{
    char *rhost= new char[nameLen+1];
    memset(rhost,'\0',nameLen+1);
    strncpy(rhost,hostName,nameLen);
    //cout<<" the rhost is "<<rhost<<endl;
    struct hostent *rHostIp;
   
    // GET HOST BY NAME IS CALLED TO GET THE IP ADDRES OF THE REMOTE HOST
    rHostIp=gethostbyname(rhost);
    if(rHostIp!=NULL)
    {
        // HOST NAME EXIST
        const in_addr* rHostIpAddress = (in_addr*)rHostIp->h_addr_list[0];
        //cout<<" the ip is "<<inet_ntoa(*rHostIpAddress) <<endl;
        //cout<<" the len of ip address is "<<strlen(inet_ntoa(*rHostIpAddress))<<endl;
        char *rIp=new char[strlen(inet_ntoa(*rHostIpAddress))+1];
        memset(rIp,'\0',strlen(inet_ntoa(*rHostIpAddress))+1);
        strncpy(rIp,inet_ntoa(*rHostIpAddress),strlen(inet_ntoa(*rHostIpAddress)));
        // cout<<" strlen(rip) "<<strlen(rIp)<<endl;
        //cout<<" rip is "<<rIp<<endl;
   
        // REPLY MSG CONSTRCUTION BEGINS
        msg *replyMsg=new msg();
        memset(replyMsg,0,sizeof(msg));
        replyMsg->type=ADR_RPLY;
        replyMsg->datalen=strlen(rIp);
        replyMsg->data=new char[strlen(rIp)+1];
        memset(replyMsg->data,'\0',strlen(rIp)+1);
        strncpy(replyMsg->data,rIp,strlen(rIp));
        // REPLY MSG CONSTRUCTION IS DONE
   
        // REPLY BUFFER CONSTRUCTION BEGINS
        int replyBufSize=sizeof(replyMsg->type)+sizeof(replyMsg->offset)+sizeof(replyMsg->datalen)+strlen(rIp);
        // cout<<" The reply msg buf size is "<<replyBufSize<<endl;
        char *replyBuf=new char[replyBufSize];
        memcpy(&replyBuf[0],(char *)&replyMsg->type,sizeof(replyMsg->type));
        memcpy(&replyBuf[2],(char *)&replyMsg->offset,sizeof(replyMsg->offset));
        memcpy(&replyBuf[6],(char *)&replyMsg->datalen,sizeof(replyMsg->datalen));
        memcpy(&replyBuf[10],(char *)replyMsg->data,strlen(rIp));
        // REPLY BUFFER CONSTRUCTION IS DONE
   
        // SEDING HEADER
        for(int i=0;i<HDR_LEN;i++)
        {
            if(send(*cliSock,&replyBuf[i],1,0)<0)
            {
				pthread_testcancel();
                cout<<"Error in conncection "<<endl;
                return;
            }
			pthread_testcancel();
            //cout<<" Sending "<< i <<"byte"<<endl;
        }
        // SENDING HEADER DONE
   
        // SENDING PAYLOAD
       // cout<<"\n msg sie is "<<strlen(rIp)<<endl;
       for(int i=HDR_LEN;i<replyBufSize;i++)
        {
            //cout<<"Sending "<<i<<endl;
            if(send(*cliSock,&replyBuf[i],1,0)<0)
            {
				pthread_testcancel();
                cout<<"Error in conncection "<<endl;
                return;
            }
			pthread_testcancel();
        }
        // SENDING PAYLOAD DONE
    }
    else
    {
        // THE HOST NAME DOES NOT EXIST
        // REPLYING WITH ADR_FAIL
       
        // REPLY MSG CONSTRCUTION BEGINS
        msg *replyMsg=new msg();
        memset(replyMsg,0,sizeof(msg));
        replyMsg->type=ADR_FAIL;
        replyMsg->datalen=0;
        // REPLY MSG CONSTRUCTION IS DONE
       
        // REPLY BUFFER CONSTRUCTION BEGINS
        int replyBufSize=sizeof(replyMsg->type)+sizeof(replyMsg->offset)+sizeof(replyMsg->datalen);
        //cout<<" The reply msg buf size is "<<replyBufSize<<endl;
        char *replyBuf=new char[replyBufSize];
        memcpy(&replyBuf[0],(char *)&replyMsg->type,sizeof(replyMsg->type));
        memcpy(&replyBuf[2],(char *)&replyMsg->offset,sizeof(replyMsg->offset));
        memcpy(&replyBuf[6],(char *)&replyMsg->datalen,sizeof(replyMsg->datalen));
        // REPLY BUFFER CONSTRUCTION IS DONE
       
        // SEDING HEADER
        for(int i=0;i<HDR_LEN;i++)
        {
            if(send(*cliSock,&replyBuf[i],1,0)<0)
            {
				pthread_testcancel();
                cout<<"Error in conncection "<<endl;
                return;
            }
            //cout<<" Sending "<< i <<"byte"<<endl;
			pthread_testcancel();
        }
        // SENDING HEADER DONE
       
        // NO NEED TO SEND PAYLOAD SINCE THE ADDRESS IS FAIL
       
    }
}

// FUCNTION REMOVED FROM HERE- ALL FAIL

void *handleClientRequest(void *cliSockFd)
{
    int *cliSock=(int *)cliSockFd;
    int bytesRecvd=0;
    pthread_mutex_lock(consoleLock);
    //cout<<" Inside hadle Client Request function "<<(*cliSock)<<endl;
    pthread_mutex_unlock(consoleLock);
    // READING HEADER
    char *headBuf=new char[HDR_LEN];
    for(int i=0;i<HDR_LEN;i++)
    {
        if(recv(*cliSock,&headBuf[i],1,0)<=0)
        {
				pthread_testcancel();
                cout<<"Error in conncection "<<endl;
				cout<<"Header less than 10 bytes"<<endl;
				allFail(cliSock);
                close(*cliSock);
                pthread_exit(NULL);
        }
        bytesRecvd++;
		pthread_testcancel();
        //cout<<"\n recvied "<<i<<"bytes"<<endl;
    }
    // READING HEADER DONE
    msg *receivedMsg=new msg();
    memcpy(&receivedMsg->type,(uint16_t *)&headBuf[0],sizeof(uint16_t));
    memcpy(&receivedMsg->offset,(uint32_t *)&headBuf[2],sizeof(uint32_t));
    memcpy(&receivedMsg->datalen,(uint32_t *)&headBuf[6],sizeof(uint32_t));
    //cout<<"\n Received header is "<<endl;
    //cout<<" 1 "<<receivedMsg->type<<endl;
    //cout<<" 2 "<<receivedMsg->offset<<endl;
    //cout<<" 3 "<<ntohl(receivedMsg->datalen)<<endl;
    // HEADER RECEIVING DONE
   
    // RECEIVING PAYLOAD
    receivedMsg->data= new char[ntohl(receivedMsg->datalen)+1];
    memset(receivedMsg->data,'\0',ntohl(receivedMsg->datalen)+1);
    char *payload=new char[(ntohl(receivedMsg->datalen))];
    for(unsigned int i=0;i<(ntohl(receivedMsg->datalen));i++)
    {
        if(recv(*cliSock,&payload[i],1,0)<0)
        {
			pthread_testcancel();
            cout<<"\n Error in connection "<<endl;
            close(*cliSock);
            pthread_exit(NULL);
        }
        bytesRecvd++;
		pthread_testcancel();
        //cout<<"\n recvied "<<i<<"bytes"<<endl;
    }
    memcpy(receivedMsg->data,(char *)&payload[0],(ntohl(receivedMsg->datalen)));
   // cout<<"\n the pay load is "<<receivedMsg->data<<endl;
    //cout<<"\n len of payload is "<<strlen(receivedMsg->data)<<endl;
    // RECEVING PAYLOAD DONE
    if(mset==1)
    {
        // -m OPTION IS SET SO MUST PRINT VERBOSE INFORMATION
        int peername=getpeername(*cliSock,(struct sockaddr *)&peer_addr,&peer_addrLen);
        if(peername<0)
        {
            cout<<"Error: getpeername failed "<<endl;
        }
        cout<<endl;
        cout<<"Received "<<bytesRecvd<<" bytes from "<<inet_ntoa(peer_addr.sin_addr)<<endl;
        printf("MessageType: 0x%02x\n",receivedMsg->type);
        printf("Offset: 0x%08x\n",receivedMsg->offset);
        printf("DataLength: 0x%08x\n",receivedMsg->datalen);
        cout<<endl;
    }
    // CODE SEGMENT TO IMPLEMENT THE DELAY IN THE RESPONSES
	for(int i=0;i<delay;i++)
	{
		sleep(1);
	}
	// CHECKING THE REQUEST TYPE
    if(receivedMsg->type==ADR_REQ)
    {
        // THE REQUEST TYPE IS ADR_REQ
        adrReq(cliSock,receivedMsg->data,strlen(receivedMsg->data));
        close(*cliSock);
    }
    else if(receivedMsg->type==FSZ_REQ)
    {
        // THE REQUEST TYPE IS FSZ_REQ
        fszReq(cliSock,receivedMsg->data,strlen(receivedMsg->data));
        close(*cliSock);
    }
    else if(receivedMsg->type==GET_REQ)
    {
        // THE REQUEST TYPE IS GET_REQ
        getReq(cliSock,receivedMsg->data,strlen(receivedMsg->data),ntohl(receivedMsg->offset));
        close(*cliSock);
    }
    else
    {
        //  MAL-FORMED REQUEST SEND ALL FAIL
		allFail(cliSock);
    }
    pthread_exit(NULL);
}

/*  A LISTENIGN THREAD WHICH CREATES A SOCKETS AND LISTENS
AND ACCEPTS FOR INCOMING CONNECTION */

void *listenThreadFn(void *arg)
{

    
	pthread_t *timer=new pthread_t();
    if((lisSock=socket(AF_INET,SOCK_STREAM,0))<0)
    {
        cout<<"Error : Server socket could not be created"<<endl;
        exit(1);
    }
    setsockopt(lisSock,SOL_SOCKET,SO_REUSEADDR,(void *)(&resueAddr),sizeof(int)); // SETTING SOCKET OPTION TO RESUE PORT NUMBERS
    memset(&serv_addr,0,sizeof(struct sockaddr_in));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(port);
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
   
    //SERVER SOCKET IS BINDED
    if(bind(lisSock,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0)
    {
        cout<<"Error : Binding failed "<<endl;
        exit(1);
    }

    pthread_create(timer,NULL,timerFn,NULL);
	pthread_mutex_lock(threadRefLok);
	threadRefs.push_back(*timer);
	pthread_mutex_unlock(threadRefLok);
    // SERVER SOCKET IS LISTENING
    if(listen(lisSock,CON_MAX)==-1)
	{
		cout<<"Error in listening "<<endl;
		exit(1);
	}
    //int cliSock;
    socklen_t cliAddrLen;
    struct sockaddr_in cliAddr;
    cliAddrLen=sizeof(cliAddr);
    for(;;)
    {
        pthread_mutex_lock(consoleLock);
        //cout<<"Server started waiting to accept connection "<<endl;
        pthread_mutex_unlock(consoleLock);
		pthread_t *worker=new pthread_t();     // WORKER THREAD IS CREATED FOR EACH ACCEPTED CONNECTION
		int *cliSock= new int();
        *cliSock=accept(lisSock,(struct sockaddr *)(&cliAddr),&cliAddrLen);
        if(cliSock < 0)
        {
			pthread_testcancel();
            pthread_mutex_lock(consoleLock);
            cout<<"Error : Could not accpet connection from client"<<endl;
            pthread_mutex_unlock(consoleLock);
        }
		pthread_testcancel();
        pthread_create(worker,NULL,handleClientRequest,cliSock);
		pthread_mutex_lock(threadRefLok);
		threadRefs.push_back(*worker);
		pthread_mutex_unlock(threadRefLok);

    }

}


/*
 MAIN FUNCTION OF THIS PROGRRAM.
 THE PROGRAM EXECUTION STARTS HERE
 */
int main(int argc,char *argv[])
{
   
    lis= new pthread_t();
    pthread_t *sigHandlerThread=new pthread_t();
    consoleLock= new pthread_mutex_t();
    pthread_mutex_init(consoleLock,NULL);
    numWorkersLok=new pthread_mutex_t();
  
   
    threadRefLok= new pthread_mutex_t();
    pthread_mutex_init(threadRefLok,NULL);
   
    // INTIALIZING THE SIGNAL SET AND ADDING SINGAL TO THE SIGNAL SET
    sigemptyset (&signals);
    sigaddset (&signals, SIGINT);
    sigaddset (&signals, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &signals, NULL);
    pthread_create(sigHandlerThread,NULL,signalHandler,NULL);
    pthread_mutex_lock(threadRefLok);
    threadRefs.push_back(*sigHandlerThread);
    pthread_mutex_unlock(threadRefLok);


    //cout<<"The count command line arguments are "<<argc<<endl;
    if ((argc)<2)
    {
        cout<<" malformed command "<<endl;
        exit(1);
    }

    //  CODE SEGEMENT TO PARSE THE ARGUEMENTS THAT ARE PASSED AT THE COMMAND LINE
   
    for (int i=1;i<argc ;i++ )
    {
       
        //cout<<"\t"<<argv[i]<<endl;
        if(!(strncmp(argv[i],"-t",2))) // IF -t IS SPECIFIED THERE MUST BE A 3RD COMMAND LINE PARAMETER
        {
			if(argc>2)
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
					cout<<"bad timeout argument"<<endl;
					exit(1);
				}
				else
				{
                autoShut=atoi(argv[i+1]);
                //cout<<"Auto shutdown set "<<autoShut<<endl;
				}
			}
			else
			{
				cout<<" malformed command"<<endl;
				exit(1);

			}
        }
        if(!(strncmp(argv[i],"-d",2)))
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
				cout<<"bad delay argument"<<endl;
				exit(1);
			}
			else
			{
				delay=atoi(argv[i+1]);
			}
            //cout<<"\n Auto delay set "<<delay<<endl;
        }
        if(!(strncmp(argv[i],"-m",2)))
        {


            mset=1;
            //cout<<"\n -m option set "<<endl;
        }
        port=atoi(argv[argc-1]);
        if(port<=1024 && port>65536)
        {
          cout<<"Bad port number "<<endl;
        }
    }
    pthread_create(lis,NULL,listenThreadFn,NULL);
    pthread_mutex_lock(threadRefLok);
    threadRefs.push_back(*lis);
    pthread_mutex_unlock(threadRefLok);
	//cout<<"\n ThreadRefs size "<<(int)threadRefs.size()<<endl;
    //cout<< "The sevrer is running in port number "<<port<<endl;
	for(int i = 0; i < (int)threadRefs.size() ; i++)
	{
		//cout<<"Main server thread is waiting for its child thread to complete"<<endl;
		pthread_join(threadRefs[i], NULL);
		
	}
    cout<<endl;
}
