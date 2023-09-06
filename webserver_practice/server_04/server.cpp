#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
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
  //struct sockaddr_in server_addr;
  //bzero(&server_addr, sizeof(server_addr));
  //server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  //server_addr.sin_port = htons(8888);
  //server_addr.sin_family = AF_INET;

  //int sockfd = socket(AF_INET,SOCK_STREAM,0);

  //errif(bind(sockfd, (const struct sockaddr*)&server_addr, sizeof(server_addr)) == -1, "bind failed!\n");

  //errif(listen(sockfd, SOCKMAX) == -1, "listen failed!\n");

  ////struct sockaddr_in client_addr;
  ////bzero(&client_addr,sizeof(client_addr));
  ////socklen_t len = sizeof(client_addr);
  ////int client_fd = accept(sockfd, (struct sockaddr*)&client_addr,&len);
  ////errif(client_fd == -1, "accept failed !\n");
  ////printf("The client fd  is %d, the client ip is %s the client port is %d\n", client_fd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

  //int epfd = epoll_create1(0);
  //errif(epfd == -1, "epoll create error\n");

  //struct epoll_event events[MAX_EVENTS], ev;
  //bzero(&events, sizeof(events));

  //bzero(&ev, sizeof(ev));

  //ev.data.fd = sockfd;   //register sockfd as the observed file descriptor.
  //ev.events = EPOLLIN | EPOLLET;
  //setnonblocking(sockfd);
  //epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
 
  Socket* serv_sock = new Socket();
  InetAddress* serv_addr = new InetAddress("127.0.0.1", 8888);
  serv_sock->bind(serv_addr);
  serv_sock->listen();
  Epoll *ep = new Epoll();
  serv_sock->setnonblocking();
  ep->addFd(serv_sock->getFd(), EPOLLIN | EPOLLET);

  while(1)
  {
    std::vector<epoll_event> events = ep->poll(); 
    int nfds = events.size();
    for(int i = 0; i < nfds; i++)
    {
      if(events[i].data.fd == serv_sock->getFd())
      {
        InetAddress *clnt_addr = new InetAddress();
        Socket *clnt_sock = new Socket(serv_sock->accept(clnt_addr));
        printf("New client fd %d! IP:%s port %d\n",clnt_sock->getFd(), inet_ntoa(clnt_addr->addr.sin_addr), ntohs(clnt_addr->addr.sin_port));
        clnt_sock->setnonblocking();
        ep->addFd(clnt_sock->getFd(), EPOLLET|EPOLLIN);
      }
      else if(events[i].events & EPOLLIN)
      {
        handleReadEvent(events[i].data.fd);
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
