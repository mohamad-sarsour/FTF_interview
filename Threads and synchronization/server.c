#include "segel.h"
#include "request.h"
#include "FIFO_queue.h"

// global variables 
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var=PTHREAD_COND_INITIALIZER;

int queue_size=0;
int QmaxSize=0;
int currently_handled=0;


void getargs(int *port, int argc, char *argv[])
{
    if (argc < 2) {
	printf("sdsmd");
	fprintf(stderr, "Usage: %s <port>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
}

void* thread_function(void* arg)
{
	stat_thread* thread_st = (stat_thread*)arg;
	int* fd_request;
	struct timeval time;
	while(1){
		pthread_mutex_lock(&mutex);
		while((fd_request=dequeue(&time))==NULL){
			pthread_cond_wait(&condition_var,&mutex);
		}
		queue_size--;
		currently_handled++;
		if(fd_request != NULL){
			pthread_mutex_unlock(&mutex);
			requestHandle(*fd_request,thread_st,time);
			pthread_mutex_lock(&mutex);
			currently_handled--;
			if(currently_handled+queue_size==QmaxSize||currently_handled+queue_size==QmaxSize-1 ){
				pthread_cond_broadcast(&condition_var);
			}		
			close(*fd_request);			
			free(fd_request);	
			pthread_mutex_unlock(&mutex);	
		}
	}
}

void handle_overload(const char* policy){

	if(strcmp(policy,"block")==0){
		pthread_mutex_lock(&mutex);
		while(queue_size+currently_handled==QmaxSize){
			pthread_cond_wait(&condition_var,&mutex);
		}
		pthread_mutex_unlock(&mutex);
	}
	else if(strcmp(policy,"dh")==0){
		pthread_mutex_lock(&mutex);
		int* tmp=dequeue(NULL);
		queue_size--;
		close(*tmp);
		free(tmp);
		pthread_mutex_unlock(&mutex);
	}
	else if(strcmp(policy,"random")==0){
		pthread_mutex_lock(&mutex);
		if(queue_size+currently_handled==QmaxSize){
		random_drop(queue_size);
		if(queue_size%4==0){
			queue_size=3*(queue_size/4);
		}
		else{
			queue_size=queue_size-(queue_size/4) +1;
		    }
	}
	pthread_mutex_unlock(&mutex);
	}
	return;
}

void Create_thread_pool(stat_thread** thread_pool,int poolSize){
	
	for(int i=0;i<poolSize;++i){	
		thread_pool[i]= malloc(sizeof(stat_thread));
		thread_pool[i]->thread_id=i;
		thread_pool[i]->dynam_count=0;
		thread_pool[i]->static_count=0;
		thread_pool[i]->tot_req_count=0;	
		pthread_create(&thread_pool[i]->thread,NULL,thread_function,thread_pool[i]);
	}
}

int main(int argc, char *argv[])
{	
        int listenfd, connfd, port, clientlen;
    	struct sockaddr_in clientaddr;

	getargs(&port, argc, argv);
	int poolSize= atoi(argv[2]);
	QmaxSize= atoi(argv[3]);
    	
	stat_thread** thread_pool=(stat_thread**)malloc(sizeof(stat_thread*)*poolSize);
	Create_thread_pool(thread_pool,poolSize);

        listenfd = Open_listenfd(port);
        clientlen = sizeof(clientaddr);  
        while (1){

		connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
		struct timeval time;
	        gettimeofday(&time,NULL);
		
		int* fdConnec = (int*)malloc(sizeof(int));
		*fdConnec = connfd;
		pthread_mutex_lock(&mutex);	
		if(currently_handled==QmaxSize && !(strcmp(argv[4],"block")==0)){
			close(connfd);
			free(fdConnec);	
			pthread_mutex_unlock(&mutex);
			continue;
		}
		else if(queue_size+currently_handled==QmaxSize){
			if(strcmp(argv[4],"dt")==0){
				Close(*fdConnec);
				free(fdConnec);
				pthread_mutex_unlock(&mutex);
				continue;
			}
			else{
				pthread_mutex_unlock(&mutex);
				handle_overload(argv[4]);
			    }
		 }
		enqueue(fdConnec,time);
		if(++queue_size+currently_handled==1){
			pthread_cond_broadcast(&condition_var);
		}
		pthread_mutex_unlock(&mutex);
    	  }
}
