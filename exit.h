#ifndef EXIT
#define EXIT
#include "View.h"
class View_exit : public View
{
public:
	void process(int fd,char* json);
	void response();
private:
	int _fd;
	const char*  _str;
};

#endif
