#include "mysql.h"
#include "exit.h"
#include "pthread.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <json/json.h>
#include <string.h>
#include <string>
#include <errno.h>
#include <iostream>
using namespace std;
extern Mysql Mysql_sever;
void View_exit::process(int fd,char* json)
{
	_fd = fd;
	Json::Value root;
	Json::Reader read;
	if(-1 == read.parse(json, root))
	{
		cerr<<"Register Json.parse fail;errno:"<<endl;
	}

	char cmd[100] = "SELECT * FROM unline WHERE fd='";
	sprintf(cmd+strlen(cmd), "%d", _fd);
	sprintf(cmd+strlen(cmd), "%s", "';");

	if(mysql_real_query(Mysql_sever._mpcon, cmd, strlen(cmd)))
	{
		cerr<<"mysql_real_query fail;errno:"<<errno<<endl;
		return;
	}
	Mysql_sever._mp_res = mysql_store_result(Mysql_sever._mpcon);
	if(Mysql_sever._mp_row = mysql_fetch_row(Mysql_sever._mp_res))
	{
		char cmd[128] = "DELETE FROM unline where fd='";
		sprintf(cmd+strlen(cmd), "%d", _fd);
		sprintf(cmd+strlen(cmd), "%s", "';");

		if(mysql_real_query(Mysql_sever._mpcon, cmd, strlen(cmd)))
		{
			cerr<<"mysql_real_query fail;errno:"<<errno<<endl;
			return;
		}
	}
}

void View_exit::response()
{	
}
