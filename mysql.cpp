#include <iostream>
#include <string.h>
#include <errno.h>
#include <string>
using namespace std;
#include"mysql.h"
Mysql::Mysql()
{
	_mpcon = mysql_init((MYSQL*)0);
	if(NULL == _mpcon)
	{
		cerr<<"mycon == NULL;errno:"<<errno<<endl;
		throw;
	}
	//链接数据库
	if(!mysql_real_connect(_mpcon, "127.0.0.1", "root", "123456", NULL, 3306, NULL, 0))
	{
		cerr<<"mysql connect fail;errno:"<<errno<<endl;
		throw;
	}
	//选择数据库
	if(mysql_select_db(_mpcon, "chat"))
	{
		cerr<<"databass select fail;errno:"<<errno<<endl;
		throw;
	}
}

Mysql::~Mysql()
{
	if(NULL != _mp_res)
	{
		mysql_free_result(_mp_res);
	}

	mysql_close(_mpcon);
}


Mysql Mysql_sever;
