#include "common.h"
#include <signal.h>
#include <pthread.h>
#include <vector>

#define CON_MAX 25


int delay=0;
int autoShut=60;
int port=-1;
int mset=0;
struct sockaddr_in serv_addr,peer_addr;
int lisSock, resueAddr=1;
socklen_t serv_addrLen=sizeof(serv_addr);
socklen_t peer_addrLen=sizeof(peer_addr);

pthread_mutex_t *consoleLock;
pthread_t *lis;

pthread_mutex_t *threadRefLok;
vector<pthread_t> threadRefs;

int numWorkers=0;
pthread_mutex_t *numWorkersLok;
sigset_t signals;
