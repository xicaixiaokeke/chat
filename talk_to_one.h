#ifndef TALK_TO_ONE
#define TALK_TO_ONT
#include "View.h"
class View_talk_to_one : public View
{
public:
	void process(int fd,char* json);
	void response();
private:
	int _fd;
	const char*  _str;
};

#endif
