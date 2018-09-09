#include "mysql.h"
#include "get_list.h"
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

void View_get_list::process(int fd,char* json)
{
	_fd = fd;
	Json::Value root;
	Json::Reader read;
	if(-1 == read.parse(json, root))
	{
		cerr<<"Register Json.parse fail;errno:"<<endl;
	}

	char cmd[100] = "SELECT * FROM user;";

	if(mysql_real_query(Mysql_sever._mpcon, cmd, strlen(cmd)))
	{
		cerr<<"mysql_real_query fail;errno:"<<errno<<endl;
		return;
	}
}

void View_get_list::response()
{
	char send_buff[128] = {0};
	Mysql_sever._mp_res = mysql_store_result(Mysql_sever._mpcon);
	while(Mysql_sever._mp_row = mysql_fetch_row(Mysql_sever._mp_res))
	{
		sprintf(send_buff+strlen(send_buff), "%s", Mysql_sever._mp_row[0]);
		sprintf(send_buff+strlen(send_buff), "\n");
	}

	Json::Value val;
	val["back_new"] = send_buff;
	if(-1 == send(_fd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str()), 0))
	{
		cerr<<"Get list send fail;errno:"<<errno<<endl;
		return ;
	}
}
