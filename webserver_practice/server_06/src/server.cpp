#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include "Channel.h"
#include "EventLoop.h"
#include "util.h"
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include "Epoll.h"
#include "InetAddress.h"
#include "Socket.h"
#include "server.h"
#include <functional>
#define SOCKMAX 30
#define READ_BUFFER 1024
#define MAX_EVENTS 30



void handleReadEvent(int sockfd);
/*
 * Add epoll to support multi-io.
 *
 * */
Server::Server(EventLoop* _loop):loop(_loop)
{
  Socket* serv_sock = new Socket();
  InetAddress* serv_addr = new InetAddress("127.0.0.1", 8888);
  serv_sock->bind(serv_addr);
  serv_sock->listen();
  serv_sock->setnonblocking();

  Channel* servChannel = new Channel(loop, serv_sock->getFd());
  std::function<void()>cb = std::bind(&Server::newConnection, this, serv_sock);
  servChannel->setCallback(cb);
  servChannel->enableReading();

}

Server::~Server()
{

}



void Server::handleReadEvent(int sockfd)
{
  char buf[READ_BUFFER];
  while(true)
  {
    bzero(buf, sizeof(buf));
    ssize_t bytes_read = read(sockfd, buf, sizeof(buf));
    if(bytes_read > 0)
    {
      printf("The message from client fd %d: %s\n", sockfd, buf);
      write(sockfd, buf, sizeof(buf));
    }
    else if((bytes_read == -1) && (errno == EINTR))
    {
      printf("continue reading....\n");
      continue;
    }
    else if((bytes_read == -1) && ((errno == EAGAIN) || (errno = EWOULDBLOCK)))
    {
      printf("finish reading once, errno: %d\n", errno);
      break;
    }
    else if(bytes_read == 0)
    {
       printf("client is disconnected!!\n");
       close(sockfd);
       break;
    }
  }
}

void Server::newConnection(Socket* serv_sock)
{
  InetAddress* clnt_addr = new InetAddress();
  Socket* clnt_sock = new Socket(serv_sock->accept(clnt_addr));
  clnt_sock->setnonblocking();
  Channel *clntChannel = new Channel(loop, clnt_sock->getFd());
  std::function<void()> cb = std::bind(&Server::handleReadEvent, this, clnt_sock->getFd());
  clntChannel->setCallback(cb);
  clntChannel->enableReading();

}
