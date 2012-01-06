#include "common.h"

/*

 THIS IS FUNCTION FOR WHICH A NEW THREAD IS CREATED .
 THIS HANDLES THE CLIENT REQUESTS
 TAKES THE CLIENT SOCKET DESCRIPTOR AS THE ARGUMENT
*/

void allFail(int *cliSock)
{
       
	// REPLY MSG CONSTRCUTION BEGINS
    msg *replyMsg=new msg();
    memset(replyMsg,0,sizeof(msg));
    replyMsg->type=ALL_FAIL;
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
