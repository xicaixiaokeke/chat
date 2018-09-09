#ifndef TCPSEVER_H
#define TCPSEVER_H

#include <map>
#include <vector>
#include "pthread.h"
#include <iostream>
using namespace std;
class Arr
{
	public:
		int arr[2];
		Arr(int brr[2])
		{
			arr[0] = brr[0];
			arr[1] = brr[1];
		}
};

class Tcpsever
{
	public:
		Tcpsever(char* ip, short port, int pth_num);
		~Tcpsever();
		void run();
	private:
		//class Arr;
		int _listen_fd;//监听套接字
		int _pth_num;//启动的线程的个数
		struct event_base *_base;//libevent

		vector<Arr> _socket_pair;//socket pair vector
		vector<Pthread*> _pthread;//pthread vector
		map<int,int> _pth_work_num;//用于和子线程交流的fd+对应子线程监听的个数
		
		void get_socket_pair();
		void get_pthread();

		friend void listen_cb(int fd,short event,void* arg);
		friend void sock_pair_cb(int fd,short event,void* arg);
};

#endif 
