#ifndef __EVENTLOOP_H
#define __EVENTLOOP_H

class Epoll;
class Channel;
class EventLoop
{
private:
	Epoll *ep;
	bool quit;

public:
	EventLoop();
	~EventLoop();

	void loop();
	void updateChannel(Channel*);

};
#endif
