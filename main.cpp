//#include<iostream>
#include <stdlib.h>
#include"tcpsever.h"
//using namespace std;


int main(int argc,char *argv[])
{
	cout<<"chat run "<<endl;
	if(argc < 4)
	{
		cout<<"error"<<endl;
		return 0;
	}

	//分离参数
	char* ip = argv[1];
	short port = atoi(argv[2]);
	int pth_num = atoi(argv[3]);

	Tcpsever sever(ip, port, pth_num);
	sever.run();
}


