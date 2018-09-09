#ifndef LOGIN
#define LOGIN
#include "View.h"
class View_login : public View
{
public:
	void process(int fd,char* json);
	void response();
private:
	int _fd;
	const char*  _str;
	void send_offline();
	char* send_json;
};

#endif
