//#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <event.h>
#include <errno.h>
#include <semaphore.h>
#include "tcpsever.h"
//using namespace std;
sem_t sem;
void listen_cb(int fd,short event,void *arg);
void sock_pair_cb(int fd,short event,void *arg);

Tcpsever::Tcpsever(char* ip, short port, int pth_num)
{
	///创建服务器
	int _listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == _listen_fd)
	{
		cerr<<"socket fail;error:"<<errno<<endl;
		throw ;	
	}
	
	struct sockaddr_in ser;
	ser.sin_family = AF_INET;
	ser.sin_port = htons(port);
	ser.sin_addr.s_addr = inet_addr(ip);
	
	if(-1 == bind(_listen_fd, (struct sockaddr*)&ser, sizeof(ser)))
	{
		cerr<<"_listen_fd bind fail;errno:"<<errno<<endl;
		throw;
	}

	if(-1 == listen(_listen_fd, 20))
	{
		cerr<<"listen fail;errno:"<<errno<<endl;
		throw;
	}
	//_listen_fd;
	_pth_num = pth_num;

	//给libevent申请空间
	_base = event_base_new();

	//创建事件，绑定监听套接子的回调函数(listen_cb)
	struct event* listen_event = event_new(_base, _listen_fd, EV_READ|
			EV_PERSIST, listen_cb, this);
	if(NULL == listen_event)
	{
		cerr<<"event new fail;error"<<errno<<endl;
		throw ;
	}

	event_add(listen_event, NULL);
}

void Tcpsever::run()
{
	sem_init(&sem,0,0);
	//申请socketpair（函数自查）
	get_socket_pair();

	//创建线程
	get_pthread();

	//规定  int arr[2]  arr[0]<=>主线程占用   arr[1]<=>子线程占用

	//为主线程的socktpair创建事件，绑定回调函数（sock_pair_cb）	
	/*int i;
	for(i=0; i<_pth_num; i++)
	{
		struct event* sock_event = event_new(_base, _socket_pair[i].arr[0], 
				EV_READ|EV_PERSIST, sock_pair_cb, this);
		event_add(sock_event, NULL);
	}*/
	
	event_base_dispatch(_base);
}

void Tcpsever::get_socket_pair()
{
	int i;
	for(i = 0;i < _pth_num;i++ )
	{
		//申请双向管道
		int arr[2];
		if(-1 == socketpair(AF_UNIX, SOCK_STREAM, 0, arr))
		{
			cerr<<"socketpair fail;error:"<<errno<<endl;
			return;
		}
			
		//将双向管道加入到_sock_pair.push_back();
		_socket_pair.push_back(Arr(arr));

		//_pth_work_num.push_buck(makepair(arr[0],0));

		struct event* sock_event = event_new(_base, arr[0],	EV_READ|EV_PERSIST, sock_pair_cb, this);
		if(sock_event != NULL)
		{
			event_add(sock_event, NULL);
			_pth_work_num.insert(make_pair(arr[0], 0));
		}
		
	}
}

void Tcpsever::get_pthread()
{
	//开辟线程
	int i;
	for(i = 0; i< _pth_num; i++)
	{
		_pthread.push_back(new Pthread(_socket_pair[i].arr[1]));
	}
}


void listen_cb(int fd,short event,void *arg)
{
	//信号控制
	Tcpsever* ser = (Tcpsever*)arg;
	//接受用户链接
	struct sockaddr_in cli;
	socklen_t len = sizeof(cli);
	int cli_fd = accept(fd, (struct sockaddr*)&cli, &len);
	if(-1 == cli_fd)
	{
		cerr<<"accept fail;error:"<<errno<<endl;
		return ;
	}

	//查找当前监听数量最少的子线程
	int min_fd;
	map<int, int>::iterator it = (ser->_pth_work_num).begin();
	//int i = it->second;
	int i= it->second;
	min_fd = it->first;
	for(; it != ser->_pth_work_num.end(); ++it)
	{
		if(it->second < i)
		{
			i = it->second;
			min_fd = it->first;
		}
	}
//cout<<"管道：："<<min_fd<<" "<<"fd::"<<cli_fd<<endl;
	//将客户端套接子通过socktpair发给子线程
	char sendbuff[128] = {0};
	sprintf(sendbuff, "%d", cli_fd);
	
	send(min_fd, sendbuff, strlen(sendbuff), 0);
	sem_wait(&sem);
}


void sock_pair_cb(int fd,short event,void *arg)
{
	Tcpsever* ser = (Tcpsever*)arg;
	//读取管道内容
	char recvbuff[128] = {0};
	if(0 > recv(fd, recvbuff, 127, 0))
	{
		cerr<<"recv fail;errno:"<<errno<<endl;
		return;
	}
	//更新到map表_pth_work_num  ----->fd
	//map<int, int>::iterator it = ser->_pth_work_num.find(fd);
	//it->second = atoi(recvbuff):
	ser->_pth_work_num[fd] = atoi(recvbuff);
}


Tcpsever::~Tcpsever()
{

}
