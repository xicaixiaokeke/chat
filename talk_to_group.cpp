#include "mysql.h"
#include "talk_to_group.h"

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
char fail_buff[128] = {0};

void View_talk_to_group::process(int fd,char* json)
{
	_fd = fd;
	char new_buff[1024] = {0};
	Json::Value root;
	Json::Reader read;
	if(-1 == read.parse(json, root))
	{
		cerr<<"Register Json.parse fail;errno:"<<endl;
	}
	//保存names
	const char* a = root["name"].asString().c_str();
	char names[128] = {0};
	sprintf(names, "%s", a);
//
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
	sprintf(new_buff+strlen(new_buff), "%s", root["new"].asString().c_str());
//
//
	char* p = strtok(names, ",");
	while(p != NULL)
	{
		char name[128] = {0};
		sprintf(name, "%s", p);
		
		char cmd3[128] = "SELECT * FROM user WHERE name='";
		sprintf(cmd3+strlen(cmd3), "%s", name);
		sprintf(cmd3+strlen(cmd3), "%s", "';");
		if(mysql_real_query(Mysql_sever._mpcon, cmd3, strlen(cmd3)))
		{
			cerr<<"mysql_real_query fail;errno:"<<errno<<endl;
			return;
		}
		Mysql_sever._mp_res = mysql_store_result(Mysql_sever._mpcon);
		if(Mysql_sever._mp_row = mysql_fetch_row(Mysql_sever._mp_res))
		{//查找接收消息的人是否存在
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
		}
		else
		{
			sprintf(fail_buff+strlen(fail_buff), "%s", name);
			sprintf(fail_buff+strlen(fail_buff), "%s", ",");
		}
		
		p = strtok(NULL, ",");
	}
}

void View_talk_to_group::response()
{
	if(strlen(fail_buff) == 0)
	{
		_str = "发送成功";
		sprintf(fail_buff, "%s", _str);
	}
	else
	{
		sprintf(fail_buff+strlen(fail_buff), "%s", " 用户名错误!!");
	}
	Json::Value val;
	val["back_new"] = fail_buff;
	if(-1 == send(_fd, val.toStyledString().c_str(),strlen(val.toStyledString().c_str()), 0))
	{
		cerr<<"send fail;errno:"<<errno<<endl;
		return;
	}

	memset(fail_buff, 0, 128);
}
