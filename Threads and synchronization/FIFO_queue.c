#include "FIFO_queue.h"
#include <stdlib.h>

node_t* head=NULL;
node_t* tail=NULL;

struct timeval get_head_time(){
	return head->arival_toto;
}

void enqueue(int* request_fd,struct timeval arival_time){

	node_t* newnode =(node_t*)malloc(sizeof(node_t));
        newnode->fd =request_fd;
	newnode->arival_toto=arival_time;
	newnode->next = NULL;
	if(head==NULL){   //len=0
		head=newnode;
		tail=newnode;
		return;
	}
	if(tail==head && tail!=NULL){ // len=1
		tail=newnode;
		head->next=newnode;///head->next=tail;
		return;
	}else{//len>1
		tail->next=newnode;
	}
	tail=newnode;
}

int* dequeue(struct timeval* time){

	if(head==NULL){
		return NULL;
	}
	int* result=(int*)malloc(sizeof(int));
	if(time!=NULL){
		*time=head->arival_toto;
	}
	*result=*head->fd;
	node_t* tmp=head;
	head=head->next;
	if(head==NULL){
		tail=NULL;
	}
	free(tmp);
	return result;
}


void drop_head(){
	node_t* tmp=head;
	head=tmp->next;
	free(tmp->fd);
	free(tmp);
}


int Randoms(int lower, int upper)
{
        int num = (rand() %(upper - lower + 1)) + lower;
        return num;
}
  

int random_drop(int qmaxSize){
	int to_be_deleted=0;
if(qmaxSize%4==0){
 to_be_deleted=qmaxSize/4;
}
else{
to_be_deleted=1+qmaxSize/4;
}
if(!head){
	return 0;}
for(int higher= qmaxSize;higher>qmaxSize-to_be_deleted;higher--){
int node_to_delete= Randoms(0, higher)-1;

node_t* tmp=head;
node_t*	tmp_preior=tmp;
	tmp=tmp->next;
	if(!tmp){
	return 0;}
	while(node_to_delete!=0&&tmp->next){
		tmp_preior=tmp_preior->next;
		tmp=tmp->next;
			node_to_delete--;
	}
if(tmp){
	if(tmp==tail){
		tmp_preior->next=tmp->next;
		close(tmp->fd);
		free(tmp->fd);
	free(tmp);
	tail=tmp_preior;
		}
		else{
tmp_preior->next=tmp->next;
	close(tmp->fd);		
	free(tmp->fd);
	free(tmp);}}
}
return 1;
}
