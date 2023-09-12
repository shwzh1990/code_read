#include "Socket.h"
#include "InetAddress.h"
#include "util.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

Socket::Socket(): fd(-1)
{
  fd = socket(AF_INET, SOCK_STREAM, 0);
  errif(fd == -1, "socket failed!!\n");
}

Socket::Socket(int _fd): fd(_fd)
{
  errif(fd == -1, "socket create error\n");
}

Socket::~Socket()
{
  if(fd == -1)
  {
    close(fd);
    fd = -1;
  }
}

void Socket::bind(InetAddress* addr)
{
  errif(::bind(fd, (sockaddr*)&addr->addr, addr->addr_len) == -1, "bind failed!\n");
}

void Socket::setnonblocking()
{
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

void Socket::listen()
{
  errif(::listen(fd, 30)==-1, "socket lisent error\n");
}

int Socket::accept(InetAddress* addr)
{
  int clnt_sockfd = ::accept(fd, (sockaddr*)&addr->addr, &addr->addr_len);
  errif(clnt_sockfd == -1, "socket accept error!\n");
  return clnt_sockfd;
}

int Socket::getFd()
{
  return fd;
}
