#ifndef GET_LIST
#define GET_LIST
#include "View.h"
class View_get_list : public View
{
public:
	void process(int fd,char* json);
	void response();
private:
	int _fd;
	const char*  _str;
};

#endif
