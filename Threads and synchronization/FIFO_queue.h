#ifndef FIFO_QUEUE_H
#define FIFO_QUEUE_H
#include <sys/time.h>
struct node{
	struct node* next;
	int* fd;
	struct timeval arival_toto;
	
};
typedef struct node node_t;
 
void enqueue(int* request_fd,struct timeval arival_time);
int* dequeue(struct timeval* time);
void drop_head();
int random_drop(int maxSize);
struct timeval get_head_time();

#endif 
