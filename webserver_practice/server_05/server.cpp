#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include "Channel.h"
#include "util.h"
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include "Epoll.h"
#include "InetAddress.h"
#include "Socket.h"
#define SOCKMAX 30
#define READ_BUFFER 1024
#define MAX_EVENTS 30



void handleReadEvent(int sockfd);
/*
 * Add epoll to support multi-io.
 *
 * */
int main()
{
 
  Socket* serv_sock = new Socket();
  InetAddress* serv_addr = new InetAddress("127.0.0.1", 8888);
  serv_sock->bind(serv_addr);
  serv_sock->listen();
  Epoll *ep = new Epoll();
  serv_sock->setnonblocking();
  Channel *servChannel = new Channel(ep, serv_sock->getFd());
  servChannel->enableReading();
  
  while(1)
  {
    std::vector<Channel*> activeChannels = ep->poll(); 
    int nfds = activeChannels.size();
    for(int i = 0; i < nfds; i++)
    {
      int nfds = activeChannels[i]->getFd();
      if(nfds == serv_sock->getFd())
      {
        InetAddress *clnt_addr = new InetAddress();
        Socket *clnt_sock = new Socket(serv_sock->accept(clnt_addr));
        printf("New client fd %d! IP:%s port %d\n",clnt_sock->getFd(), inet_ntoa(clnt_addr->addr.sin_addr), ntohs(clnt_addr->addr.sin_port));
        clnt_sock->setnonblocking();
	Channel* clntChannel = new Channel(ep, clnt_sock->getFd());
	clntChannel->enableReading();
      }
      else if(activeChannels[i]->getRevents() & EPOLLIN)
      {
        handleReadEvent(activeChannels[i]->getFd());
      }
      else
      {
        printf("something else happend\n");
      }
    }
  }
  delete serv_sock;
  delete serv_addr;
  return 0;
}



void handleReadEvent(int sockfd)
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
