#ifndef EPOLL_H
#define EPOLL_H
#include <sys/epoll.h>
#include "Channel.h"
#include <vector>
class Epoll 
{
  private:
    int epfd;
    struct epoll_event *events;
  public: 
    Epoll();
    ~Epoll();

    void addFd(int fd, uint32_t op);
    std::vector<Channel*> poll(int timeout = -1);
    void updateChannel(Channel* channel);

};


#endif
