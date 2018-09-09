#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <event.h>
#include <string.h>
#include <string>
#include "public.h"
#include <json/json.h>
#include "control.h"
#include "pthread.h"
extern sem_t sem;
void* pthread_run(void*);
void client_cb(int fd,short event,void *arg);
void sock_pair_1_cb(int fd,short event,void *arg);

extern Control control_sever;
Pthread::Pthread(int sock_fd)
{
	_sock_fd = sock_fd;
	_base = event_base_new();

	//启动线程
	if(-1 == pthread_create(&_pthread, NULL, pthread_run, (void*)this))
	{
		cerr<<"pthread_creat fail;errno"<<errno<<endl;
		throw ;
	}
}

void* pthread_run(void *arg)
{
	Pthread* pth = (Pthread*)arg;
	//将sock_pair_1加入到libevent  sock_pair_1_cb()
	struct event* sock_event = event_new(pth->_base, pth->_sock_fd, EV_READ|EV_PERSIST, sock_pair_1_cb, pth);
	
	//pth->_event_map.insert(make_pair(pth->_sock_fd, sock_event));//将事件和fd加入map表中
	if(sock_event != NULL)
	{	
		event_add(sock_event, NULL);
	}
	event_base_dispatch(pth->_base);
}

void sock_pair_1_cb(int fd,short event,void *arg)
{
	//信号控制
	
	Pthread* pth = (Pthread*)arg;
	//recv -> clien_fd
	char recvbuff[128] = {0};
	if(0 > recv(fd, recvbuff, 127, 0))
	{
		cerr<<"recv sock_pair fail;errno:"<<errno<<endl;
		return ;	
	}
	int cli_fd = atoi(recvbuff);
	//将client_fd加入到libevent     client_cb()
	
	struct event* cli_event = event_new(pth->_base, cli_fd, EV_READ|EV_PERSIST, client_cb, pth);
	if(cli_event != NULL)
	{
		pth->_event_map.insert(make_pair(cli_fd, cli_event));
		event_add(cli_event, NULL);
	}
	//给主线程回复当前监听的客户端数量
	int size = pth->_event_map.size();
	char sendbuff[128] = {0};
	sprintf(sendbuff, "%d", size);
	if(-1 == send(fd, sendbuff, strlen(sendbuff), 0))
	{
		cerr<<"send fail;errno"<<endl;
		return;
	}
//cout<<"管道："<<fd<<"  "<<"cli_fd::"<<cli_fd<<"   size::"<<size<<endl;
	sem_post(&sem);
}

void client_cb(int fd,short event,void *arg)
{
	//recv  ->buff
	char recvbuff[128] = {0};
	if(0 > recv(fd, recvbuff, 127, 0))
	{
		cerr<<"client recv fail;errno:"<<errno<<endl;
		return;
	}
	//将buff发给control
	control_sever.process(fd,recvbuff);

	Json::Value root;
	Json::Reader read;
	if(-1 == read.parse(recvbuff, root))
	{
		cerr<<"client_cb read.parse fail;errno:"<<endl;
		return ;
	}
	if(MSG_TYPE_EXIT == root["reason_type"].asInt())
	{
		Pthread* p = (Pthread*)arg;
		//
		map<int,struct event*>::iterator it = p->_event_map.find(fd);
		event_del(it->second);
		p->_event_map.erase(fd);
	}
}


