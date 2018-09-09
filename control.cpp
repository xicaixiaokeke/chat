#include "View.h"
#include "register.h"
#include "get_list.h"
#include "talk_to_one.h"
#include "talk_to_group.h"
#include "login.h"
#include "exit.h"
#include "public.h"
#include "control.h"

#include <json/json.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <iostream>
using namespace std;

Control::Control()
{
	_map.insert(make_pair(MSG_TYPE_REGISTER, new View_register()));
	_map.insert(make_pair(MSG_TYPE_LOGIN, new View_login()));
	_map.insert(make_pair(MSG_TYPE_TALK_TO_ONE, new View_talk_to_one()));
	_map.insert(make_pair(MSG_TYPE_TALK_TO_GROUP,new View_talk_to_group()));
	_map.insert(make_pair(MSG_TYPE_GET_LIST, new View_get_list()));
	_map.insert(make_pair(MSG_TYPE_EXIT, new View_exit()));
}

void Control::process(int fd,char* json)
{
	char send_json[128] = {0};
	sprintf(send_json, "%s", json);
	//解析json，获取消息类型
	Json::Value root;
	Json::Reader read;
	if(-1 == read.parse(json, root))
	{
		cerr<<"Control process json prase fail;errno:"<<errno<<endl;
		return;
	}
	//根据消息类型在map中查找
	map<int, View*>::iterator it = _map.find(root["reason_type"].asInt());
	//判断是否找到
	if(it == _map.end())
	{	
		Json::Value val;
		val["back_new"] = "Type fail";

		if(-1 == send(fd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str()), 0))
		{
			cerr<<"Control process send fail;errno:"<<errno<<endl;
			return;
		}
	}
	else
	{
		it->second->process(fd,json);
		it->second->response();
	}
}
Control control_sever;
