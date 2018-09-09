#include "mysql.h"
#include "talk_to_one.h"
#include "pthread.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <json/json.h>
#include <string.h>
#include <string>
#include <errno.h>
#include <iostream>
using namespace std;
extern Mysql Mysql_sever;

void View_talk_to_one::process(int fd,char* json)
{
	_fd = fd;
	char new_buff[1024] = {0};

	Json::Value root;
	Json::Reader read;
	if(-1 == read.parse(json, root))
	{
		cerr<<"Register Json.parse fail;errno:"<<endl;
	}
	//保存name
	const char* a = root["name"].asString().c_str();
	char name[128] = {0};
	sprintf(name, "%s", a);
//
	char cmd3[100] = "SELECT * FROM user WHERE name='";
	sprintf(cmd3+strlen(cmd3), "%s", name);
	sprintf(cmd3+strlen(cmd3), "%s", "';");
	if(mysql_real_query(Mysql_sever._mpcon, cmd3, strlen(cmd3)))
	{
		cerr<<"mysql_real_query fail;errno"<<errno<<endl;
		return;
	}
	Mysql_sever._mp_res = mysql_store_result(Mysql_sever._mpcon);
	if(Mysql_sever._mp_row = mysql_fetch_row(Mysql_sever._mp_res))
	{//查找接受消息的人是否存在
		char cmd2[100] = "SELECT * FROM unline WHERE fd='";
		sprintf(cmd2+strlen(cmd2), "%d", _fd);
		sprintf(cmd2+strlen(cmd2), "%s", "';");
		
		if(mysql_real_query(Mysql_sever._mpcon, cmd2, strlen(cmd2)))
		{
			cerr<<"mysql_real_query fail;errno:"<<errno<<endl;
			return;
		}
		Mysql_sever._mp_res = mysql_store_result(Mysql_sever._mpcon);
		if(Mysql_sever._mp_row = mysql_fetch_row(Mysql_sever._mp_res))
		{//将发消息者写进消息里面
			sprintf(new_buff, "%s", Mysql_sever._mp_row[1]);
		}
//
		sprintf(new_buff+strlen(new_buff), "%s", "::");
//
		char cmd[100] = "SELECT * FROM unline WHERE name ='";
		sprintf(cmd+strlen(cmd), "%s", name);
		sprintf(cmd+strlen(cmd), "%s", "';");
	
		if(mysql_real_query(Mysql_sever._mpcon, cmd, strlen(cmd)))
		{
			cerr<<"mysql_real_query fail;errno:"<<errno<<endl;
			return;
		}
		Mysql_sever._mp_res = mysql_store_result(Mysql_sever._mpcon);
	
		if(Mysql_sever._mp_row = mysql_fetch_row(Mysql_sever._mp_res))
		{//是否在线
			int send_fd = atoi(Mysql_sever._mp_row[0]);
			sprintf(new_buff+strlen(new_buff), "%s", root["new"].asString().c_str());
			Json::Value val;
			val["back_new"] = new_buff;
			if(-1 == send(send_fd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str()), 0))
			{
				cerr<<"send reason fail;errno:"<<errno<<endl;
				return;
			}
		}
		else
		{//没在线将消息存入离线列表
			sprintf(new_buff+strlen(new_buff), "%s", root["new"].asString().c_str());
		
			char cmd[1024] = "INSERT INTO offline VALUE('";
			sprintf(cmd+strlen(cmd), "%s", name);
			sprintf(cmd+strlen(cmd), "%s", "','");
			sprintf(cmd+strlen(cmd), "%s", new_buff);
			sprintf(cmd+strlen(cmd), "%s", "');");

			if(mysql_real_query(Mysql_sever._mpcon, cmd, strlen(cmd)))
			{
				cerr<<"insert unline fail;errno:"<<errno<<endl;
				return;
			}
		}

		_str = "发送成功！！";
	}
	else
	{
		_str = "该用户不在好友列表!!";
	}
}

void View_talk_to_one::response()
{
	Json::Value val;
	val["back_new"] = _str;
	if(-1 == send(_fd, val.toStyledString().c_str(),strlen(val.toStyledString().c_str()), 0))
	{
		cerr<<"send fail; errno:"<<errno<<endl;
		return;
	}	
}
