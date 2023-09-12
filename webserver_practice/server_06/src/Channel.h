#ifndef CHANNEL_H
#define CHANNEL_H
#include <stdio.h>
#include <functional>
#include <cstdint>
#include "EventLoop.h"
class Epoll;
class Channel
{
private:
	EventLoop *loop;
	int fd;
	uint32_t events;
	uint32_t revents;
	bool inEpoll;
        std::function<void()>callback;
public:
	Channel(EventLoop* _loop, int _fd);
	~Channel();

	void enableReading();
        void handleEvent();
	int getFd();
	uint32_t getEvents();
	uint32_t getRevents();
	bool getInEpoll();
	void setInEpoll();
        void setCallback(std::function<void()>);
	void setRevents(uint32_t);
};

#endif
