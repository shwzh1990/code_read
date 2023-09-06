#ifndef SOCKET_H
#define SOCKET_H
#include "InetAddress.h"
class Socket 
{
  private:
         int fd;

  public:
    Socket();
    Socket(int _fd);
    void bind(InetAddress* addr);
    void listen();
    void setnonblocking();
    int accept(InetAddress*);
    int getFd();
    ~Socket();



};



#endif
