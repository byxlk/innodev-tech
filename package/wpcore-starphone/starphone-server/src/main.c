#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "common.h"
#include "network_server.h"



int main(int argc, char **argv) 
{	
    starphone_server *sps;

    _DEBUG("Compile Time:%s",__TIME__);
    _DEBUG("Perper memory init ...");
    
    sps = (starphone_server *) malloc(sizeof(starphone_server));
    if(NULL ==  sps)
    {
            _ERROR("malloc starphone_server struct faild.");
            exit(-1);
    }

    _DEBUG("Start udp broadcast...\n");
    XW_pthread_create(&sps->thread_id_upd,
                                       (void*)XW_pthread_udp_broadcast, NULL);
	
    //_DEBUG("Start tcp server...\n");
    //_pthread_create(&id, (void*)tcp_server, NULL);
	
    //pthread_join(sps->thread_id_tcp, NULL);
    pthread_join(sps->thread_id_upd, NULL);
    
    return 0;
}