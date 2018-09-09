#include "mysql.h"
#include "login.h"

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
char name[128] = {0};

void View_login::process(int fd,char* json)
{
	//解析 json
	//name    pw
	_fd = fd;
	Json::Value root;
	Json::Reader read;
	if(-1 == read.parse(json, root))
	{
		cerr<<"login json prase fail;errno:"<<errno<<endl;
		return ;
	}
	const char* a = root["name"].asString().c_str();

	sprintf(name, "%s", a);

	const char* pw = root["pw"].asString().c_str();
	//在数据库中查找name有没有重复
	char cmd[100] = "SELECT * FROM user WHERE name='";
	sprintf(cmd+strlen(cmd), "%s", name);
	sprintf(cmd+strlen(cmd), "%s", "'and pw='");
	sprintf(cmd+strlen(cmd), "%s", pw);
	sprintf(cmd+strlen(cmd), "%s", "';");
	if(mysql_real_query(Mysql_sever._mpcon, cmd, strlen(cmd)))
	{
		cerr<<"mysql_real_query fail;errno:"<<errno<<endl;
		return;
	}
	Mysql_sever._mp_res = mysql_store_result(Mysql_sever._mpcon);
	
	if(Mysql_sever._mp_row = mysql_fetch_row(Mysql_sever._mp_res))
	{//账户密码是否正确	
		char cmd[100] = "SELECT * FROM unline WHERE name='";
		sprintf(cmd+strlen(cmd), "%s", name);
		sprintf(cmd+strlen(cmd), "%s", "';");
		if(mysql_real_query(Mysql_sever._mpcon, cmd, strlen(cmd)))
		{
			cerr<<"mysql_real_query fail;errno:"<<errno<<endl;
			return;
		}
		Mysql_sever._mp_res = mysql_store_result(Mysql_sever._mpcon);
		
		if(!(Mysql_sever._mp_row = mysql_fetch_row(Mysql_sever._mp_res)))
		{//是否在线
			char cmd[100] = "INSERT INTO unline VALUE('";
			sprintf(cmd+strlen(cmd), "%d", _fd);
			sprintf(cmd+strlen(cmd), "%s", "','");
			sprintf(cmd+strlen(cmd), "%s", name);
			sprintf(cmd+strlen(cmd), "%s", "');");
			if(mysql_real_query(Mysql_sever._mpcon, cmd, strlen(cmd)))
			{
				cerr<<"insert unline fail;errno:"<<errno<<endl;
				return;
			}
			_str = "success";
		}
		else
		{
			_str= "用户已经在线！！！";
		}
	}
	else
	{
		_str = "信息错误！！";
	}

}

void View_login::response()
{
	if(strcmp(_str, "success") == 0)
	{
		char cmd[128] = "SELECT* FROM offline WHERE name='";
		sprintf(cmd+strlen(cmd), "%s", name);
		sprintf(cmd+strlen(cmd), "%s", "';");
		if(mysql_real_query(Mysql_sever._mpcon, cmd, strlen(cmd)))
		{//查找是否有离线
			cerr<<"mysql_real_query fail;errno:"<<errno<<endl;
			return;
		}
		char buff[1024] = {0};
		Mysql_sever._mp_res = mysql_store_result(Mysql_sever._mpcon);
	
		while(Mysql_sever._mp_row = mysql_fetch_row(Mysql_sever._mp_res))
		{//将离线消息链接起来
			sprintf(buff+strlen(buff), "%s", Mysql_sever._mp_row[1]);
			sprintf(buff+strlen(buff), "%s", "\n");
		}
		char cmd1[128] = "delete FROM offline WHERE name='";
		sprintf(cmd1+strlen(cmd1), "%s", name);
		sprintf(cmd1+strlen(cmd1), "%s", "';");

		if(mysql_real_query(Mysql_sever._mpcon, cmd1, strlen(cmd1)))
		{//删除离线消息
			cerr<<"mysql_real_query fail;errno:"<<errno<<endl;
			return;
		}

		char send_buff[1024] = {0};
		sprintf(send_buff, "%s", _str);
		sprintf(send_buff+strlen(send_buff), "%s", buff);
		//用json 将消息打包，发送给客户端
		Json::Value val;
		val["back_new"] = send_buff;
		if(-1 == send(_fd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str()), 0))
		{
			cerr<<"login::response send fail;errno:"<<errno<<endl;
			return;
		}
	}
	else
	{
		//用json 将消息打包，发送给客户端
		Json::Value val;
		val["back_new"] = _str;
		if(-1 == send(_fd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str()), 0))
		{
			cerr<<"login::response send fail;errno:"<<errno<<endl;
			return;
		}

	}
	
}

