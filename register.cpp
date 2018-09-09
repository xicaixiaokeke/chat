#include "mysql.h"
#include "register.h"
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

void View_register::process(int fd,char* json)
{
	//解析 json
	//name    pw
	_fd = fd;
	Json::Value root;
	Json::Reader read;
	if(-1 == read.parse(json, root))
	{
		cerr<<"Register json prase fail;errno:"<<errno<<endl;
		return ;
	}
	const char* name = root["name"].asString().c_str();
	char name1[128] = {0};
	sprintf(name1, "%s", name);

	//在数据库中查找name有没有重复
	char cmd[128] = "SELECT * FROM user WHERE name='";
	sprintf(cmd+strlen(cmd), "%s", name);
	sprintf(cmd+strlen(cmd), "%s", "';");
	
	if(mysql_real_query(Mysql_sever._mpcon, cmd, strlen(cmd)))
	{
		cerr<<"mysql_real_query fail;errno:"<<errno<<endl;
		return;
	}

	Mysql_sever._mp_res = mysql_store_result(Mysql_sever._mpcon);
	

	if(!(Mysql_sever._mp_row = mysql_fetch_row(Mysql_sever._mp_res)))
	{	
		const char* pw = root["pw"].asString().c_str();
		char cmd[128] = "INSERT INTO user VALUE('";
		sprintf(cmd+strlen(cmd), "%s", name1);
		sprintf(cmd+strlen(cmd), "%s", "','");
		sprintf(cmd+strlen(cmd), "%s", pw);
		sprintf(cmd+strlen(cmd), "%s", "');");

		if(mysql_real_query(Mysql_sever._mpcon, cmd, strlen(cmd)))
		{
			cerr<<"insert usr fail;errno:"<<errno<<endl;
			return;
		}
		_str = "success";
	}
	else
	{
		_str= "fail";
	}
}

void View_register::response()
{	
	char send_buff[128] = {0};
	sprintf(send_buff, "%s", _str);
	//用json 将消息打包，发送给客户端
	Json::Value val;
	val["back_new"] = send_buff;
	if(-1 == send(_fd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str()), 0))
	{
		cerr<<"Register::response send fail;errno:"<<errno<<endl;
		return;
	}
}
